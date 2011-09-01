#include <stdexcept>
#include <limits>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "GeneticListScheduler.h"
#include "Graph.h"
#include "Task.h"
#include "ListScheduler.h"
#include "DynamicPower.h"
#include "Lifetime.h"

GeneticListScheduler::GeneticListScheduler(Graph *_graph, Hotspot *_hotspot,
	const tunning_t &_tunning) :
	graph(_graph), hotspot(_hotspot), tunning(_tunning)
{
	rng.reseed(tunning.seed);
	sampling_interval = hotspot->sampling_interval();
}

schedule_t GeneticListScheduler::solve(const priority_t &start_priority)
{
	size_t i, j;
	size_t task_count = graph->task_count;

	if (task_count == 0) throw std::runtime_error("The graph is empty.");

	/* Reset */
	stats = stats_t();
	if (tunning.cache) cache.clear();

	task_vector_t &tasks = graph->tasks;

	/* Continue */
	eoGenContinue<chromosome_t> gen_cont(tunning.max_generations);
	eoSteadyFitContinue<chromosome_t> steady_cont(tunning.min_generations,
		tunning.stall_generations);
	eoCombinedContinue<chromosome_t> continuator(gen_cont, steady_cont);
	gen_cont.verbose = false;

	/* Evaluate */
	GeneticListSchedulerEvalFuncPtr evaluate(this);

	/* Create */
	population_t population;
	priority_t priority = start_priority;

	if (priority.empty()) {
		/* Fill in with mobility by default */
		priority.resize(task_count);
		for (i = 0; i < task_count; i++)
			priority[i] = tasks[i]->mobility;
	}

	if (priority.size() != task_count)
		throw std::runtime_error("The priority vector has bad dimensions.");

	chromosome_t chromosome(task_count);
	gene_t min_gene;
	gene_t max_gene;

	chromosome[0] = min_gene = max_gene = priority[0];

	gene_t gene;
	for (i = 1; i < task_count; i++) {
		chromosome[i] = gene = priority[i];
		if (min_gene > gene) min_gene = gene;
		if (max_gene < gene) max_gene = gene;
	}

	/* Monitor */
	eoCheckPoint<chromosome_t> checkpoint(continuator);
	eslabGenerationalMonitor monitor(this, population);
	checkpoint.add(monitor);

	monitor.start();

	evaluate(chromosome);

	/* Fill the first part with uniform chromosomes */
	size_t create_count = tunning.uniform_ratio * tunning.population_size;
	for (i = 0; i < create_count; i++)
		population.push_back(chromosome);

	/* Fill the second part with randomly generated chromosomes */
	create_count = tunning.population_size - create_count;
	for (i = 0; i < create_count; i++) {
		for (j = 0; j < task_count; j++)
			chromosome[j] = eo::random(min_gene, max_gene);
		chromosome.invalidate();
		evaluate(chromosome);
		population.push_back(chromosome);
	}

	monitor();

	/* Select */
	eoDetTournamentSelect<chromosome_t> select_one(tunning.tournament_size);
	eoSelectPerc<chromosome_t> select(select_one);

	/* Transform = Crossover + Mutate */
	eslabNPtsBitCrossover crossover(tunning.crossover_points);
	eslabUniformRangeMutation mutate(min_gene, max_gene, tunning.mutation_points);
	eslabTransform transform(crossover, tunning.crossover_rate,
		mutate, tunning.mutation_rate);

	/* Replace = Merge + Reduce */
	eslabElitismMerge merge(tunning.elitism_rate);
	eoLinearTruncate<chromosome_t> reduce;
	eoMergeReduce<chromosome_t> replace(merge, reduce);

	eslabEvolution ga(checkpoint, evaluate, select, transform, replace);

	ga(population);

	monitor.finish();

	return ListScheduler::process(graph, chromosome);
}

double GeneticListScheduler::evaluate(const chromosome_t &chromosome)
{
	stats.evaluations++;

	/* Make a new schedule */
	schedule_t schedule = ListScheduler::process(graph, chromosome);

	if (!tunning.cache)
		return evaluate_schedule(schedule);

	MD5Digest key(schedule);
	cache_t::const_iterator it = cache.find(key);

	double fitness;

	if (it != cache.end()) {
		stats.cache_hits++;
		fitness = it->second;

		if (tunning.verbose) {
			std::cout << "#";
			std::cout.flush();
		}
	}
	else {
		fitness = evaluate_schedule(schedule);
		cache[key] = fitness;
	}

	return fitness;
}

