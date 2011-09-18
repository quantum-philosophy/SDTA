#ifndef __PRIORITY_H__
#define __PRIORITY_H__

#include "common.h"

class Priority
{
	public:

	static priority_t calculate(const Architecture &architecture,
		const Graph &graph, const mapping_t &mapping);
};

#endif
