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

	stats.pareto_optima.clear();

	size_t population_size = population.size();
	for (size_t i = 0; i < population_size; i++)
		stats.pareto_optima.push_back((price_t)population[i].objectiveVector());
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

void MultiObjectiveGLSStats::reset()
{
	last_executions = 0;
}

void MultiObjectiveGLSStats::process()
{
	if (silent) return;

	size_t population_size = population->size();
	size_t width = 0;
	size_t executions = cache_hits + deadline_misses + evaluations;

	width = population_size -
		(executions - last_executions) + 1;

	double lifetime;
	double best_lifetime = 0;
	double worst_lifetime = 0;
	for (size_t i = 0; i < population_size; i++) {
		lifetime = (*population)[i].objectiveVector()[AGING_OBJECTIVE];

		if (lifetime > best_lifetime) best_lifetime = lifetime;
		if (lifetime < worst_lifetime) worst_lifetime = lifetime;
	}

	std::cout
		<< std::setw(width) << " "
		<< std::setprecision(2) << std::setw(10)
		<< worst_lifetime << " " << best_lifetime
		<< std::setprecision(3) << std::setw(8)
		<< " {" << crossover_rate << " " << mutation_rate << "}"
		<< " [" << population->unique() << "/" << population_size << "]"
		<< std::endl
		<< std::setw(4) << generations << ": ";

	last_executions = executions;
}

void MultiObjectiveGLSStats::display(std::ostream &o) const
{
	GenericGLSStats<chromosome_t>::display(o);

	o
		<< std::setprecision(2)
		<< "  Pareto optima:   " << print_t<price_t>(pareto_optima) << std::endl;
}
