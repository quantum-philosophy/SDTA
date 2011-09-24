#ifndef __LIST_SCHEDULER_H__
#define __LIST_SCHEDULER_H__

#include "common.h"
#include "Architecture.h"
#include "Graph.h"

typedef std::list<tid_t> list_schedule_t;

class ListScheduler
{
	const processor_vector_t &processors;
	const task_vector_t &tasks;

	public:

	ListScheduler(const Architecture &architecture, const Graph &graph) :
		processors(architecture.processors), tasks(graph.tasks) {}

	Schedule process(const layout_t &layout, const priority_t &priority) const;

	protected:

	virtual void push(list_schedule_t &pool,
		const priority_t &priority, tid_t id) const = 0;
	virtual tid_t pull(list_schedule_t &pool,
		const priority_t &priority) const = 0;

	bool ready(const Task *task, const bit_string_t &scheduled) const;
};

class DeterministicListScheduler: public ListScheduler
{
	public:

	DeterministicListScheduler(const Architecture &architecture,
		const Graph &graph) : ListScheduler(architecture, graph) {}

	protected:

	void push(list_schedule_t &pool, const priority_t &priority, tid_t id) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority) const;
};

class StochasticListScheduler: public ListScheduler
{
	public:

	StochasticListScheduler(const Architecture &architecture,
		const Graph &graph) : ListScheduler(architecture, graph) {}

	void push(list_schedule_t &pool, const priority_t &priority, tid_t id) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority) const;
};

class RandomListScheduler: public ListScheduler
{
	public:

	RandomListScheduler(const Architecture &architecture,
		const Graph &graph) : ListScheduler(architecture, graph) {}

	void push(list_schedule_t &pool, const priority_t &priority, tid_t id) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority) const;
};

#endif
