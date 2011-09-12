#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "common.h"
#include <stdexcept>

class Graph
{
	friend class ListScheduler;

	template<class CT, class PT, class ST>
	friend class GenericGLS;
	friend class SingleObjectiveGLS;
	friend class MultiObjectiveGLS;

	friend class DynamicPower;
	friend class Lifetime;
	friend std::ostream &operator<< (std::ostream &, const Graph *);

	public:

	Graph() : task_count(0), architecture(NULL), duration(0), deadline(0) {}

	void add_task(Task *task);
	void add_link(Task *parent, Task *child);

	void assign_mapping(const Architecture *architecture, const mapping_t &mapping);
	void assign_mapping(const mapping_t &mapping);
	void assign_schedule(const schedule_t &mapping);
	inline void assign_deadline(double time) { deadline = time; }

	inline size_t size() const { return task_count; }
	inline double get_duration() const { return duration; }

	inline const Task *operator[] (tid_t id) const { return tasks[id]; }

	price_t evaluate(Hotspot *hotspot) const;

	void reorder_tasks(const std::vector<tid_t> &order);

	layout_t calc_layout(const Architecture *architecture = NULL) const;
	priority_t calc_priority() const;

	inline const constrains_t &get_constrains()
	{
		if (constrains.empty()) calc_constrains();
		return constrains;
	}

	protected:

	void calc_constrains();

	size_t count_dependents(const Task *task) const;
	size_t count_dependents(const Task *task, bit_string_t &counted) const;
	size_t count_dependencies(const Task *task) const;
	size_t count_dependencies(const Task *task, bit_string_t &counted) const;

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

	task_vector_t tasks;
	size_t task_count;

	const Architecture *architecture;
	mapping_t mapping;
	schedule_t schedule;

	double duration;
	double deadline;

	constrains_t constrains;
};

class GraphBuilder : public Graph
{
	public:

	GraphBuilder(const std::vector<unsigned int> &type,
		const std::vector<std::vector<bool> > &link);
	~GraphBuilder();
};

#endif
