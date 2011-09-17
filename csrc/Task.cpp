#include <iomanip>

#include "Task.h"
#include "Processor.h"

void Task::assign_processor(const Processor *processor)
{
	this->processor = processor;

	duration = processor->calc_duration(type);
	power = processor->calc_power(type);
}

void Task::propagate_start(double time)
{
#ifndef SHALLOW_CHECK
	if (!processor)
		throw std::runtime_error("The task is not assigned to any processor.");
#endif

	/* We might already have an assigned start time with a larger value */
	if (!(start < time)) return;

	start = time;
	time = time + duration;

	/* Shift data dependent tasks */
	size_t size = children.size();
	for (size_t i = 0; i < size; i++)
		children[i]->propagate_start(time);

	/* Shift space dependent tasks (the same core) */
	if (successor) successor->propagate_start(time);
}

void Task::collect_asap(vector_t &asap, double time) const
{
	double &my_asap = asap[id];

	/* We might already have an assigned ASAP time with a larger value */
	if (!(my_asap < time)) return;

	my_asap = time;
	time = time + (duration ? duration : 1);

	/* Shift data dependent tasks */
	size_t size = children.size();
	for (size_t i = 0; i < size; i++)
		children[i]->collect_asap(asap, time);
}

void Task::collect_alap(vector_t &alap, double time) const
{
	double &my_alap = alap[id];

	time = time - (duration ? duration : 1);

	/* We might already have an assigned ALAP time with a smaller value */
	if (!(time < my_alap)) return;

	my_alap = time;

	/* Shift data dependent tasks */
	size_t size = parents.size();
	for (size_t i = 0; i < size; i++)
		parents[i]->collect_alap(alap, time);
}

void Task::collect_depth(std::vector<int> &depth, int level) const
{
	int &my_depth = depth[id];

	/* We might already have an assigned depth with a larger value */
	if (!(my_depth < level)) return;

	my_depth = level++;

	/* Shift data dependent tasks */
	size_t size = children.size();
	for (size_t i = 0; i < size; i++)
		children[i]->collect_depth(depth, level);
}

std::ostream &operator<< (std::ostream &o, const Task *task)
{
	o.precision(2);
	o.flags(std::ios::fixed);

	o	<< std::setw(4) << task->id << " ( "
		<< std::setw(4) << (task->processor ? task->processor->id : -1) << " : "
		<< std::setw(4) << task->type << " : "
		<< std::setw(8) << task->start << " : "
		<< std::setw(8) << task->duration << " ) -> [ ";

	size_t children_count = task->children.size();
	for (size_t i = 0; i < children_count; i++) {
		o << task->children[i]->id;
		if (i + 1 < children_count) o << ", ";
	}

	o << " ]" << std::endl;

	return o;
}
