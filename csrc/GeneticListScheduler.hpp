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
schedule_t GeneticListScheduler<chromosome_t>::solve(
	const priority_t &start_priority)
{
	size_t i, j;
	size_t task_count = graph->task_count;

	if (task_count == 0) throw std::runtime_error("The graph is empty.");

	/* Reset */
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

	if (tunning.verbose)
		std::cout << "   0: ";

	/* Monitor */
	eoCheckPoint<chromosome_t> checkpoint(continuator);
	stats.watch(population, !tunning.verbose);
	eslabEvolutionMonitor<chromosome_t> evolution_monitor(
		population, tunning.dump_evolution);

	checkpoint.add(stats);
	checkpoint.add(evolution_monitor);

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

	stats();

	/* Transform = Crossover + Mutate */
	eslabNPtsBitCrossover<chromosome_t> crossover(tunning.crossover_points);
	eslabUniformRangeMutation<chromosome_t> mutate(min, max,
		tunning.gene_mutation_rate);
	eslabTransform<chromosome_t> transform(crossover, tunning.crossover_rate,
		mutate, tunning.chromosome_mutation_rate);

	process(population, checkpoint, transform);

	if (tunning.verbose)
		std::cout << "end" << std::endl;

	chromosome = population.best_element();

	return ListScheduler::process(graph, chromosome);
}

template<class chromosome_t>
typename GeneticListScheduler<chromosome_t>::fitness_t
	GeneticListScheduler<chromosome_t>::evaluate(const chromosome_t &chromosome)
{
	/* Make a new schedule */
	schedule_t schedule = ListScheduler::process(graph, chromosome);

	if (!tunning.cache)
		return evaluate_schedule(schedule);

	MD5Digest key(schedule);
	typename cache_t::const_iterator it = cache.find(key);

	double fitness;

	if (it != cache.end()) {
		stats.hit_cache();
		fitness = it->second;
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
	rank_t _min, rank_t _max, double _rate) :
	max(_max), min(_min), rate(_rate)
{
	if (rate < 0 || rate > 1)
		std::runtime_error("The gene mutation rate is invalid.");
}

template<class chromosome_t>
bool eslabUniformRangeMutation<chromosome_t>::operator()(chromosome_t &chromosome)
{
	size_t size = chromosome.size();
	bool changed = false;

	for (size_t i = 0; i < size; i++)
		if (rng.flip(rate)) {
			changed = true;
			chromosome[i] = eo::random(min, max);
		}

	return changed;
}

/******************************************************************************/
/* eslabEvolutionMonitor                                                      */
/******************************************************************************/

template<class chromosome_t>
eslabEvolutionMonitor<chromosome_t>::eslabEvolutionMonitor(
	eoPop<chromosome_t> &_population, const std::string &filename) :
	population(_population)
{
	stream.open(filename.c_str());
	if (!stream.is_open())
		throw std::runtime_error("Cannot open the output file.");
}

template<class chromosome_t>
eoMonitor& eslabEvolutionMonitor<chromosome_t>::operator()(void)
{
	size_t population_size = population.size();

	for (size_t i = 0; i < population_size; i++)
		stream << population[i].fitness() << "\t";

	stream << std::endl;

	return *this;
}

/******************************************************************************/
/* GLSStats                                                                   */
/******************************************************************************/

template<class chromosome_t>
std::ostream &operator<< (std::ostream &o, const GLSStats<chromosome_t> &stats)
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Stats:" << std::endl

		<< std::setprecision(0)
		<< "  Generations:         " << stats.generations << std::endl
		<< "  Evaluations:         " << stats.evaluations << std::endl
		<< "  Cache hits:          " << stats.cache_hits << std::endl
		<< "  Deadline misses:     " << stats.deadline_misses << std::endl

		<< std::setiosflags(std::ios::scientific)
		<< std::setprecision(2)
		<< "  Best fitness:        " << stats.best_fitness << std::endl
		<< "  Worst fitness:       " << stats.worst_fitness << std::endl;
}
