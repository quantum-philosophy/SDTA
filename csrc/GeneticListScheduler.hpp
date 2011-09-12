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

template<class CT, class PT>
void GenericGLSStats<CT, PT>::watch(population_t &_population, bool _silent)
{
	population = &_population;
	silent = _silent;

	generations = 0;
	evaluations = 0;
	deadline_misses = 0;

	crossover_rate = 0;
	mutation_rate = 0;

	reset();
}

template<class CT, class PT>
eoMonitor &GenericGLSStats<CT, PT>::operator()()
{
	if (!population)
		throw std::runtime_error("The population is not defined.");

	generations++;

	process();

	return *this;
}

template<class CT, class PT>
void GenericGLSStats<CT, PT>::display(std::ostream &o) const
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Stats:" << std::endl

		<< std::setprecision(0)
		<< "  Generations:     " << generations << std::endl
		<< "  Evaluations:     " << evaluations << std::endl
		<< "  Deadline misses: " << deadline_misses << std::endl;
}

/******************************************************************************/
/* GenericGLS                                                                 */
/******************************************************************************/

template<class CT, class PT, class ST>
GenericGLS<CT, PT, ST>::GenericGLS(Architecture *_architecture,
	Graph *_graph, Hotspot *_hotspot, const GLSTuning &_tuning) :

	architecture(_architecture), graph(_graph), hotspot(_hotspot),

	/* Constants */
	tuning(_tuning), task_count(graph->task_count),
	chromosome_length(task_count * (1 + (tuning.include_mapping ? 1 : 0))),
	constrains(graph->get_constrains()),
	sampling_interval(hotspot->sampling_interval())
{
	if (task_count == 0)
		throw std::runtime_error("The graph is empty.");

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
}

template<class CT, class PT, class ST>
ST &GenericGLS<CT, PT, ST>::solve(
	const priority_t &priority, const layout_t &layout)
{
	population_t population;

	/* Continue */
	eoGenContinue<chromosome_t> gen_continue(tuning.max_generations);

	/* Monitor */
	eoCheckPoint<chromosome_t> checkpoint(gen_continue);
	stats.watch(population, !tuning.verbose);
	checkpoint.add(stats);

	/* TODO: Get rid of... */ if (tuning.verbose) std::cout << "   0: ";

	/* Create */
	populate(population, priority, layout);

	/* TODO: Get rid of... */ stats();

	/* Transform = Crossover + Mutate */
	eslabNPtsBitCrossover<chromosome_t, population_t> crossover(
		tuning.crossover_points, tuning.crossover_min_rate,
		tuning.crossover_scale, tuning.crossover_exponent, stats);
	eslabUniformRangeMutation<chromosome_t, population_t> mutate(
		constrains, tuning.mutation_min_rate, tuning.mutation_scale,
		tuning.mutation_exponent, stats);
	eslabTransform<chromosome_t> transform(crossover, mutate);

	process(population, checkpoint, transform);

	return stats;
}

template<class CT, class PT, class ST>
void GenericGLS<CT, PT, ST>::populate(population_t &population,
	priority_t priority, layout_t layout)
{
	size_t i, j, k;
	size_t create_count;

	population.clear();

	/* The scheduling part */
	if (priority.empty())
		priority = graph->calc_priority();

	if (priority.size() != task_count)
		throw std::runtime_error("The priority vector has bad dimensions.");

	/* The mapping part */
	if (tuning.include_mapping) {
		if (layout.empty())
			layout = graph->calc_layout();

		if (layout.size() != task_count)
			throw std::runtime_error("The layout vector has bad dimensions.");
	}

	chromosome_t chromosome(chromosome_length);

	for (i = 0; i < task_count; i++) {
		chromosome[i] = priority[i];

		if (tuning.include_mapping)
			chromosome[task_count + i] = layout[i];
	}

	evaluate_chromosome(chromosome);

	/* Fill the first part with uniform chromosomes */
	create_count = tuning.uniform_ratio * tuning.population_size;
	for (i = 0; i < create_count; i++)
		population.push_back(chromosome);

	/* Fill the second part with randomly generated chromosomes */
	create_count = tuning.population_size - create_count;
	for (i = 0; i < create_count; i++) {
		for (j = 0; j < chromosome_length; j++)
			chromosome[j] = constrains[j].random();

		chromosome.invalidate();
		evaluate_chromosome(chromosome);
		population.push_back(chromosome);
	}
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

template<class CT, class PT>
eslabNPtsBitCrossover<CT, PT>::eslabNPtsBitCrossover(size_t _points,
	double _min_rate, double _scale, double _exponent,
	GenericGLSStats<CT, PT> &_stats) :

	points(_points), min_rate(_min_rate), scale(_scale),
	exponent(_exponent), stats(_stats)
{
	if (points < 1)
		std::runtime_error("The number of crossover points is invalid.");

	if (min_rate < 0 || min_rate > 1)
		std::runtime_error("The crossover minimal rate is invalid.");
}

template<class CT, class PT>
bool eslabNPtsBitCrossover<CT, PT>::operator()(CT &one, CT &another)
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

template<class CT, class PT>
eslabUniformRangeMutation<CT, PT>::eslabUniformRangeMutation(
	const constrains_t &_constrains, double _min_rate, double _scale,
	double _exponent, GenericGLSStats<CT, PT> &_stats) :

	constrains(_constrains), min_rate(_min_rate), scale(_scale),
	exponent(_exponent), stats(_stats)
{
	if (min_rate < 0 || min_rate > 1)
		std::runtime_error("The mutation minimal rate is invalid.");
}

template<class CT, class PT>
bool eslabUniformRangeMutation<CT, PT>::operator()(CT &chromosome)
{
	double rate;

	rate = stats.mutation_rate = std::max(min_rate,
		scale * std::exp(exponent * (double)stats.generations));

	size_t size = chromosome.size();
	bool changed = false;

	for (size_t i = 0; i < size; i++)
		if (rng.flip(rate)) {
			changed = true;
			chromosome[i] = constrains[i].random();
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
