#include "SingleObjectiveGLS.h"
#include "Graph.h"
#include "Lifetime.h"
#include "DynamicPower.h"

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
