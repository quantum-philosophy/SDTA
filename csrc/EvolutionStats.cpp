#include <iostream>

#include "EvolutionStats.h"

void BasicEvolutionStats::display(std::ostream &o) const
{
	o
		<< std::setiosflags(std::ios::fixed)
		<< std::setprecision(0)
		<< "Generations: " << generations << std::endl;
}

std::ostream &operator<< (std::ostream &o, const BasicEvolutionStats &stats)
{
	stats.display(o);
	return o;
}
