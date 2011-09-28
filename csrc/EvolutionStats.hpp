#include "EvolutionStats.h"

template<class CT, class PT>
eoMonitor &GenericEvolutionStats<CT, PT>::operator()()
{
	if (!population)
		throw std::runtime_error("The population is not defined.");

	generations++;

	if (silent) return *this;

	size_t population_size = population->size();
	size_t unique = population->unique();
	double diversity = population->diversity();

	size_t current_evaluations =
		evaluation.evaluations - last_evaluations;
	size_t current_deadline_misses =
		evaluation.deadline_misses - last_deadline_misses;
	size_t current_cache_hits =
		evaluation.cache_hits - last_cache_hits;

	last_evaluations = evaluation.evaluations;
	last_deadline_misses = evaluation.deadline_misses;
	last_cache_hits = evaluation.cache_hits;

	std::cout
		<< std::endl
		<< std::setprecision(0)
		<< std::setw(4) << generations
		<< " [ "
			<< std::setw(4) << current_evaluations << ", "
			<< std::setw(4) << current_deadline_misses << ", "
			<< std::setw(4) << current_cache_hits
		<< " ]"
		<< std::setprecision(3)
		<< "[ "
			<< std::setw(6) << crossover_rate << " "
			<< std::setw(6) << mutation_rate << " "
			<< std::setw(6) << training_rate
		<< " ]"
		<< "[ "
			<< std::setw(4) << unique << "/"
			<< population_size
			<< " (" << std::setprecision(2) << diversity << ")"
		<< " ]";

	return *this;
}
