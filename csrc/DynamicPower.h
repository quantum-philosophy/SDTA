#ifndef __DYNAMIC_POWER_H__
#define __DYNAMIC_POWER_H__

#include "common.h"

class DynamicPower
{
	public:

	static void compute(const Graph *graph, double sampling_interval,
		matrix_t &power);
};

#endif
