#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include "common.h"

#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"

class Layout
{
	public:

	static layout_t ordinal(const Architecture &architecture,
		const Graph &graph)
	{
		size_t processor_count = architecture.size();
		size_t task_count = graph.size();

		layout_t layout(task_count);

		for (size_t i = 0; i < task_count; i++)
			layout[i] = i % processor_count;

		return layout;
	}

	static layout_t random(const Architecture &architecture,
		const Graph &graph)
	{
		size_t processor_count = architecture.size();
		size_t task_count = graph.size();

		layout_t layout(task_count);

		for (size_t i = 0; i < task_count; i++)
			layout[i] = Random::number(processor_count);

		return layout;
	}

	static layout_t earliest(const Architecture &architecture,
		const Graph &graph, const priority_t &priority);
};

#endif
