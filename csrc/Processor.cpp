#include "Processor.h"

void Processor::add_type(unsigned long int nc, double ceff)
{
	this->nc.push_back(nc);
	this->ceff.push_back(ceff);
	type_count = this->nc.size();
}

std::ostream &operator<< (std::ostream &o, const Processor *processor)
{
	o.precision(2);
	o.flags(std::ios::fixed);

	o	<< std::setw(4) << processor->id << " ( "
		<< std::setw(8) << processor->frequency / 1e9 << " : "
		<< std::setw(8) << processor->voltage << " : "
		<< std::setw(8) << processor->type_count << " )" << std::endl;

	return o;
}
