#include "SOEvolution.h"
#include "Graph.h"
#include "Lifetime.h"
#include "DynamicPower.h"
#include "ListScheduler.h"
#include "Schedule.h"

/******************************************************************************/
/* SOEvolution                                                                */
/******************************************************************************/

void SOEvolution::process(population_t &population,
	eslabCheckPoint<chromosome_t> &checkpoint)
{
	evaluate_t evaluator(*this);

	/* Select */
	eslabRouletteSelect<chromosome_t> select_one;
	eoSelectPerc<chromosome_t> select(select_one);

	/* Transform = Crossover + Mutate + Train */
	rate_t crossover_rate(tuning.crossover_min_rate, tuning.crossover_scale,
		tuning.crossover_exponent, stats.generations, stats.crossover_rate);
	eslabNPtsBitCrossover<chromosome_t> crossover(
		tuning.crossover_points, crossover_rate);

	rate_t mutation_rate(tuning.mutation_min_rate, tuning.mutation_scale,
		tuning.mutation_exponent, stats.generations, stats.mutation_rate);
	eslabUniformRangeMutation<chromosome_t> mutate(constrains, mutation_rate);

	rate_t training_rate(tuning.training_min_rate, tuning.training_scale,
		tuning.training_exponent, stats.generations, stats.training_rate);
	eslabPeerTraining<chromosome_t> train(constrains, evaluator,
		tuning.max_lessons, tuning.stall_lessons, training_rate);

	eslabTransform<chromosome_t> transform(crossover, mutate, train);

	/* Replace = Merge + Reduce */
	eslabElitismMerge<chromosome_t> merge(tuning.elitism_rate);
	eslabReduce<chromosome_t> reduce;
	eoMergeReduce<chromosome_t> replace(merge, reduce);

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
/* SOEvolutionStats                                                                 */
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
/* eslabSOEvolutionMonitor                                                    */
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
/* eslabSOStallContinue                                                       */
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
