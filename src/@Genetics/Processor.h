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

	public:

	Processor(double _frequency, double _voltage, unsigned long int _ngate) :
		id(-1), frequency(_frequency), voltage(_voltage), ngate(_ngate) {}
};

#endif
