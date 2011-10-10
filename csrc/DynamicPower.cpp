#include "DynamicPower.h"
#include "Processor.h"
#include "Task.h"
#include "Schedule.h"

DynamicPower::DynamicPower(const processor_vector_t &_processors,
	const task_vector_t &tasks, double deadline, double _sampling_interval) :

	processors(_processors), processor_count(_processors.size()),
	sampling_interval(_sampling_interval),
	step_count(ceil(deadline / _sampling_interval))
{
	if (step_count == 0)
		throw std::runtime_error("The number of steps is zero.");

	size_t task_count = tasks.size();

	types.resize(task_count);

	for (size_t i = 0; i < task_count; i++)
		types[i] = tasks[i]->get_type();
}

void DynamicPower::compute(const Schedule &schedule, matrix_t &_dynamic_power) const
{
	pid_t pid;

	_dynamic_power.resize(step_count, processor_count);
	double *dynamic_power = _dynamic_power.pointer();

	size_t i, j, task_count, start, end;
	const Processor *processor;
	double power;

	/* Here we build a profile for the whole time period of the graph
	 * including its actual duration (only tasks) plus the gap to
	 * the deadline.
	 */

	for (pid = 0; pid < processor_count; pid++) {
		const LocalSchedule &local_schedule = schedule[pid];
		task_count = local_schedule.size();
		processor = processors[pid];

		for (i = 0; i < task_count; i++) {
			const ScheduleItem &item = local_schedule[i];

			start = floor(item.start / sampling_interval);
			end = floor((item.start + item.duration) / sampling_interval);
			power = processor->calc_power(types[item.id]);

#ifndef SHALLOW_CHECK
			if (end >= step_count)
				throw std::runtime_error("The duration of the task is too long.");
#endif
			for (j = start; j <= end && j < step_count; j++)
				dynamic_power[j * processor_count + pid] = power;
		}
	}
}
