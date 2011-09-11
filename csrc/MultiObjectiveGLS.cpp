#include "MultiObjectiveGLS.h"
#include "Graph.h"
#include "Lifetime.h"
#include "DynamicPower.h"

price_t eslabMOPop::best_lifetime() const
{
	price_t price(std::numeric_limits<double>::min(), 0);

	size_t population_size = size();

	for (size_t i = 0; i < population_size; i++) {
		fitness_t fitness = (*this)[i].objectiveVector();
		if (fitness[LIFETIME_OBJECTIVE] > price.lifetime) {
			price.lifetime = fitness[LIFETIME_OBJECTIVE];
			price.energy = fitness[ENERGY_OBJECTIVE];
		}
	}

	return price;
}

price_t eslabMOPop::best_energy() const
{
	price_t price(0, std::numeric_limits<double>::max());

	size_t population_size = size();

	for (size_t i = 0; i < population_size; i++) {
		fitness_t fitness = (*this)[i].objectiveVector();
		if (fitness[ENERGY_OBJECTIVE] < price.energy) {
			price.lifetime = fitness[LIFETIME_OBJECTIVE];
			price.energy = fitness[ENERGY_OBJECTIVE];
		}
	}

	return price;
}

void MultiObjectiveGLS::process(population_t &population,
	eoCheckPoint<chromosome_t> &checkpoint, eoTransform<chromosome_t> &transform)
{
	eslabMOStallContinue stall_continue(tuning.min_generations,
		tuning.stall_generations);
	checkpoint.add(stall_continue);

	moeoNSGAII<chromosome_t> ga(checkpoint, evaluator, transform);

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

		fitness[LIFETIME_OBJECTIVE] = std::numeric_limits<double>::min();
		fitness[ENERGY_OBJECTIVE] = std::numeric_limits<double>::max();
	}
	else {
		stats.evaluate();

		price_t price = graph->evaluate(hotspot);
		fitness[LIFETIME_OBJECTIVE] = price.lifetime;
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

	price_t best_lifetime = population->best_lifetime();
	price_t best_energy = population->best_energy();

	std::cout
		<< std::setw(width) << " "
		<< std::setprecision(2)
		<< "( "
			<< std::setw(10) << best_lifetime.lifetime
		<< ", * ) "
		<< "( *, "
			<< std::setw(10) << best_energy.energy
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

void MOGLSStats::display(std::ostream &o) const
{
	GenericGLSStats<chromosome_t, population_t>::display(o);

	o
		<< std::setprecision(2)
		<< "  Pareto optima:   " << print_t<price_t>(pareto_optima) << std::endl;
}

void eslabMOStallContinue::reset()
{
	eslabStallContinue<chromosome_t, population_t>::reset();

	last_fitness.lifetime = std::numeric_limits<double>::min();
	last_fitness.energy = std::numeric_limits<double>::max();
}

bool eslabMOStallContinue::improved(const population_t &population)
{
	bool result = false;

	size_t population_size = population.size();

	for (size_t i = 0; i < population_size; i++) {
		price_t fitness = population[i].objectiveVector();

		if (fitness.lifetime > last_fitness.lifetime) {
			last_fitness.lifetime = fitness.lifetime;
			result = true;
		}
		if (fitness.energy < last_fitness.energy) {
			last_fitness.energy = fitness.energy;
			result = true;
		}
	}

	return result;
}