double GeneticListScheduler::evaluate_schedule(const schedule_t &schedule)
{
	double fitness;

	graph->assign_schedule(schedule);

	if (graph->duration > graph->deadline) {
		stats.deadline_misses++;
		fitness = std::numeric_limits<double>::min();

		if (tunning.verbose) {
			std::cout << "!";
			std::cout.flush();
		}
	}
	else {
		matrix_t dynamic_power, temperature, total_power;

		/* The graph is rescheduled now, obtain the dynamic power profile */
		DynamicPower::compute(graph, sampling_interval, dynamic_power);

		/* Now, we can get the temperature profile, and the total power profile
		 * including the leakage part.
		 */
		unsigned int iterations = hotspot->solve(graph->architecture,
			dynamic_power, temperature, total_power);

		fitness = Lifetime::predict(temperature, sampling_interval);

		if (tunning.verbose) {
			std::cout << ".";
			std::cout.flush();
		}
	}

	return fitness;
}

/******************************************************************************/

void eslabEvolution::operator()(population_t &population)
{
	size_t population_size = population.size();;
	population_t offspring;

	/* Initial evaluation */
	evaluate(population);

	do {
		/* Select */
		select(population, offspring);

		/* Transform = Crossover + Mutate */
		transform(offspring);

		/* Evaluate newcomers */
		evaluate(offspring);

		/* Evolve */
		replace(population, offspring);

		if (population.size() != population_size)
			throw std::runtime_error("The size of the population changes.");
	}
	while (continuator(population));
}

void eslabEvolution::evaluate(population_t &population) const
{
	apply<chromosome_t>(evaluate_one, population);
}

/******************************************************************************/

eslabTransform::eslabTransform(
	eoQuadOp<chromosome_t> &_crossover, double _crossover_rate,
	eoMonOp<chromosome_t> &_mutate, double _mutation_rate) :
	crossover(_crossover), crossover_rate(_crossover_rate),
	mutate(_mutate), mutation_rate(_mutation_rate)
{
	if (crossover_rate < 0 && crossover_rate > 1)
		std::runtime_error("The crossover rate is wrong.");

	if (mutation_rate < 0 && mutation_rate > 1)
		std::runtime_error("The mutation rate is wrong.");
}

void eslabTransform::operator()(population_t &population)
{
	size_t i;
	size_t population_size = population.size();
	size_t crossover_count = population_size / 2;

	bool changed;
	std::vector<bool> changes(population_size, false);

	for (i = 0; i < crossover_count; i++) {
		if (!rng.flip(crossover_rate)) continue;

		changed = crossover(population[2 * i], population[2 * i + 1]);
		changes[2 * i] = changes[2 * i] || changed;
		changes[2 * i + 1] = changes[2 * i + 1] || changed;
	}

	for (i = 0; i < population_size; i++) {
		if (!rng.flip(mutation_rate)) continue;

		changed = mutate(population[i]);
		changes[i] = changes[i] || changed;
	}

	for (i = 0; i < population_size; i++)
		if (changes[i]) population[i].invalidate();
}

/******************************************************************************/

eslabElitismMerge::eslabElitismMerge(double _rate) : rate(_rate)
{
	if (rate < 0 || rate > 1)
		std::runtime_error("The elitism rate is invalid.");
}

void eslabElitismMerge::operator()(const population_t &population,
	population_t &offspring)
{
	size_t count = rate * population.size();

	std::vector<const chromosome_t *> elite;
	population.nth_element(count, elite);

	for (size_t i = 0; i < count; i++)
		offspring.push_back(*elite[i]);
}

/******************************************************************************/

eslabNPtsBitCrossover::eslabNPtsBitCrossover(size_t _points) : points(_points)
{
	if (points < 1)
		std::runtime_error("The number of crossover points is invalid.");
}

bool eslabNPtsBitCrossover::operator()(chromosome_t &one, chromosome_t &another)
{
	size_t i;
	size_t size = one.size();
	size_t select_points = points;

	if (size != another.size())
		throw std::runtime_error("The chromosomes have different size.");

	std::vector<bool> turn_points(size, false);

	do {
		i = eo::rng.random(size);

		if (turn_points[i]) continue;
		else {
			turn_points[i] = true;
			--select_points;
		}
	}
	while (select_points);

	bool change = false;

	for (i = 0; i < size; i++) {
		if (turn_points[i]) change = !change;
		if (change) {
			double tmp = one[i];
			one[i] = another[i];
			another[i] = tmp;
		}
	}

	return true;
}

/******************************************************************************/

eslabUniformRangeMutation::eslabUniformRangeMutation(gene_t _min, gene_t _max,
	size_t _points) : max(_max), min(_min), points(_points)
{
	if (points < 1)
		std::runtime_error("The number of mutation points is invalid.");
}

bool eslabUniformRangeMutation::operator()(chromosome_t &chromosome)
{
	size_t size = chromosome.size();

	for (size_t i = 0; i < points; i++)
		chromosome[rng.random(size)] = eo::random(min, max);

	return true;
}

/******************************************************************************/

void eslabGenerationalMonitor::start()
{
	if (tunning.verbose) {
		std::cout << "   0: ";
		std::cout.flush();
	}
}

