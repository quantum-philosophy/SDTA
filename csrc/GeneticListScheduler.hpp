#include <stdexcept>
#include <limits>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>

#include "Graph.h"
#include "Task.h"
#include "ListScheduler.h"

/******************************************************************************/
/* eslabPop                                                                   */
/******************************************************************************/

template<class CT>
size_t eslabPop<CT>::unique() const
{
	const eslabPop<CT> &self = *this;
	size_t population_size = self.size();

	if (!population_size) return 0;

	size_t i, j, k, count;
	bool found;
	size_t chromosome_length = self[0].size();
	std::vector<bool> done(population_size, false);

	count = 0;
	for (i = 0; i < population_size; i++) {
		if (done[i]) continue;
		count++;

		for (j = i + 1; j < population_size; j++) {
			found = true;

			for (k = 0; k < chromosome_length; k++)
				if (self[i][k] != self[j][k]) {
					found = false;
					break;
				}

			if (found) done[j] = true;
		}
	}

	return count;
}

template<class CT>
double eslabPop<CT>::diversity() const
{
	const eslabPop<CT> &self = *this;
	size_t population_size = self.size();

	if (!population_size) return 0;

	size_t i, j, k;
	size_t count = 0;
	size_t chromosome_length = self[0].size();

	for (i = 0; i < population_size - 1; i++)
		for (j = i + 1; j < population_size; j++)
			for (k = 0; k < chromosome_length; k++)
				if (self[i][k] != self[j][k]) count++;

	return (double)count /
		((double)population_size * ((double)population_size - 1) / 2.0) /
		(double)chromosome_length;
}

/******************************************************************************/
/* GenericGLSStats                                                            */
/******************************************************************************/

template<class CT>
void GenericGLSStats<CT>::watch(population_t &_population,
	bool _silent)
{
	population = &_population;
	silent = _silent;

	generations = 0;
	evaluations = 0;
	cache_hits = 0;
	deadline_misses = 0;

	crossover_rate = 0;
	mutation_rate = 0;

	reset();
}

template<class CT>
eoMonitor &GenericGLSStats<CT>::operator()()
{
	if (!population)
		throw std::runtime_error("The population is not defined.");

	generations++;

	process();

	return *this;
}

template<class CT>
void GenericGLSStats<CT>::display(std::ostream &o) const
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Stats:" << std::endl

		<< std::setprecision(0)
		<< "  Generations:     " << generations << std::endl
		<< "  Evaluations:     " << evaluations << std::endl
		<< "  Cache hits:      " << cache_hits << std::endl
		<< "  Deadline misses: " << deadline_misses << std::endl;
}

/******************************************************************************/
/* GenericGLS                                                                 */
/******************************************************************************/

template<class CT, class PT, class ST>
GenericGLS<CT, PT, ST>::GenericGLS(
	Graph *_graph, Hotspot *_hotspot, const GLSTuning &_tuning) :
	graph(_graph), hotspot(_hotspot), tuning(_tuning)
{
#ifdef EO_1_2_0
	eo::log << eo::setlevel(eo::quiet);
#endif

	if (tuning.seed >= 0) rng.reseed(tuning.seed);
	else {
		time_t seed = time(NULL);
		if (tuning.verbose)
			std::cout << "Chosen seed: " << seed << std::endl;
		rng.reseed(seed);
	}

	sampling_interval = hotspot->sampling_interval();
}

template<class CT, class PT, class ST>
ST &GenericGLS<CT, PT, ST>::solve(
	const priority_t &start_priority)
{
	size_t i, j;
	size_t task_count = graph->task_count;

	if (task_count == 0) throw std::runtime_error("The graph is empty.");

	/* Reset */
	if (tuning.cache) cache.clear();

	task_vector_t &tasks = graph->tasks;

	/* Continue */
	eoGenContinue<chromosome_t> gen_cont(tuning.max_generations);
	eslabStallContinue<chromosome_t> stall_cont(tuning.min_generations,
		tuning.stall_generations);
	eoCombinedContinue<chromosome_t> continuator(gen_cont, stall_cont);

	/* Create */
	population_t population;
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

	if (tuning.verbose)
		std::cout << "   0: ";

	/* Monitor */
	eoCheckPoint<chromosome_t> checkpoint(continuator);
	stats.watch(population, !tuning.verbose);
	eslabEvolutionMonitor<chromosome_t> evolution_monitor(
			population, tuning.dump_evolution);

	checkpoint.add(stats);
	checkpoint.add(evolution_monitor);

	evaluate_chromosome(chromosome);

	/* Fill the first part with uniform chromosomes */
	size_t create_count = tuning.uniform_ratio * tuning.population_size;
	for (i = 0; i < create_count; i++)
		population.push_back(chromosome);

	/* Fill the second part with randomly generated chromosomes */
	create_count = tuning.population_size - create_count;
	for (i = 0; i < create_count; i++) {
		for (j = 0; j < task_count; j++)
			chromosome[j] = min + eo::random(max - min);
		chromosome.invalidate();
		evaluate_chromosome(chromosome);
		population.push_back(chromosome);
	}

	stats();

	/* Transform = Crossover + Mutate */
	eslabNPtsBitCrossover<chromosome_t> crossover(tuning.crossover_points,
		tuning.crossover_min_rate, tuning.crossover_scale,
		tuning.crossover_exponent, stats);
	eslabUniformRangeMutation<chromosome_t> mutate(min, max,
		tuning.mutation_min_rate, tuning.mutation_scale,
		tuning.mutation_exponent, stats);
	eslabTransform<chromosome_t> transform(crossover, mutate);

	process(population, checkpoint, transform);

	return stats;
}

