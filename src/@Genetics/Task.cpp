#include "Task.h"
#include "Processor.h"

void Task::map(const Processor *pe)
{
	processor = pe;
	duration = nc * pe->frequency;
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

	time = time + duration;

	/* Shift data dependent tasks */
	size_t size = parents.size();
	for (size_t i = 0; i < size; i++)
		parents[i]->propagate_alap(time);
}
