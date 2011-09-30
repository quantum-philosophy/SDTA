#ifndef __PRIORITY_H__
#define __PRIORITY_H__

#include "common.h"

class Priority
{
	public:

	static priority_t mobile(const Architecture &architecture,
		const Graph &graph, const mapping_t &mapping = mapping_t());
};

#endif
