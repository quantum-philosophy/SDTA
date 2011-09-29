#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "common.h"

class Graph
{
	template<class PT>
	friend class ListScheduler;

	template<class CT, class PT, class ST>
	friend class GenericEvolution;

	friend class SOEvolution;
	friend class MOEvolution;

	friend class DynamicPower;
	friend class Lifetime;
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
