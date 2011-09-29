#include "Pool.h"

Pool::Pool(size_t _processor_count, size_t _task_count, const layout_t &_layout,
	const priority_t &_priority, void *_data) :

	processor_count(_processor_count), task_count(_task_count),
	processor_time(_processor_count, 0), task_time(_task_count, 0),
	processed(_task_count, false), scheduled(_task_count, false),
	layout(_layout), priority(_priority)
{
}
