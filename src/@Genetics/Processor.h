#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include "Common.h"

class Processor
{
	friend class Graph;
	friend class Task;

	pid_t id;
	double frequency;
	double voltage;
	unsigned long int ngate;

	std::vector<unsigned long int> nc;
	std::vector<double> ceff;

	public:

	Processor(double _frequency, double _voltage, unsigned long int _ngate) :
		id(-1), frequency(_frequency), voltage(_voltage), ngate(_ngate) {}

	void add_type(unsigned long int nc, double ceff)
	{
		this->nc.push_back(nc);
		this->ceff.push_back(ceff);
	}
};

#endif
