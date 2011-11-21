#include "GraphAnalysis.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"

vector_t GraphAnalysis::precise_mobility(const processor_vector_t &processors,
	const task_vector_t &tasks, const mapping_t &mapping)
{
	tid_t id;
	pid_t pid;

	size_t processor_count = processors.size();
	size_t task_count = tasks.size();

	vector_t duration(task_count);

	if (task_count != mapping.size())
		throw std::runtime_error("The mapping vector is invalid.");

	for (id = 0; id < task_count; id++) {
		pid = mapping[id];

		if (pid >= processor_count)
			throw std::runtime_error("The processor is invalid.");

		duration[id] = processors[pid]->calc_duration(tasks[id]->get_type());
	}

	return calculate_mobility(processors, tasks, duration);
}

vector_t GraphAnalysis::average_mobility(const processor_vector_t &processors,
	const task_vector_t &tasks)
{
	vector_t duration = average_duration(processors, tasks);
	return calculate_mobility(processors, tasks, duration);
}

vector_t GraphAnalysis::statical_criticality(
	const processor_vector_t &processors, const task_vector_t &tasks)
{
	size_t task_count = tasks.size();

	vector_t duration = average_duration(processors, tasks);
	vector_t asap = get_asap(tasks, duration);
	double asap_duration = get_asap_duration(tasks, asap, duration);

	vector_t criticality(task_count);

	for (tid_t id = 0; id < task_count; id++)
		criticality[id] = asap_duration - asap[id];

	return criticality;
}

vector_t GraphAnalysis::calculate_mobility(const processor_vector_t &processors,
	const task_vector_t &tasks, const vector_t &duration)
{
	size_t task_count = tasks.size();

	if (task_count != duration.size())
		throw std::runtime_error("The duration vector is invalid.");

	vector_t asap = get_asap(tasks, duration);
	double asap_duration = get_asap_duration(tasks, asap, duration);
	vector_t alap = get_alap(tasks, duration, asap_duration);
	vector_t mobility(task_count);

	for (tid_t id = 0; id < task_count; id++) {
		mobility[id] = alap[id] - asap[id];
		if (mobility[id] < DBL_EPSILON) mobility[id] = 0;
	}

	return mobility;
}

vector_t GraphAnalysis::average_duration(
	const processor_vector_t &processors, const task_vector_t &tasks)
{
	tid_t id;
	pid_t pid;

	size_t processor_count = processors.size();
	size_t task_count = tasks.size();

	vector_t duration(task_count, 0);

	for (id = 0; id < task_count; id++)
		for (pid = 0; pid < processor_count; pid++)
			duration[id] += processors[pid]->calc_duration(tasks[id]->get_type());

	for (id = 0; id < task_count; id++)
		duration[id] /= double(processor_count);

	return duration;
}

vector_t GraphAnalysis::get_asap(const task_vector_t &tasks, const vector_t &duration)
{
	size_t task_count = tasks.size();

	vector_t asap(task_count, -1);

	/* Calculate ASAP */
	for (tid_t id = 0; id < task_count; id++) {
		if (tasks[id]->is_root())
			collect_asap(tasks[id], duration, asap, 0);
	}

	return asap;
}

vector_t GraphAnalysis::get_alap(const task_vector_t &tasks,
	const vector_t &duration, double asap_duration)
{
	size_t task_count = tasks.size();

	vector_t alap(task_count, DBL_MAX);

	/* Calculate ALAP */
	for (tid_t id = 0; id < task_count; id++) {
		if (tasks[id]->is_leaf())
			collect_alap(tasks[id], duration, alap, asap_duration);
	}

	return alap;
}

double GraphAnalysis::get_asap_duration(const task_vector_t &tasks,
	const vector_t &asap, const vector_t &duration)
{
	size_t task_count = tasks.size();

	double asap_duration = 0;

	/* Calculate the overall duration according to ASAP */
	for (tid_t id = 0; id < task_count; id++)
		if (tasks[id]->is_leaf())
			asap_duration = std::max(asap_duration, asap[id] + duration[id]);

	return asap_duration;
}

void GraphAnalysis::collect_asap(const Task *task, const vector_t &duration,
	vector_t &asap, double time)
{
	double &my_asap = asap[task->id];

	/* We might already have an assigned ASAP time with a larger value */
	if (!(my_asap < time)) return;

	my_asap = time;
	time = time + duration[task->id];

	/* Shift data dependent tasks */
	size_t size = task->children.size();
	for (size_t i = 0; i < size; i++)
		collect_asap(task->children[i], duration, asap, time);
}

void GraphAnalysis::collect_alap(const Task *task, const vector_t &duration,
	vector_t &alap, double time)
{
	double &my_alap = alap[task->id];

	time = time - duration[task->id];

	/* We might already have an assigned ALAP time with a smaller value */
	if (!(time < my_alap)) return;

	my_alap = time;

	/* Shift data dependent tasks */
	size_t size = task->parents.size();
	for (size_t i = 0; i < size; i++)
		collect_alap(task->parents[i], duration, alap, time);
}