void eslabGenerationalMonitor::finish()
{
	if (tunning.verbose)
		std::cout << "end" << std::endl;
}

eoMonitor& eslabGenerationalMonitor::operator()(void)
{
	stats.best_fitness = population.best_element().fitness();
	stats.generations++;

	if (tunning.verbose) {
		size_t width = tunning.population_size -
			(stats.evaluations - last_evaluations) + 1;

		std::cout << std::setw(width) << " " << stats.best_fitness;
		std::cout << std::endl << std::setw(4)
			<< stats.generations << ": ";
		std::cout.flush();

		last_evaluations = stats.evaluations;
	}

	return *this;
}

/******************************************************************************/

void GeneticListScheduler::tunning_t::defaults()
{
	/* Randomness */
	seed = 0;

	/* Create */
	uniform_ratio = 0.5;

	/* Continuator */
	population_size = 25;
	min_generations = 0;
	max_generations = 100;
	stall_generations = 20;

	/* Select */
	elitism_rate = 0.5;
	tournament_size = 3;

	/* Crossover */
	crossover_points = 2;

	/* Mutate */
	mutation_rate = 0.05;
	mutation_points = 2;

	verbose = false;
	cache = true;
}

GeneticListScheduler::tunning_t::tunning_t()
{
	defaults();
}

GeneticListScheduler::tunning_t::tunning_t(const char *filename)
{
	defaults();

	std::ifstream file(filename);
	file.exceptions(std::fstream::failbit | std::fstream::badbit);

	if (!file.is_open())
		throw std::runtime_error("Cannot open the tunning file.");

	std::string line, name;
	double value;

	while (true) {
		try {
			std::getline(file, line);
		}
		catch (...) {
			break;
		}

		/* Skip empty lines and comments */
		if (line.empty() || line[0] == '#') continue;

		std::stringstream stream(line);
		stream.exceptions(std::ios::failbit | std::ios::badbit);

		stream >> name;
		stream >> value;

		if (name == "seed")
			seed = value;

		/* Create */
		else if (name == "uniform_ratio")
			uniform_ratio = value;
		else if (name == "population_size")
			population_size = value;

		/* Continue */
		else if (name == "min_generations")
			min_generations = value;
		else if (name == "max_generations")
			max_generations = value;
		else if (name == "stall_generations")
			stall_generations = value;

		/* Select */
		else if (name == "elitism_rate")
			elitism_rate = value;
		else if (name == "tournament_size")
			tournament_size = value;

		/* Crossover */
		else if (name == "crossover_rate")
			crossover_rate = value;
		else if (name == "crossover_points")
			crossover_points = value;

		/* Mutate */
		else if (name == "mutation_rate")
			mutation_rate = value;
		else if (name == "mutation_points")
			mutation_points = value;

		else if (name == "verbose")
			verbose = value;
		else if (name == "cache")
			cache = value;

		else
			throw std::runtime_error("An unknown tunning parameter.");
	}
}

/******************************************************************************/

std::ostream &operator<< (std::ostream &o,
	const GeneticListScheduler::tunning_t &tunning)
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Tunning:" << std::endl

		<< std::setprecision(0)
		<< "  Seed:                " << tunning.seed << std::endl

		/* Create */
		<< std::setprecision(2)
		<< "  Uniform ratio:       " << tunning.uniform_ratio << std::endl
		<< std::setprecision(0)
		<< "  Population size:     " << tunning.population_size << std::endl

		/* Continue */
		<< "  Minimum generations: " << tunning.min_generations << std::endl
		<< "  Maximum generations: " << tunning.max_generations << std::endl
		<< "  Stall generations:   " << tunning.stall_generations << std::endl

		/* Select */
		<< std::setprecision(2)
		<< "  Elitism rate:        " << tunning.elitism_rate << std::endl
		<< std::setprecision(0)
		<< "  Tournament size:     " << tunning.tournament_size << std::endl

		/* Crossover */
		<< std::setprecision(2)
		<< "  Crossover rate:      " << tunning.crossover_rate << std::endl
		<< std::setprecision(0)
		<< "  Crossover points:    " << tunning.crossover_points << std::endl

		/* Mutate */
		<< std::setprecision(2)
		<< "  Mutation rate:       " << tunning.mutation_rate << std::endl
		<< std::setprecision(0)
		<< "  Mutation points:     " << tunning.mutation_points << std::endl;
}

std::ostream &operator<< (std::ostream &o,
	const GeneticListScheduler::stats_t &stats)
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Stats:" << std::endl

		<< std::setprecision(0)
		<< "  Generations:         " << stats.generations << std::endl
		<< "  Evaluations:         " << stats.evaluations << std::endl
		<< "  Cache hits:          " << stats.cache_hits << std::endl
		<< "  Deadline misses:     " << stats.deadline_misses << std::endl

		<< std::setprecision(2)
		<< "  Best fitness:        " << stats.best_fitness << std::endl;
}
