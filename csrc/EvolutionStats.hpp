#include "EvolutionStats.h"

template<class CT, class PT>
eoMonitor &EvolutionStats<CT, PT>::operator()()
{
#ifndef SHALLOW_CHECK
	if (!population)
		throw std::runtime_error("The population is not defined.");
#endif

	generations++;

	if (silent) return *this;

#ifdef EXTENDED_STATS
	size_t population_size = population->size();
	size_t unique = population->unique();
	double diversity = population->diversity();
#endif

	size_t current_evaluations =
		evaluation.evaluations - last_evaluations;
	size_t current_deadline_misses =
		evaluation.deadline_misses - last_deadline_misses;
	size_t current_temperature_runaways =
		evaluation.temperature_runaways - last_temperature_runaways;
	size_t current_cache_hits =
		evaluation.cache_hits - last_cache_hits;

	last_evaluations = evaluation.evaluations;
	last_deadline_misses = evaluation.deadline_misses;
	last_temperature_runaways = evaluation.temperature_runaways;
	last_cache_hits = evaluation.cache_hits;

	std::cout
		<< std::endl
		<< std::setprecision(0)
		<< std::setw(4) << generations
		<< " [ "
			<< std::setw(4) << current_evaluations - current_cache_hits -
				current_deadline_misses << ", "
			<< std::setw(4) << current_deadline_misses << ", "
			<< std::setw(4) << current_temperature_runaways << ", "
			<< std::setw(4) << current_cache_hits
		<< " ]"
		<< std::setprecision(3)
		<< "[ "
			<< std::setw(6) << crossover_rate << " "
			<< std::setw(6) << mutation_rate << " "
			<< std::setw(6) << training_rate
		<< " ]"
#ifdef EXTENDED_STATS
		<< "[ "
			<< std::setw(4) << unique << "/"
			<< population_size
			<< " (" << std::setprecision(2) << diversity << ")"
		<< " ]"
#endif
		;

	return *this;
}
