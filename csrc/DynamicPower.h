#ifndef __DYNAMIC_POWER_H__

#include "common.h"

class DynamicPower
{
	const processor_vector_t &processors;
	const size_t processor_count;
	const double sampling_interval;
	const size_t step_count;

	std::vector<unsigned int> types;

	public:

	DynamicPower(const processor_vector_t &processors,
		const task_vector_t &tasks, double deadline, double sampling_interval);

	void compute(const Schedule &schedule, matrix_t &_dynamic_power) const;
};

#endif
