#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "Common.h"

class Graph
{
	friend class ListScheduler;

	task_vector_t tasks;
	processor_vector_t processors;

	size_t task_count;
	size_t processor_count;

	bool mapped;
	bool scheduled;

	mapping_t mapping;
	schedule_t schedule;

	public:

	Graph() :
		task_count(0), processor_count(0),
		scheduled(false), mapped(false) {}

	void add_task(Task *task);
	void add_link(Task *parent, Task *child);
	void add_processor(Processor *processor);

	void assign_mapping(const mapping_t &mapping);
	void assign_schedule(const schedule_t &mapping);

	task_vector_t get_roots() const;
	task_vector_t get_leaves() const;

	/* Build the whole graph with tasks and processors */
	static Graph *build(std::vector<unsigned long int> &nc,
		std::vector<double> &ceff, std::vector<std::vector<bool> > &link,
		std::vector<double> &frequency, std::vector<double> &voltage,
		std::vector<unsigned long int> &ngate);

	private:

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

#endif
