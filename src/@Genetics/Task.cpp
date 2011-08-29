#include <stdexcept>
#include <iomanip>

#include "Task.h"
#include "Processor.h"

void Task::assign_processor(const Processor *processor)
{
	if (processor->type_count < type + 1)
		throw std::runtime_error("The processor does not have such type.");

	this->processor = processor;

	/* t = NC / f */
	duration = processor->nc[type] / processor->frequency;

	/* P = Ceff * f * V^2 */
	power = processor->ceff[type] * processor->frequency *
		processor->voltage * processor->voltage;
}

void Task::propagate_start(double time)
{
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

void Task::propagate_asap(double time)
{
	/* We might already have an assigned ASAP time with a larger value */
	if (!(asap < time)) return;

	asap = time;
	time = time + duration;

	/* Shift data dependent tasks */
	size_t size = children.size();
	for (size_t i = 0; i < size; i++)
		children[i]->propagate_asap(time);
}

void Task::propagate_alap(double time)
{
	time = std::max(0.0, time - duration);

	/* We might already have an assigned ALAP time with a smaller value */
	if (!(time < alap)) return;

	alap = time;
	mobility = std::max(0.0, alap - asap);

	/* Shift data dependent tasks */
	size_t size = parents.size();
	for (size_t i = 0; i < size; i++)
		parents[i]->propagate_alap(time);
}

std::ostream &operator<< (std::ostream &o, const Task *task)
{
	o.precision(2);
	o.flags(std::ios::fixed);

	o	<< std::setw(4) << task->id << " ( "
		<< std::setw(4) << (task->processor ? task->processor->id : -1) << " : "
		<< std::setw(4) << task->type << " : "
		<< std::setw(8) << task->start << " : "
		<< std::setw(8) << task->duration << " : "
		<< std::setw(8) << task->asap << " : "
		<< std::setw(8) << task->mobility << " : "
		<< std::setw(8) << task->alap << " ) -> [ ";

	size_t children_count = task->children.size();
	for (size_t i = 0; i < children_count; i++) {
		o << task->children[i]->id;
		if (i + 1 < children_count) o << ", ";
	}

	o << " ] " << std::endl;

	return o;
}
