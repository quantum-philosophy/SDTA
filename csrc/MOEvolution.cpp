#include "MOEvolution.h"
#include "Graph.h"
#include "Lifetime.h"
#include "DynamicPower.h"
#include "Schedule.h"

/******************************************************************************/
/* eslabMOPop                                                                 */
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
/* MOEvolution                                                                */
/******************************************************************************/

void MOEvolution::process(population_t &population,
	eslabCheckPoint<chromosome_t> &checkpoint)
{
	evaluate_t evaluator(*this);

	eslabMOStallContinue stall_continue(tuning.stall_generations);
	eslabMOEvolutionMonitor evolution_monitor(population, tuning.dump_evolution);
	checkpoint.add(stall_continue);
	checkpoint.add(evolution_monitor);

	/* Transform = Crossover + Mutate + Train */
	rate_t crossover_rate(stats.generations, tuning.crossover_min_rate,
		tuning.crossover_scale, tuning.crossover_exponent);
	eslabNPtsBitCrossover<chromosome_t, population_t> crossover(
		tuning.crossover_points, crossover_rate, stats);

	rate_t mutation_rate(stats.generations, tuning.mutation_min_rate,
		tuning.mutation_scale, tuning.mutation_exponent);
	eslabUniformRangeMutation<chromosome_t, population_t> mutate(
		constrains, mutation_rate, stats);

	rate_t training_rate(stats.generations, tuning.training_min_rate,
		tuning.training_scale, tuning.training_exponent);
	eslabPeerTraining<chromosome_t, population_t> train(
		constrains, evaluator, tuning.max_lessons, tuning.stall_lessons,
		training_rate, stats);

	eslabTransform<chromosome_t> transform(crossover, mutate, train);

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
/* MOEvolutionStats                                                                 */
/******************************************************************************/

void MOEvolutionStats::process()
{
	best_lifetime = population->best_lifetime();
	best_energy = population->best_energy();

	if (silent) return;

	size_t population_size = population->size();
	size_t unique = population->unique();
	double diversity = population->diversity();

	std::cout
		<< std::endl
		<< std::setprecision(2)
		<< std::setw(4) << generations
		<< "( "
			<< std::setw(10) << best_lifetime.lifetime
		<< ", * ) "
		<< "( *, "
			<< std::setw(10) << best_energy.energy
		<< " ) "
		<< std::setprecision(3)
		<< "{ "
			<< std::setw(6) << crossover_rate << " "
			<< std::setw(6) << mutation_rate << " "
			<< std::setw(6) << training_rate
		<< " }"
		<< "[ "
			<< std::setw(4) << unique << "/"
			<< population_size
			<< " (" << std::setprecision(2) << diversity << ")"
		<< " ] :";
}

void MOEvolutionStats::display(std::ostream &o) const
{
	GenericEvolutionStats<chromosome_t, population_t>::display(o);

	o
		<< std::setprecision(2)
		<< "  Best lifetime:   " << best_lifetime << std::endl
		<< "  Best energy:     " << best_energy << std::endl

		<< std::setprecision(2)
		<< "  Pareto optima:   " << print_t<price_t>(pareto_optima) << std::endl;
}

/******************************************************************************/
/* eslabMOEvolutionMonitor                                                    */
/******************************************************************************/

eoMonitor& eslabMOEvolutionMonitor::operator()()
{
	size_t population_size = population.size();

	for (size_t i = 0; i < population_size; i++) {
		fitness_t fitness = population[i].objectiveVector();
		stream << fitness[LIFETIME_OBJECTIVE] << "\t"
			<< fitness[ENERGY_OBJECTIVE] << "\t";
	}

	stream << std::endl;

	return *this;
}

/******************************************************************************/
/* eslabMOStallContinue                                                       */
/******************************************************************************/

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
