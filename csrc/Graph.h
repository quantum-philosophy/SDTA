#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "common.h"

class Graph
{
	friend class ListScheduler;

	template<class chromosome_t>
	friend class GenericGLS;
	friend class SingleObjectiveGLS;
	friend class MultiObjectiveGLS;

	friend class DynamicPower;
	friend class Lifetime;
	friend std::ostream &operator<< (std::ostream &, const Graph *);

	protected:

	task_vector_t tasks;
	size_t task_count;

	const Architecture *architecture;
	mapping_t mapping;
	schedule_t schedule;

	double duration;
	double deadline;

	public:

	Graph() : task_count(0), architecture(NULL), duration(0), deadline(0) {}

	void add_task(Task *task);
	void add_link(Task *parent, Task *child);

	void assign_mapping(const Architecture *architecture,
		const mapping_t &mapping);
	void assign_schedule(const schedule_t &mapping);
	inline void assign_deadline(double time) { deadline = time; }

	task_vector_t get_roots() const;
	task_vector_t get_leaves() const;

	inline size_t size() const { return task_count; }
	inline double get_duration() const { return duration; }

	const Task *operator[] (tid_t id) const { return tasks[id]; }

	price_t evaluate(Hotspot *hotspot) const;

	priority_t calc_priority() const;

	void reorder_tasks(const std::vector<tid_t> &order);

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

	void fix_epsilon() const;
};

class GraphBuilder : public Graph
{
	public:

	GraphBuilder(std::vector<unsigned int> &type,
		std::vector<std::vector<bool> > &link);
	~GraphBuilder();
};

#endif
