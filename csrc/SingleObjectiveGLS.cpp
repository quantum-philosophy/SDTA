#include "SingleObjectiveGLS.h"
#include "Graph.h"
#include "Lifetime.h"
#include "DynamicPower.h"
#include "ListScheduler.h"

void SingleObjectiveGLS::process(eoPop<chromosome_t> &population,
	eoContinue<chromosome_t> &continuator,
	eoTransform<chromosome_t> &transform)
{
	/* Select */
	eoDetTournamentSelect<chromosome_t> select_one(tuning.tournament_size);
	eoSelectPerc<chromosome_t> select(select_one);

	/* Replace = Merge + Reduce */
	eslabElitismMerge<chromosome_t> merge(tuning.elitism_rate);
	eoLinearTruncate<chromosome_t> reduce;
	eoMergeReduce<chromosome_t> replace(merge, reduce);

	eslabEvolution<chromosome_t> ga(continuator, evaluator, select,
		transform, replace);

	ga(population);

	stats.best_priority = population.best_element();
	stats.best_schedule = ListScheduler::process(graph, stats.best_priority);
}

void SingleObjectiveGLS::evaluate_chromosome(chromosome_t &chromosome)
{
	evaluator(chromosome);
}

SingleObjectiveGLS::fitness_t
SingleObjectiveGLS::evaluate_schedule(const schedule_t &schedule)
{
	fitness_t fitness;

	graph->assign_schedule(schedule);

	if (graph->duration > graph->deadline) {
		stats.miss_deadline();

		fitness = std::numeric_limits<fitness_t>::min();
	}
	else {
		stats.evaluate();

		matrix_t dynamic_power, temperature, total_power;

		/* The graph is rescheduled now, obtain the dynamic power profile */
		DynamicPower::compute(graph, sampling_interval, dynamic_power);

		/* Now, we can get the temperature profile, and the total power profile
		 * including the leakage part.
		 */
		size_t iterations = hotspot->solve(graph->architecture,
			dynamic_power, temperature, total_power);

		fitness = Lifetime::predict(temperature, sampling_interval);
	}

	return fitness;
}

void SingleObjectiveGLSStats::reset()
{
	last_executions = 0;
}

void SingleObjectiveGLSStats::process()
{
	worst_fitness = population->worse_element().fitness();
	best_fitness = population->best_element().fitness();

	if (silent) return;

	size_t population_size = population->size();
	size_t width = 0;
	size_t executions = cache_hits + deadline_misses + evaluations;

	width = population_size -
		(executions - last_executions) + 1;

	std::cout
		<< std::setw(width) << " "
		<< std::setprecision(2) << std::setw(10)
		<< worst_fitness << " " << best_fitness
		<< std::setprecision(3) << std::setw(8)
		<< " {" << crossover_rate << " " << mutation_rate << "}"
		<< " [" << population->unique() << "/" << population_size << "]"
		<< std::endl
		<< std::setw(4) << generations << ": ";

	last_executions = executions;
}

void SingleObjectiveGLSStats::display(std::ostream &o) const
{
	GenericGLSStats<chromosome_t>::display(o);

	o
		<< std::setprecision(2)
		<< "  Best fitness:    " << best_fitness << std::endl
		<< "  Worst fitness:   " << worst_fitness << std::endl

		<< std::setprecision(0)
		<< "  Best priority:   " << print_t<rank_t>(best_priority) << std::endl
		<< "  Best schedule:   " << print_t<int>(best_schedule) << std::endl;
}
