#ifndef __GRAPH_ANALYSIS_H__
#define __GRAPH_ANALYSIS_H__

#include "common.h"

class GraphAnalysis
{
	public:

	static vector_t precise_mobility(const processor_vector_t &processors,
		const task_vector_t &tasks, const mapping_t &mapping);

	static vector_t average_mobility(const processor_vector_t &processors,
		const task_vector_t &tasks);

	static vector_t statical_criticality(const processor_vector_t &processors,
		const task_vector_t &tasks);

	private:

	static vector_t calculate_mobility(const processor_vector_t &processors,
		const task_vector_t &tasks, const vector_t &duration);

	static vector_t average_duration(const processor_vector_t &processors,
		const task_vector_t &tasks);

	static vector_t get_asap(const task_vector_t &tasks, const vector_t &duration);
	static vector_t get_alap(const task_vector_t &tasks, const vector_t &duration,
		double asap_duration);

	static double get_asap_duration(const task_vector_t &tasks, const vector_t &asap,
		const vector_t &duration);

	static void collect_asap(const Task *task, const vector_t &duration,
		vector_t &asap, double time);
	static void collect_alap(const Task *task, const vector_t &duration,
		vector_t &asap, double time);
};

#endif
