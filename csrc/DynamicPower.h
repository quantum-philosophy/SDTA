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

class CoarseDynamicPower
{
	const processor_vector_t &processors;
	const size_t processor_count;
	const double deadline;

	std::vector<unsigned int> types;

	std::vector<int> trace;
	std::vector<size_t> index;
	std::vector<size_t> count;
	vector_t time;

	public:

	CoarseDynamicPower(const processor_vector_t &processors,
		const task_vector_t &tasks, double deadline);

	void compute(const Schedule &schedule,
		vector_t &intervals, matrix_t &power);
};

#endif
