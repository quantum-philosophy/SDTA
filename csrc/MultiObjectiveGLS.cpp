#include "MultiObjectiveGLS.h"
#include "Graph.h"
#include "Lifetime.h"
#include "DynamicPower.h"

void MultiObjectiveGLS::process(eoPop<chromosome_t> &population,
	eoContinue<chromosome_t> &continuator,
	eoTransform<chromosome_t> &transform)
{
	moeoNSGAII<chromosome_t> ga(continuator, evaluator, transform);

	ga(population);
}

void MultiObjectiveGLS::evaluate_chromosome(chromosome_t &chromosome)
{
	evaluator(chromosome);
}

MultiObjectiveGLS::fitness_t
MultiObjectiveGLS::evaluate_schedule(const schedule_t &schedule)
{
	fitness_t fitness;

	graph->assign_schedule(schedule);

	if (graph->duration > graph->deadline) {
		stats.miss_deadline();

		fitness[AGING_OBJECTIVE] = std::numeric_limits<double>::min();
		fitness[ENERGY_OBJECTIVE] = std::numeric_limits<double>::max();
	}
	else {
		stats.evaluate();

		price_t price = graph->evaluate(hotspot);
		fitness[AGING_OBJECTIVE] = price.lifetime;
		fitness[ENERGY_OBJECTIVE] = price.energy;
	}

	return fitness;
}
