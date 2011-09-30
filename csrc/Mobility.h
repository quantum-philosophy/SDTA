#ifndef __MOBILITY_H__
#define __MOBILITY_H__

#include "common.h"

class Mobility
{
	public:

	static vector_t precise(const Architecture &architecture,
		const Graph &graph, const mapping_t &mapping);

	static vector_t average(const Architecture &architecture,
		const Graph &graph);

	private:

	static vector_t calculate(const Architecture &architecture,
		const Graph &graph, const vector_t &duration);

	static void collect_asap(const Task *task, const vector_t &duration,
		vector_t &asap, double time);
	static void collect_alap(const Task *task, const vector_t &duration,
		vector_t &asap, double time);
};

#endif
