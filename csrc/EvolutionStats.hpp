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
