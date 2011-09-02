#include <stdexcept>
#include <limits>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "Graph.h"
#include "Task.h"
#include "ListScheduler.h"

/******************************************************************************/
/* GeneticListScheduler                                                       */
/******************************************************************************/

template<class chromosome_t>
GeneticListScheduler<chromosome_t>::GeneticListScheduler(
	Graph *_graph, Hotspot *_hotspot, const GLSTunning &_tunning) :
	graph(_graph), hotspot(_hotspot), tunning(_tunning)
{
	eo::log << eo::setlevel(eo::quiet);

	if (tunning.seed >= 0) rng.reseed(tunning.seed);

	sampling_interval = hotspot->sampling_interval();
}

template<class chromosome_t>
schedule_t &GeneticListScheduler<chromosome_t>::solve(
	const priority_t &start_priority)
{
	size_t i, j;
	size_t task_count = graph->task_count;

	if (task_count == 0) throw std::runtime_error("The graph is empty.");

	/* Reset */
	stats.clear();
	if (tunning.cache) cache.clear();

	task_vector_t &tasks = graph->tasks;

	/* Continue */
	eoGenContinue<chromosome_t> gen_cont(tunning.max_generations);
	eoSteadyFitContinue<chromosome_t> steady_cont(tunning.min_generations,
		tunning.stall_generations);
	eoCombinedContinue<chromosome_t> continuator(gen_cont, steady_cont);

	/* Create */
	eoPop<chromosome_t> population;
	priority_t priority = start_priority;

	if (priority.empty())
		priority = graph->calc_priority();

	if (priority.size() != task_count)
		throw std::runtime_error("The priority vector has bad dimensions.");

	chromosome_t chromosome(task_count);
	rank_t min;
	rank_t max;

	chromosome[0] = min = max = priority[0];

	rank_t rank;
	for (i = 1; i < task_count; i++) {
		chromosome[i] = rank = priority[i];
		if (min > rank) min = rank;
		if (max < rank) max = rank;
	}

	/* Monitor */
	eoCheckPoint<chromosome_t> checkpoint(continuator);
	eslabGenerationalMonitor<chromosome_t> monitor(this, population);
	checkpoint.add(monitor);

	monitor.start();

	evaluate_chromosome(chromosome);

	/* Fill the first part with uniform chromosomes */
	size_t create_count = tunning.uniform_ratio * tunning.population_size;
	for (i = 0; i < create_count; i++)
		population.push_back(chromosome);

	/* Fill the second part with randomly generated chromosomes */
	create_count = tunning.population_size - create_count;
	for (i = 0; i < create_count; i++) {
		for (j = 0; j < task_count; j++)
			chromosome[j] = eo::random(min, max);
		chromosome.invalidate();
		evaluate_chromosome(chromosome);
		population.push_back(chromosome);
	}

	/* Transform = Crossover + Mutate */
	eslabNPtsBitCrossover<chromosome_t> crossover(tunning.crossover_points);
	eslabUniformRangeMutation<chromosome_t> mutate(min, max, tunning.mutation_points);
	eslabTransform<chromosome_t> transform(crossover, tunning.crossover_rate,
		mutate, tunning.mutation_rate);

	monitor();

	process(population, checkpoint, transform);

	monitor.finish();

	chromosome = population.best_element();

	stats.priority = chromosome;
	stats.schedule = ListScheduler::process(graph, chromosome);

	return stats.schedule;
}

template<class chromosome_t>
typename GeneticListScheduler<chromosome_t>::fitness_t
	GeneticListScheduler<chromosome_t>::evaluate(const chromosome_t &chromosome)
{
	stats.evaluations++;

	/* Make a new schedule */
	schedule_t schedule = ListScheduler::process(graph, chromosome);

	if (!tunning.cache)
		return evaluate_schedule(schedule);

	MD5Digest key(schedule);
	typename cache_t::const_iterator it = cache.find(key);

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

/******************************************************************************/
/* eslabTransform                                                             */
/******************************************************************************/

template<class chromosome_t>
eslabTransform<chromosome_t>::eslabTransform(
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

template<class chromosome_t>
void eslabTransform<chromosome_t>::operator()(eoPop<chromosome_t> &population)
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
/* eslabNPtsBitCrossover                                                      */
/******************************************************************************/

template<class chromosome_t>
eslabNPtsBitCrossover<chromosome_t>::eslabNPtsBitCrossover(
	size_t _points) : points(_points)
{
	if (points < 1)
		std::runtime_error("The number of crossover points is invalid.");
}

template<class chromosome_t>
bool eslabNPtsBitCrossover<chromosome_t>::operator()(
	chromosome_t &one, chromosome_t &another)
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
			rank_t tmp = one[i];
			one[i] = another[i];
			another[i] = tmp;
		}
	}

	return true;
}

/******************************************************************************/
/* eslabMutation                                                              */
/******************************************************************************/

template<class chromosome_t>
eslabUniformRangeMutation<chromosome_t>::eslabUniformRangeMutation(
	rank_t _min, rank_t _max, size_t _points) :
	max(_max), min(_min), points(_points)
{
	if (points < 1)
		std::runtime_error("The number of mutation points is invalid.");
}

template<class chromosome_t>
bool eslabUniformRangeMutation<chromosome_t>::operator()(chromosome_t &chromosome)
{
	size_t size = chromosome.size();

	for (size_t i = 0; i < points; i++)
		chromosome[rng.random(size)] = eo::random(min, max);

	return true;
}

/******************************************************************************/
/* eslabMonitor                                                               */
/******************************************************************************/

template<class chromosome_t>
void eslabGenerationalMonitor<chromosome_t>::start()
{
	if (tunning.verbose) {
		std::cout << "   0: ";
		std::cout.flush();
	}
}

template<class chromosome_t>
void eslabGenerationalMonitor<chromosome_t>::finish()
{
	if (tunning.verbose)
		std::cout << "end" << std::endl;
}

template<class chromosome_t>
eoMonitor& eslabGenerationalMonitor<chromosome_t>::operator()(void)
{
	stats.fitness = population.best_element().fitness();
	stats.generations++;

	if (tunning.verbose) {
		size_t width = tunning.population_size -
			(stats.evaluations - last_evaluations) + 1;

		std::cout << std::setw(width)
			<< " " << stats.fitness
			<< " " << population.worse_element().fitness()
			<< std::endl << std::setw(4)
			<< stats.generations << ": ";
		std::cout.flush();

		last_evaluations = stats.evaluations;
	}

	return *this;
}
