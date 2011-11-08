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
	_dynamic_power.nullify();

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
			if ((int)end - (int)step_count > 0)
				throw std::runtime_error("The duration of the task is too long.");
#endif
			for (j = start; j <= end && j < step_count; j++)
				dynamic_power[j * processor_count + pid] = power;
		}
	}
}

CoarseDynamicPower::CoarseDynamicPower(const processor_vector_t &_processors,
	const task_vector_t &tasks, double _deadline) :

	processors(_processors), processor_count(_processors.size()),
	deadline(_deadline),

	trace(processor_count), index(processor_count),
	count(processor_count), time(processor_count)
{
	size_t task_count = tasks.size();

	types.resize(task_count);

	for (size_t i = 0; i < task_count; i++)
		types[i] = tasks[i]->get_type();
}

void CoarseDynamicPower::compute(const Schedule &schedule,
	vector_t &intervals, matrix_t &power)
{
	size_t pos;
	pid_t pid, next_pid = 0;
	double last_time, next_time;
	double *ptr;

	for (pid = 0; pid < processor_count; pid++) {
		trace[pid] = -1;
		count[pid] = schedule[pid].size();
		index[pid] = 0;
		time[pid] = (count[pid] > 0) ? schedule[pid][0].start : -1;
	}

	const size_t max_pos = 2 * schedule.tasks() + 1;

	intervals.resize(max_pos);
	power.resize(max_pos, processor_count);

	pos = 0;
	last_time = 0;

	while (true) {
		next_time = -1;

		for (pid = 0; pid < processor_count; pid++) {
			if (count[pid] == 0) continue;
			if (next_time < 0 || time[pid] < next_time) {
				next_time = time[pid];
				next_pid = pid;
			}
		}

		if (next_time < 0) {
			if (deadline > last_time) {
				intervals[pos] = deadline - last_time;
				__NULLIFY(power[pos], processor_count);
				pos++;
			}
			break;
		}

		if (last_time != next_time) {
			intervals[pos] = next_time - last_time;

			ptr = power[pos];
			for (pid = 0; pid < processor_count; pid++)
				if (trace[pid] < 0) ptr[pid] = 0;
				else ptr[pid] = processors[pid]->calc_power((unsigned int)trace[pid]);

			pos++;
		}

		const LocalSchedule &local_schedule = schedule[next_pid];

		if (trace[next_pid] < 0) {
			const ScheduleItem &item = local_schedule[index[next_pid]];
			trace[next_pid] = types[item.id];
			time[next_pid] = item.start + item.duration;
		}
		else {
			trace[next_pid] = -1;

			if (--count[next_pid] > 0) {
				const ScheduleItem &item = local_schedule[++index[next_pid]];
				time[next_pid] = item.start;
			}
		}

		last_time = next_time;
	}

	intervals.shrink(pos);
	power.shrink(pos);
}
