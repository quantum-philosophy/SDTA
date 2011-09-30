#include "Layout.h"
#include "ListScheduler.h"
#include "Schedule.h"

layout_t Layout::earliest(const Architecture &architecture,
	const Graph &graph, const priority_t &priority)
{
	size_t task_count = graph.size();

	layout_t layout(task_count);

	EarliestProcessorListScheduler scheduler(architecture, graph);
	Schedule schedule = scheduler.process(layout_t(), priority);

	const step_t * const mapping = schedule.point_mapping();

	for (size_t i = 0; i < task_count; i++) layout[i] = mapping[i];

	return layout;
}
