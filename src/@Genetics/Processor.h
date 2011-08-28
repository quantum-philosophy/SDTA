#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include <iostream>
#include "Common.h"

class Processor
{
	friend class Architecture;
	friend class Task;
	friend std::ostream &operator<< (std::ostream &, const Processor *);
	friend std::ostream &operator<< (std::ostream &, const Task *);

	pid_t id;
	double frequency;
	double voltage;
	unsigned long int ngate;

	std::vector<unsigned long int> nc;
	std::vector<double> ceff;
	size_t type_count;

	size_t size() const { return type_count; }

	public:

	Processor(double _frequency, double _voltage, unsigned long int _ngate) :
		id(-1), type_count(0), frequency(_frequency), voltage(_voltage),
		ngate(_ngate) {}

	void add_type(unsigned long int nc, double ceff);
};

#endif
