#include <iostream>

#include "EvolutionStats.h"

void EvolutionStats::display(std::ostream &o) const
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Stats:" << std::endl

		<< std::setprecision(0)
		<< "  Generations:     " << generations << std::endl
		<< "  Evaluations:     " << evaluations << std::endl
		<< "  Deadline misses: " << deadline_misses << std::endl;
}

std::ostream &operator<< (std::ostream &o, const EvolutionStats &stats)
{
	stats.display(o);
	return o;
}
