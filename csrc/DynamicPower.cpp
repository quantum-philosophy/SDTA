#include <stdexcept>
#include <string.h>
#include <math.h>

#include "DynamicPower.h"
#include "Task.h"
#include "Graph.h"
#include "Processor.h"
#include "Architecture.h"
#include "Schedule.h"

void DynamicPower::compute(const Architecture &architecture,
	const Graph &graph, const Schedule &schedule,
	double sampling_interval, matrix_t &dynamic_power)
{
	pid_t pid;
	const processor_vector_t &processors = architecture.processors;
	const task_vector_t &tasks = graph.tasks;

	size_t processor_count = architecture.size();
	size_t step_count = ceil(graph.get_deadline() / sampling_interval);

#ifndef SHALLOW_CHECK
	if (step_count == 0)
		throw std::runtime_error("The number of steps is zero.");
#endif

	dynamic_power.resize(step_count, processor_count);
	double *ptr = dynamic_power.pointer();

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
			power = processor->calc_power(tasks[item.id]->type);

#ifndef SHALLOW_CHECK
			if (end >= step_count)
				throw std::runtime_error("The duration of the task is too long.");
#endif
			for (j = start; j < end && j < step_count; j++)
				ptr[j * processor_count + pid] = power;
		}
	}
}
