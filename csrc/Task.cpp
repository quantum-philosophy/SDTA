#include <iomanip>

#include "Task.h"

std::ostream &operator<< (std::ostream &o, const Task *task)
{
	o.precision(2);
	o.flags(std::ios::fixed);

	o << std::setw(4) << task->id << " -> [ ";

	size_t children_count = task->children.size();
	for (size_t i = 0; i < children_count; i++) {
		o << task->children[i]->id;
		if (i + 1 < children_count) o << ", ";
	}

	o << " ]" << std::endl;

	return o;
}