template<class CT, class PT, class ST>
typename GenericGLS<CT, PT, ST>::fitness_t
GenericGLS<CT, PT, ST>::evaluate(const chromosome_t &chromosome)
{
	/* Make a new schedule */
	schedule_t schedule = ListScheduler::process(graph, chromosome);

	if (!tuning.cache)
		return evaluate_schedule(schedule);

	MD5Digest key(schedule);
	typename cache_t::const_iterator it = cache.find(key);

	fitness_t fitness;

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

template<class CT>
eslabTransform<CT>::eslabTransform(eoQuadOp<CT> &_crossover,
	eoMonOp<CT> &_mutate) : crossover(_crossover), mutate(_mutate) {}

template<class CT>
void eslabTransform<CT>::operator()(population_t &population)
{
	size_t i;
	size_t population_size = population.size();
	size_t crossover_count = population_size / 2;

	bool changed;
	std::vector<bool> changes(population_size, false);
	for (i = 0; i < crossover_count; i++) {
		changed = crossover(population[2 * i], population[2 * i + 1]);
		changes[2 * i] = changes[2 * i] || changed;
		changes[2 * i + 1] = changes[2 * i + 1] || changed;
	}

	for (i = 0; i < population_size; i++) {
		changed = mutate(population[i]);
		changes[i] = changes[i] || changed;
	}

	for (i = 0; i < population_size; i++)
		if (changes[i]) population[i].invalidate();
}

/******************************************************************************/
/* eslabNPtsBitCrossover                                                      */
/******************************************************************************/

template<class CT>
eslabNPtsBitCrossover<CT>::eslabNPtsBitCrossover(size_t _points,
	double _min_rate, double _scale, double _exponent,
	GenericGLSStats<CT> &_stats) :

	points(_points), min_rate(_min_rate), scale(_scale),
	exponent(_exponent), stats(_stats)
{
	if (points < 1)
		std::runtime_error("The number of crossover points is invalid.");

	if (min_rate < 0 || min_rate > 1)
		std::runtime_error("The crossover minimal rate is invalid.");
}

template<class CT>
bool eslabNPtsBitCrossover<CT>::operator()(CT &one, CT &another)
{
	double rate;

	rate = stats.crossover_rate = std::max(min_rate,
		scale * std::exp(exponent * (double)stats.generations));

	if (!rng.flip(rate)) return false;

	size_t i;
	size_t size = one.size();
	size_t select_points = points;

	if (size != another.size())
		throw std::runtime_error("The chromosomes have different size.");

	std::vector<bool> turn_points(size, false);

	do {
		i = 1 + rng.random(size - 1);

		if (turn_points[i]) continue;
		else {
			turn_points[i] = true;
			select_points--;
		}
	}
	while (select_points);

	bool change = false;

	for (i = 1; i < size; i++) {
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

template<class CT>
eslabUniformRangeMutation<CT>::eslabUniformRangeMutation(
	rank_t _min, rank_t _max, double _min_rate, double _scale,
	double _exponent, GenericGLSStats<CT> &_stats) :

	min(_min), range(_max - _min), min_rate(_min_rate), scale(_scale),
	exponent(_exponent), stats(_stats)
{
	if (range < 0)
		std::runtime_error("The mutation range is invalid.");

	if (min_rate < 0 || min_rate > 1)
		std::runtime_error("The mutation minimal rate is invalid.");
}

template<class CT>
bool eslabUniformRangeMutation<CT>::operator()(CT &chromosome)
{
	double rate;

	rate = stats.mutation_rate = std::max(min_rate,
		scale * std::exp(exponent * (double)stats.generations));

	size_t size = chromosome.size();
	bool changed = false;

	for (size_t i = 0; i < size; i++)
		if (rng.flip(rate)) {
			changed = true;
			chromosome[i] = min + eo::random(range);
		}

	return changed;
}

/******************************************************************************/
/* eslabEvolutionMonitor                                                      */
/******************************************************************************/

template<class CT>
eslabEvolutionMonitor<CT>::eslabEvolutionMonitor(population_t &_population,
	const std::string &filename) : population(_population)
{
	stream.open(filename.c_str());
	if (!stream.is_open())
		throw std::runtime_error("Cannot open the output file.");
}

template<class CT>
eoMonitor& eslabEvolutionMonitor<CT>::operator()()
{
	size_t population_size = population.size();

	for (size_t i = 0; i < population_size; i++)
		stream << population[i].fitness() << "\t";

	stream << std::endl;

	return *this;
}
