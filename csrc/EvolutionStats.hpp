#include "EvolutionStats.h"

template<class CT, class PT>
eoMonitor &GenericEvolutionStats<CT, PT>::operator()()
{
	if (!population)
		throw std::runtime_error("The population is not defined.");

	generations++;

	process();

	return *this;
}

template<class CT, class PT>
void GenericEvolutionStats<CT, PT>::display(std::ostream &o) const
{
	EvolutionStats::display(o);

	size_t evaluations = evaluation.cache_hits + evaluation.cache_misses;
	size_t total = evaluations + evaluation.deadline_misses;

	o
		<< std::setiosflags(std::ios::fixed)
		<< std::setprecision(0)
		<< "  Total chromosomes:   " << total << std::endl
		<< "    Evaluations:       " << evaluations
			<< " (" << double(evaluations) / double(total) * 100
			<< "%)" << std::endl
		<< "      Cache hits:      " << evaluation.cache_hits
			<< " (" << double(evaluation.cache_hits) / double(total) * 100
			<< "%)" << std::endl
		<< "      Cache misses:    " << evaluation.cache_misses
			<< " (" << double(evaluation.cache_misses) / double(total) * 100
			<< "%)" << std::endl
		<< "    Deadline misses:   " << evaluation.deadline_misses
			<< " (" << double(evaluation.deadline_misses) / double(total) * 100
			<< "%)" << std::endl;
}
