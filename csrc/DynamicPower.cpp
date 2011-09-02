#include <stdexcept>
#include <string.h>
#include <math.h>

#include "DynamicPower.h"
#include "Task.h"
#include "Graph.h"
#include "Processor.h"
#include "Architecture.h"

void DynamicPower::compute(const Graph *graph, double sampling_interval,
	matrix_t &dynamic_power)
{
	const Architecture *architecture = graph->architecture;

	if (!architecture)
		throw std::runtime_error("The graph should be mapped.");

	size_t task_count = graph->task_count;
	size_t processor_count = architecture->processor_count;
	size_t step_count = ceil(graph->deadline / sampling_interval);

	dynamic_power.resize(step_count, processor_count);
	double *ptr = dynamic_power.pointer();

	/* Here we build a profile for the whole time period of the graph
	 * including its actual duration (only tasks) plus the gap to
	 * the deadline.
	 */

	const Task *task;
	size_t i, j, start, end, pid;
	for (i = 0; i < task_count; i++) {
		task = graph->tasks[i];
		pid = task->processor->id;

    	start = floor(task->start / sampling_interval);
		end = floor((task->start + task->duration) / sampling_interval);

		for (j = start; j < end && j < step_count; j++)
			ptr[j * processor_count + pid] = task->power;
	}
}
