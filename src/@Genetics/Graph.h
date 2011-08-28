#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <iostream>
#include "Common.h"

class Graph
{
	friend class ListScheduler;
	friend class GeneticListScheduler;
	friend std::ostream &operator<< (std::ostream &, const Graph *);

	protected:

	task_vector_t tasks;
	size_t task_count;

	const Architecture *architecture;
	mapping_t mapping;
	schedule_t schedule;

	public:

	Graph() : task_count(0), architecture(NULL) {}

	void add_task(Task *task);
	void add_link(Task *parent, Task *child);

	void assign_mapping(const Architecture *architecture,
		const mapping_t &mapping);
	void assign_schedule(const schedule_t &mapping);

	task_vector_t get_roots() const;
	task_vector_t get_leaves() const;

	size_t size() const { return task_count; }

	const Task *operator[] (tid_t id) const { return tasks[id]; }

	protected:

	/* The duration of the graph based on the actual start times */
	double calc_duration() const;
	/* The duration of the graph based on the ASAP times */
	double calc_asap_duration() const;

	/* Trigger the propagation of the start time */
	void calc_start() const;
	/* Trigger the propagation of the ASAP time */
	void calc_asap() const;
	/* Trigger the propagation of the ALAP time */
	void calc_alap() const;
};

class GraphBuilder : public Graph
{
	public:

	GraphBuilder(std::vector<unsigned int> &type,
		std::vector<std::vector<bool> > &link);
	~GraphBuilder();
};

#endif
