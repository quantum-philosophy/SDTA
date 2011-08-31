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

schedule_t GeneticListScheduler::solve()
{
	size_t task_count = graph->task_count;

	if (task_count == 0) throw std::runtime_error("The graph is empty.");

	/* Reset */
	stats = stats_t();
	cache.clear();

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

	chromosome_t chromosome(task_count);

	/* ... first, collect mobility */
	gene_t current_mobility;
	gene_t min_mobility;
	gene_t max_mobility;
	chromosome[0] = min_mobility = max_mobility = tasks[0]->mobility;

	for (size_t i = 1; i < task_count; i++) {
		current_mobility = tasks[i]->mobility;
		if (min_mobility > current_mobility) min_mobility = current_mobility;
		if (max_mobility < current_mobility) max_mobility = current_mobility;
		chromosome[i] = current_mobility;
	}

	/* Monitor */
	eoCheckPoint<chromosome_t> checkpoint(continuator);
	eoGenerationalMonitor monitor(this, population);
	checkpoint.add(monitor);

	monitor.start();

	/* ... evaluate it */
	evaluate(chromosome);

	/* ... fill the first part with pure mobility chromosomes */
	size_t create_count = tunning.mobility_ratio * tunning.population_size;
	for (size_t i = 0; i < create_count; i++)
		population.push_back(chromosome);

	/* ... others are randomly generated */
	create_count = tunning.population_size - create_count;
	for (size_t i = 0; i < create_count; i++) {
		for (size_t j = 0; j < task_count; j++)
			chromosome[j] = eo::random(min_mobility, max_mobility);
		chromosome.invalidate();
		evaluate(chromosome);
		population.push_back(chromosome);
	}

	monitor();

	/* Select */
	eoDetTournamentSelect<chromosome_t> selectOne(tunning.tournament_size);
	eoSelectPerc<chromosome_t> select(selectOne);

	/* Crossover */
	eoNPtsBitXover<chromosome_t> crossover(tunning.crossover_points);

	/* Mutate */
	eoUniformRangeMutation mutation(min_mobility, max_mobility,
		tunning.mutation_points);

	/* Evolve */
	eoElitism<chromosome_t> merge(tunning.generation_gap);
	eoTruncate<chromosome_t> reduce;
	eoMergeReduce<chromosome_t> replace(merge, reduce);

	/* Transform */
	eoSGATransform<chromosome_t> transform(crossover, tunning.crossover_rate,
		mutation, tunning.mutation_rate);

	eoEasyEA<chromosome_t> gga(checkpoint, evaluate, select, transform, replace);

	gga(population);

	monitor.finish();

	return ListScheduler::process(graph, chromosome);
}

double GeneticListScheduler::evaluate(const chromosome_t &chromosome)
{
	stats.evaluations++;

	/* Make a new schedule */
	schedule_t schedule = ListScheduler::process(graph, chromosome);

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

		cache[key] = fitness;
	}

	return fitness;
}

eoGenerationalMonitor::eoGenerationalMonitor(GeneticListScheduler *_scheduler,
	eoPop<chromosome_t> &_population) :
	scheduler(_scheduler), population(_population),
	tunning(scheduler->tunning), stats(scheduler->stats), last_evaluations(0) {}

bool eoUniformRangeMutation::operator()(chromosome_t& chromosome)
{
	unsigned int length = chromosome.size();
	unsigned int point;
	bool hasChanged = false;
	gene_t last;

	for (unsigned int i = 0; i < points; i++) {
		point = rng.random(length);
		last = chromosome[point];
		chromosome[point] = eo::random(min, max);
		if (last != chromosome[point]) hasChanged = true;
	}

	return hasChanged;
}

void eoGenerationalMonitor::start()
{
	if (tunning.verbose) {
		std::cout << "   0: ";
		std::cout.flush();
	}
}

void eoGenerationalMonitor::finish()
{
	if (tunning.verbose)
		std::cout << "end" << std::endl;
}

eoMonitor& eoGenerationalMonitor::operator()(void)
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
	tournament_size = 3;

	/* Crossover */
	crossover_rate = 0.8;
	crossover_points = 2;

	/* Mutate */
	mutation_rate = 0.05;
	mutation_points = 2;

	/* Evolve */
	generation_gap = 0.5;

	verbose = false;
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

		if (name.compare("seed") == 0)
			seed = value;
		else if (name.compare("mobility_ratio") == 0)
			mobility_ratio = value;
		else if (name.compare("population_size") == 0)
			population_size = value;
		else if (name.compare("min_generations") == 0)
			min_generations = value;
		else if (name.compare("max_generations") == 0)
			max_generations = value;
		else if (name.compare("stall_generations") == 0)
			stall_generations = value;
		else if (name.compare("tournament_size") == 0)
			tournament_size = value;
		else if (name.compare("crossover_rate") == 0)
			crossover_rate = value;
		else if (name.compare("crossover_points") == 0)
			crossover_points = value;
		else if (name.compare("mutation_rate") == 0)
			mutation_rate = value;
		else if (name.compare("mutation_points") == 0)
			mutation_points = value;
		else if (name.compare("generation_gap") == 0)
			generation_gap = value;
		else if (name.compare("verbose") == 0)
			verbose = value;
		else
			throw std::runtime_error("An unknown tunning parameter.");
	}
}

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

		<< "  Tournament size:     " << tunning.tournament_size << std::endl

		<< std::setprecision(2)
		<< "  Crossover rate:      " << tunning.crossover_rate << std::endl
		<< std::setprecision(0)
		<< "  Crossover points:    " << tunning.crossover_points << std::endl

		<< std::setprecision(2)
		<< "  Mutation rate:       " << tunning.mutation_rate << std::endl
		<< std::setprecision(0)
		<< "  Mutation points:     " << tunning.mutation_points << std::endl

		<< std::setprecision(2)
		<< "  Generational gap:    " << tunning.generation_gap << std::endl;
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
