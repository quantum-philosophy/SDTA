#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "common.h"
#include <stdexcept>

class Graph
{
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

	inline void finalize()
	{
		set_constrains();
	}

	inline size_t size() const
	{
		return task_count;
	}

	inline const Task *operator[] (tid_t id) const
	{
		return tasks[id];
	}

	inline const constrains_t &get_constrains() const
	{
		return constrains;
	}

	protected:

	void set_constrains();

	size_t count_dependents(const Task *task) const;
	size_t count_dependents(const Task *task, bit_string_t &counted) const;
	size_t count_dependencies(const Task *task) const;
	size_t count_dependencies(const Task *task, bit_string_t &counted) const;

	task_vector_t tasks;
	size_t task_count;

	double deadline;

	constrains_t constrains;
};

class GraphBuilder: public Graph
{
	public:

	GraphBuilder(const std::vector<unsigned int> &type,
		const std::vector<std::vector<bool> > &link);
	~GraphBuilder();
};

#endif
