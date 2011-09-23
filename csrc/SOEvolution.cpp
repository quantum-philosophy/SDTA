#include "SOEvolution.h"
#include "Graph.h"
#include "Lifetime.h"
#include "DynamicPower.h"
#include "ListScheduler.h"
#include "Schedule.h"
#include "Selection.h"

/******************************************************************************/
/* Evolution                                                                  */
/******************************************************************************/

void SOEvolution::process(population_t &population,
	eslabCheckPoint<chromosome_t> &checkpoint)
{
	evaluate_t evaluator(*this);

	/* Select */
	Selection<chromosome_t> select(tuning.selection);

	/* Transform = Crossover + Mutate + Train */
	Crossover<chromosome_t> crossover(constrains, tuning.crossover, stats);
	Mutation<chromosome_t> mutate(constrains, tuning.mutation, stats);
	Training<chromosome_t> train(evaluator, constrains, tuning.training, stats);
	Transformation<chromosome_t> transform(crossover, mutate, train);

	/* Replace = Merge + Reduce */
	/*
	ElitismMerge<chromosome_t> merge(tuning.elitism_rate);
	KillerReduction<chromosome_t> reduce;
	FulfillingReplacement<chromosome_t> replace(merge, reduce, select);
	*/
	SimilarReplacement<chromosome_t> replace;

	eslabSOStallContinue stall_continue(tuning.stall_generations);
	eslabSOEvolutionMonitor evolution_monitor(population, tuning.dump_evolution);
	checkpoint.add(stall_continue);
	checkpoint.add(evolution_monitor);

	eslabSOGeneticAlgorithm<chromosome_t> ga(checkpoint, evaluator, select,
		transform, replace);

	ga(population);

	stats.best_chromosome = population.best_element();
}

/******************************************************************************/
/* Evolution Stats                                                            */
/******************************************************************************/

void SOEvolutionStats::process()
{
	worst_lifetime = population->worse_element().fitness();
	best_lifetime = population->best_element().fitness();

	if (silent) return;

	size_t population_size = population->size();
	size_t unique = population->unique();
	double diversity = population->diversity();

	std::cout
		<< std::endl
		<< std::setprecision(2)
		<< std::setw(4) << generations
		<< " ( "
			<< std::setw(10) << worst_lifetime << ", "
			<< std::setw(10) << best_lifetime
		<< " ) "
		<< std::setprecision(3)
		<< "{ "
			<< std::setw(6) << crossover_rate << " "
			<< std::setw(6) << mutation_rate << " "
			<< std::setw(6) << training_rate
		<< " } "
		<< "[ "
			<< std::setw(4) << unique << "/"
			<< population_size
			<< " (" << std::setprecision(2) << diversity << ")"
		<< " ] :";
}

void SOEvolutionStats::display(std::ostream &o) const
{
	GenericEvolutionStats<chromosome_t, population_t>::display(o);

	o
		<< std::setprecision(2)
		<< "  Best lifetime:   " << best_lifetime << std::endl
		<< "  Worst lifetime:  " << worst_lifetime << std::endl
#ifdef REAL_RANK
#else
		<< std::setprecision(0)
#endif
		<< "  Best chromosome: " << print_t<rank_t>(best_chromosome) << std::endl;
}

/******************************************************************************/
/* Monitoring                                                                 */
/******************************************************************************/

eoMonitor& eslabSOEvolutionMonitor::operator()()
{
	size_t population_size = population.size();

	for (size_t i = 0; i < population_size; i++)
		stream << population[i].fitness() << "\t";

	stream << std::endl;

	return *this;
}

/******************************************************************************/
/* Continuation                                                               */
/******************************************************************************/

void eslabSOStallContinue::reset()
{
	eslabStallContinue<chromosome_t, population_t>::reset();
	last_fitness = 0;
}

bool eslabSOStallContinue::improved(const population_t &population)
{
	bool result = false;

	double current_fitness = population.best_lifetime();

	if (current_fitness > last_fitness) result = true;

	last_fitness = current_fitness;

	return result;
}
