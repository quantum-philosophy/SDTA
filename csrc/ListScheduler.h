#ifndef __LIST_SCHEDULER_H__
#define __LIST_SCHEDULER_H__

#include "common.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"
#include "Schedule.h"
#include "Pool.h"

class BasicListScheduler
{
	public:

	virtual Schedule process(const layout_t &layout, const priority_t &priority,
		void *data = NULL) const = 0;
};

template<class PT>
class ListScheduler: public BasicListScheduler
{
	protected:

	const processor_vector_t &processors;
	const task_vector_t &tasks;

	public:

	ListScheduler(const Architecture &architecture, const Graph &graph) :
		processors(architecture.processors), tasks(graph.tasks) {}

	Schedule process(const layout_t &layout, const priority_t &priority,
		void *data = NULL) const;

	protected:

	inline bool ready(const Task *task, const bit_string_t &scheduled) const
	{
		size_t parent_count = task->parents.size();

		for (size_t i = 0; i < parent_count; i++)
			if (!scheduled[task->parents[i]->id]) return false;

		return true;
	}
};

typedef ListScheduler<DeterministicPool> DeterministicListScheduler;
typedef ListScheduler<RandomPool> RandomGeneratorListScheduler;
typedef ListScheduler<EarliestProcessorPool> EarliestProcessorListScheduler;

#include "ListScheduler.hpp"

#endif
