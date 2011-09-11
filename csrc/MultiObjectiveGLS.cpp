#include "MultiObjectiveGLS.h"
#include "Graph.h"
#include "Lifetime.h"
#include "DynamicPower.h"

eslabMOPop::fitness_t eslabMOPop::best_fitness() const
{
	const eslabMOPop &self = *this;

	fitness_t best_vector;
	size_t population_size = self.size();
	size_t objective_count = fitness_t::nObjectives();

	/* Reset */
	for (size_t i = 0; i < objective_count; i++)
		if (fitness_t::maximizing(i))
			best_vector[i] = std::numeric_limits<double>::min();
		else
			best_vector[i] = std::numeric_limits<double>::max();

	/* Find */
	for (size_t i = 0; i < population_size; i++) {
		fitness_t current_vector = self[i].objectiveVector();
		for (size_t j = 0; j < objective_count; j++) {
			if (fitness_t::maximizing(j)) {
				if (current_vector[j] > best_vector[j])
					best_vector[j] = current_vector[j];
			}
			else {
				if (current_vector[j] < best_vector[j])
					best_vector[j] = current_vector[j];
			}
		}
	}

	return best_vector;
}

void MultiObjectiveGLS::process(population_t &population,
	eoContinue<chromosome_t> &continuator, eoTransform<chromosome_t> &transform)
{
	moeoNSGAII<chromosome_t> ga(continuator, evaluator, transform);

	ga(population);

	moeoUnboundedArchive<chromosome_t> arch;
	arch(population);

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

void MOGLSStats::reset()
{
	last_executions = 0;
}

void MOGLSStats::process()
{
	if (silent) return;

	size_t population_size = population->size();
	size_t width = 0;
	size_t executions = cache_hits + deadline_misses + evaluations;

	width = population_size -
		(executions - last_executions) + 1;

	population_t::fitness_t best = population->best_fitness();

	std::cout
		<< std::setw(width) << " "
		<< std::setprecision(2)
		<< "("
			<< std::setw(10) << best[AGING_OBJECTIVE]
		<< ", *) "
		<< "(*, "
			<< std::setw(10) << best[ENERGY_OBJECTIVE]
		<< ") "
		<< std::setprecision(3)
		<< "{"
			<< std::setw(6) << crossover_rate << " "
			<< std::setw(6) << mutation_rate
		<< "} "
		<< "["
			<< std::setw(4) << population->unique() << "/"
			<< std::setw(4) << population_size
		<< "]"
		<< std::endl
		<< std::setw(4) << generations << ": ";

	last_executions = executions;
}

void MOGLSStats::display(std::ostream &o) const
{
	GenericGLSStats<chromosome_t>::display(o);

	o
		<< std::setprecision(2)
		<< "  Pareto optima:   " << print_t<price_t>(pareto_optima) << std::endl;
}
