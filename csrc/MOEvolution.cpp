#include "MOEvolution.h"
#include "Graph.h"
#include "Lifetime.h"
#include "Schedule.h"

/******************************************************************************/
/* Population                                                                 */
/******************************************************************************/

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

/******************************************************************************/
/* Evolution                                                                  */
/******************************************************************************/

void MOEvolution::process(population_t &population)
{
	const SystemTuning &system_tuning = tuning.system;
	const OptimizationTuning &optimization_tuning = tuning.optimization;

	/* Continue */
	MOContinuation continuation(tuning.continuation);

	/* Monitor */
	eslabCheckPoint<chromosome_t> checkpoint(continuation);
	stats.watch(population, !system_tuning.verbose);
	checkpoint.add(stats);

	evaluate_t evaluator(*this);

	eslabMOEvolutionMonitor evolution_monitor(population, optimization_tuning.dump);
	checkpoint.add(evolution_monitor);

	/* Transform = Crossover + Mutate */
	Crossover<chromosome_t> crossover(architecture, graph, constrains,
		tuning.crossover, stats);
	Mutation<chromosome_t> mutate(architecture, graph, constrains,
		tuning.mutation, stats);
	Transformation<chromosome_t> transform(crossover, mutate);

	moeoNSGAII<chromosome_t> ga(checkpoint, evaluator, transform);

	ga(population);

	moeoUnboundedArchive<chromosome_t> arch;
	arch(population);

	stats.pareto_optima.clear();

	size_t population_size = population.size();
	for (size_t i = 0; i < population_size; i++)
		stats.pareto_optima.push_back((price_t)population[i].objectiveVector());
}

/******************************************************************************/
/* Evolution Stats                                                            */
/******************************************************************************/

eoMonitor &MOEvolutionStats::operator()()
{
	EvolutionStats<chromosome_t, population_t>::operator()();

	best_lifetime = population->best_lifetime();
	best_energy = population->best_energy();

	if (silent) return *this;

	std::cout
		<< std::setprecision(2)
		<< "[ "
			<< "( "
				<< std::setw(10) << best_lifetime.lifetime
			<< ", * ) "
			<< "( *, "
				<< std::setw(10) << best_energy.energy
			<< " )"
		<< " ]" << std::flush;

	return *this;
}

void MOEvolutionStats::display(std::ostream &o) const
{
	EvolutionStats<chromosome_t, population_t>::display(o);

	o
		<< std::setprecision(2)
		<< "Best lifetime: " << best_lifetime << std::endl
		<< "Best energy: " << best_energy << std::endl
#ifdef REAL_RANK
#else
		<< std::setprecision(0)
#endif
		<< "Pareto optima: " << print_t<price_t>(pareto_optima) << std::endl;
}

/******************************************************************************/
/* Monitoring                                                                 */
/******************************************************************************/

eoMonitor& eslabMOEvolutionMonitor::operator()()
{
	if (stream.is_open()) {
		size_t population_size = population.size();

		for (size_t i = 0; i < population_size; i++) {
			fitness_t fitness = population[i].objectiveVector();
			stream << fitness[LIFETIME_OBJECTIVE] << "\t"
				<< fitness[ENERGY_OBJECTIVE] << "\t";
		}

		stream << std::endl;
	}

	return *this;
}
