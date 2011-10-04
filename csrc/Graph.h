#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "common.h"

class Graph
{
	template<class PT>
	friend class ListScheduler;

	friend class SOEvolution;
	friend class MOEvolution;

	friend class Lifetime;
	friend class SteadyStateHotspot;

	friend std::ostream &operator<< (std::ostream &, const Graph *);

	public:

	Graph() : task_count(0), deadline(0) {}

	void add_task(Task *task);
	void add_link(Task *parent, Task *child);

	inline void set_deadline(double time)
	{
		deadline = time;
	}

	inline double get_deadline() const
	{
		return deadline;
	}

	inline size_t size() const
	{
		return task_count;
	}

	inline const Task *operator[] (tid_t id) const
	{
		return tasks[id];
	}

	inline const task_vector_t &get_tasks() const
	{
		return tasks;
	}

	void reorder(const order_t &order);

	protected:

	task_vector_t tasks;
	size_t task_count;

	double deadline;
};

class GraphBuilder: public Graph
{
	public:

	GraphBuilder(const std::vector<unsigned int> &type,
		const std::vector<std::vector<bool> > &link);
	~GraphBuilder();
};

#endif
