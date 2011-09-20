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
	eslabCheckPoint<chromosome_t> &checkpoint,
	eoTransform<chromosome_t> &transform)
{
	evaluate_t evaluator(*this);

	/* Select */
	eslabTournamentSelect<chromosome_t> select_one(tuning.tournament_size);
	eoSelectPerc<chromosome_t> select(select_one);

	/* Replace = Merge + Reduce */
	eslabElitismMerge<chromosome_t> merge(tuning.elitism_rate);
	eoLinearTruncate<chromosome_t> reduce;
	eoMergeReduce<chromosome_t> replace(merge, reduce);

	eslabSOStallContinue stall_continue(tuning.min_generations,
		tuning.stall_generations);
	eslabSOEvolutionMonitor evolution_monitor(population, tuning.dump_evolution);
	checkpoint.add(stall_continue);
	checkpoint.add(evolution_monitor);

	eslabSOGeneticAlgorithm<chromosome_t> ga(checkpoint, evaluator, select,
		transform, replace);

	ga(population);

	stats.best_chromosome = population.best_element();
}

SOEvolution::fitness_t
SOEvolution::evaluate(const chromosome_t &chromosome)
{
	price_t price;

	if (tuning.include_mapping) {
		eslabDualGeneEncoder<chromosome_t> dual(chromosome);
		price = evaluation.process(dual.layout(), dual.priority(), true);
	}
	else {
		price = evaluation.process(layout, chromosome, true);
	}

	if (price.lifetime < 0) stats.miss_deadline();
	else stats.evaluate();

	return price.lifetime;
}

/******************************************************************************/
/* SOEvolutionStats                                                                 */
/******************************************************************************/

void SOEvolutionStats::reset()
{
	last_executions = 0;
}

void SOEvolutionStats::process()
{
	worst_lifetime = population->worse_element().fitness();
	best_lifetime = population->best_element().fitness();

	if (silent) return;

	size_t population_size = population->size();
	size_t width = 0;
	size_t executions = deadline_misses + evaluations;

	width = population_size -
		(executions - last_executions) + 1;

	std::cout
		<< std::setw(width) << " "
		<< std::setprecision(2)
		<< "( "
			<< std::setw(10) << worst_lifetime << ", "
			<< std::setw(10) << best_lifetime
		<< " ) "
		<< std::setprecision(3)
		<< "{ "
			<< std::setw(6) << crossover_rate << " "
			<< std::setw(6) << mutation_rate
		<< " } "
		<< "[ "
			<< std::setw(4) << population->unique() << "/"
			<< population_size
		<< " ]"
		<< std::endl
		<< std::setw(4) << generations << ": ";

	last_executions = executions;
}

void SOEvolutionStats::display(std::ostream &o) const
{
	GenericEvolutionStats<chromosome_t, population_t>::display(o);

	o
		<< std::setprecision(2)
		<< "  Best lifetime:   " << best_lifetime << std::endl
		<< "  Worst lifetime:  " << worst_lifetime << std::endl

		<< std::setprecision(0)
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
