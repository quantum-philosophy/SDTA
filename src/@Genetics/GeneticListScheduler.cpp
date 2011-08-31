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

	/* Continuator */
	eoGenContinue<chromosome_t> gen_cont(tunning.max_generations);
	eoSteadyFitContinue<chromosome_t> steady_cont(tunning.min_generations,
		tunning.stall_generations);
	eoCombinedContinue<chromosome_t> continuator(gen_cont, steady_cont);
	gen_cont.verbose = false;

	/* Evaluate */
	GeneticListSchedulerEvalFuncPtr evaluate(this);

	/* Create */
	eoPop<chromosome_t> population;

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

	/* ... evaluate it */
	evaluate(chromosome);

	/* ... fill the first part with pure mobility chromosomes */
	size_t create_count = tunning.mobility_ratio * tunning.population_size;
	for (i = 0; i < create_count; i++)
		population.push_back(chromosome);

	/* ... others are randomly generated */
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
	size_t elite_count = tunning.elitism_rate * tunning.population_size;
	eslabElitismSelect select_elite(elite_count);
	eoDetTournamentSelect<chromosome_t> select_one_parent(tunning.tournament_size);
	eoSelectNumber<chromosome_t> select_parents(select_one_parent,
		tunning.population_size - elite_count);

	/* Crossover */
	eslabNPtsBitCrossover crossover(tunning.crossover_points);

	/* Mutate */
	eslabUniformRangeMutation mutate(min_gene, max_gene,
		tunning.mutation_points, tunning.mutation_rate);

	eslabFastforwardEA ga(checkpoint, evaluate, select_elite,
		select_parents, crossover, mutate);

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

eslabFastforwardEA::eslabFastforwardEA(eoContinue<chromosome_t> &_continuator,
	eoEvalFunc<chromosome_t> &_evaluate, eoSelect<chromosome_t> &_select_elite,
	eoSelect<chromosome_t> &_select_parents, eoQuadOp<chromosome_t> &_crossover,
	eoMonOp<chromosome_t> &_mutate) :

	continuator(_continuator), evaluate(_evaluate),
	select_elite(_select_elite), select_parents(_select_parents),
	crossover(_crossover), mutate(_mutate)
{
}

void eslabFastforwardEA::operator()(eoPop<chromosome_t> &population)
{
	size_t population_size = population.size();;
	size_t elite_count;
	size_t offspring_count;

	eoPop<chromosome_t> elite;
	eoPop<chromosome_t> offspring;

	do {
		/* Select the elite */
		select_elite(population, elite);
		elite_count = elite.size();

		/* Select some parents */
		select_parents(population, offspring);

		/* Crossover and mutate */
		transform(offspring);
		offspring_count = offspring.size();

		if (elite_count + offspring_count != population_size)
			throw std::runtime_error("The size of the population is wrong.");

		/* Evaluate new */
		apply<chromosome_t>(evaluate, offspring);

		/* Evolve */
		std::copy(elite.begin(), elite.end(), population.begin());
		std::copy(offspring.begin(), offspring.end(),
			population.begin() + elite_count);
	}
	while (continuator(population));
}

void eslabFastforwardEA::transform(eoPop<chromosome_t> &population) const
{
	size_t i;
	size_t population_size = population.size();
	size_t crossover_count = population_size / 2;

	for (i = 0; i < crossover_count; i++)
		crossover(population[2 * i], population[2 * i + 1]);

	for (i = 0; i < population_size; i++) {
		mutate(population[i]);
		population[i].invalidate();
	}
}

/******************************************************************************/

eslabElitismSelect::eslabElitismSelect(size_t _count) : count(_count) {}

void eslabElitismSelect::operator()(const eoPop<chromosome_t> &source,
	eoPop<chromosome_t> &destination)
{
	destination.resize(count);

	std::vector<const chromosome_t *> elite;
	source.nth_element(count, elite);

	for (size_t i = 0; i < count; i++)
		destination[i] = *elite[i];
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
	size_t _points, double _rate) :
	max(_max), min(_min), points(_points), rate(_rate) {}

bool eslabUniformRangeMutation::operator()(chromosome_t &chromosome)
{
	size_t size = chromosome.size();
	size_t tries = points == 0 ? size : points;

	for (size_t i = 0; i < tries; i++)
		if (rng.flip(rate))
			chromosome[rng.random(size)] = eo::random(min, max);

	return tries > 0;
}

/******************************************************************************/

eslabGenerationalMonitor::eslabGenerationalMonitor(GeneticListScheduler *_scheduler,
	eoPop<chromosome_t> &_population) :
	scheduler(_scheduler), population(_population),
	tunning(scheduler->tunning), stats(scheduler->stats), last_evaluations(0) {}

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
	mobility_ratio = 0.5;

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
		else if (name == "mobility_ratio")
			mobility_ratio = value;
		else if (name == "population_size")
			population_size = value;
		else if (name == "min_generations")
			min_generations = value;
		else if (name == "max_generations")
			max_generations = value;
		else if (name == "stall_generations")
			stall_generations = value;
		else if (name == "elitism_rate")
			elitism_rate = value;
		else if (name == "tournament_size")
			tournament_size = value;
		else if (name == "crossover_points")
			crossover_points = value;
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

		<< std::setprecision(2)
		<< "  Mobility ratio:      " << tunning.mobility_ratio << std::endl

		<< std::setprecision(0)
		<< "  Population size:     " << tunning.population_size << std::endl
		<< "  Minimum generations: " << tunning.min_generations << std::endl
		<< "  Maximum generations: " << tunning.max_generations << std::endl
		<< "  Stall generations:   " << tunning.stall_generations << std::endl

		<< std::setprecision(2)
		<< "  Elitism rate:         " << tunning.elitism_rate << std::endl
		<< std::setprecision(0)
		<< "  Tournament size:     " << tunning.tournament_size << std::endl

		<< std::setprecision(0)
		<< "  Crossover points:    " << tunning.crossover_points << std::endl

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
