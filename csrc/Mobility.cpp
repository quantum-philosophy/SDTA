#include <limits>

#include "Mobility.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"

vector_t Mobility::calculate(const Architecture &architecture,
	const Graph &graph, const mapping_t &mapping) const
{
	size_t task_count = graph.size();

	tid_t id;
	const Task *task;

	vector_t asap(task_count, -1);
	vector_t alap(task_count, std::numeric_limits<double>::max());
	vector_t mobility(task_count);
	vector_t duration(task_count);

	/* Calculate durations */
	for (id = 0; id < task_count; id++) {
		task = graph.tasks[id];
		duration[id] = architecture.calc_duration(task->type);
	}

	/* Calculate ASAP */
	for (id = 0; id < task_count; id++) {
		task = graph.tasks[id];
		if (task->is_root())
			collect_asap(task, duration, asap, 0);
	}

	/* Calculate the overall duration according to ASAP */
	double asap_duration = 0;
	for (id = 0; id < task_count; id++) {
		task = graph.tasks[id];
		if (task->is_leaf())
			asap_duration = std::max(asap_duration, asap[id] + duration[id]);
	}

	/* Calculate ALAP */
	for (id = 0; id < task_count; id++) {
		task = graph.tasks[id];
		if (task->is_leaf())
			collect_alap(task, duration, alap, asap_duration);
	}

	double epsilon = std::numeric_limits<double>::epsilon();
	for (size_t i = 0; i < task_count; i++) {
		mobility[i] = alap[i] - asap[i];
		if (mobility[i] < epsilon) mobility[i] = 0;
	}

	return mobility;
}

void Mobility::collect_asap(const Task *task, const vector_t &duration,
	vector_t &asap, double time) const
{
	double &my_asap = asap[task->id];

	/* We might already have an assigned ASAP time with a larger value */
	if (!(my_asap < time)) return;

	my_asap = time;
	time = time + duration[task->id];

	/* Shift data dependent tasks */
	size_t size = task->children.size();
	for (size_t i = 0; i < size; i++)
		collect_asap(children[i], asap, time);
}

void Mobility::collect_alap(const Task *task, const vector_t &duration,
	vector_t &alap, double time) const
{
	double &my_alap = alap[task->id];

	time = time - duration[task->id];

	/* We might already have an assigned ALAP time with a smaller value */
	if (!(time < my_alap)) return;

	my_alap = time;

	/* Shift data dependent tasks */
	size_t size = task->parents.size();
	for (size_t i = 0; i < size; i++)
		collect_alap(parents[i], alap, time);
}
