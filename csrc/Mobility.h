#ifndef __MOBILITY_H__
#define __MOBILITY_H__

class Mobility
{
	public:

	static vector_t calculate(const Architecture &architecture,
		const Graph &graph, const mapping_t &mapping) const;

	private:

	static void collect_asap(const Task *task, const vector_t &duration,
		vector_t &asap, double time) const;
	static void collect_alap(const Task *task, const vector_t &duration,
		vector_t &asap, double time) const;
};

#endif
