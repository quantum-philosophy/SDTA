#include "SOEvolution.h"
#include "Graph.h"
#include "Lifetime.h"
#include "ListScheduler.h"
#include "Schedule.h"
#include "Selection.h"

/******************************************************************************/
/* Evolution                                                                  */
/******************************************************************************/

void SOEvolution::process(population_t &population)
{
	const SystemTuning &system_tuning = tuning.system;
	const OptimizationTuning &optimization_tuning = tuning.optimization;

	/* Continue */
	SOContinuation continuation(tuning.continuation);

	/* Monitor */
	eslabCheckPoint<chromosome_t> checkpoint(continuation);
	stats.watch(population, !system_tuning.verbose);
	checkpoint.add(stats);

	evaluate_t evaluator(*this);

	/* Select */
	Selection<chromosome_t> select(tuning.selection);

	/* Transform = Crossover + Mutate */
	Crossover<chromosome_t> crossover(architecture, graph, constrains,
		tuning.crossover, stats);
	Mutation<chromosome_t> mutate(architecture, graph, constrains,
		tuning.mutation, stats);
	Transformation<chromosome_t> transform(crossover, mutate);

	/* Replace = Merge + Reduce */
	Replacement<chromosome_t> replace(select, tuning.replacement);

	/* Train */
	Training<chromosome_t> train(architecture, graph, constrains,
		evaluation, evaluator, tuning.training, stats);

	eslabSOEvolutionMonitor evolution_monitor(population, optimization_tuning.dump);
	checkpoint.add(evolution_monitor);

	eslabSOGeneticAlgorithm<chromosome_t> ga(checkpoint, evaluator, select,
		transform, replace, train);

	ga(population);

	stats.best_chromosome = population.best_element();

	/* Calculate the energy consumption */
	evaluation.set_shallow(false);

	stats.best_price = assess(stats.best_chromosome);
}

/******************************************************************************/
/* Evolution Stats                                                            */
/******************************************************************************/

eoMonitor &SOEvolutionStats::operator()()
{
	EvolutionStats<chromosome_t, population_t>::operator()();

	if (silent) return *this;

	double worst_lifetime = population->worse_element().fitness();
	double best_lifetime = population->best_element().fitness();

	std::cout
		<< std::setprecision(2)
		<< "[ "
			<< std::setw(10) << worst_lifetime << ", "
			<< std::setw(10) << best_lifetime
		<< " ]" << std::flush;

	return *this;
}

void SOEvolutionStats::display(std::ostream &o) const
{
	EvolutionStats<chromosome_t, population_t>::display(o);

	o
		<< std::setprecision(2)
		<< "Best lifetime: " << best_price.lifetime << std::endl
		<< "Final energy: " << best_price.energy << std::endl
#ifdef REAL_RANK
#else
		<< std::setprecision(0)
#endif
		<< "Best chromosome: " << print_t<rank_t>(best_chromosome) << std::endl;
}

/******************************************************************************/
/* Monitoring                                                                 */
/******************************************************************************/

eoMonitor& eslabSOEvolutionMonitor::operator()()
{
	if (stream.is_open()) {
		size_t population_size = population.size();

		for (size_t i = 0; i < population_size; i++)
			stream << population[i].fitness() << "\t";

		stream << std::endl;
	}

	return *this;
}
