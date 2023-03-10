#ifndef __CONSTRAIN_H__
#define __CONSTRAIN_H__

#include "common.h"

class Constrain
{
	public:

	static constrains_t structural(const Architecture &architecture,
		const Graph &graph, const layout_t &layout = layout_t());

	private:

	static size_t count_dependents(const Task *task, bit_string_t &counted);
	static size_t count_dependencies(const Task *task, bit_string_t &counted);
};

#endif
