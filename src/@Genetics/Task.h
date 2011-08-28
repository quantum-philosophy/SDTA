#ifndef __TASK_H__
#define __TASK_H__

#include <iostream>
#include <limits>
#include "Common.h"

class Task
{
	friend class Graph;
	friend class Architecture;
	friend class ListScheduler;
	friend class GeneticListScheduler;
	friend std::ostream &operator<< (std::ostream &, const Task *);

	tid_t id;
	unsigned int type;
	unsigned long int nc; /* Number of clock cycles */
	double ceff; /* Effective switched capacitance */

	/* In the graph */
	task_vector_t parents;
	task_vector_t children;

	/* On the same core */
	const Processor *processor;
	Task *ancestor;
	Task *successor;

    double duration;
    double start;		/* Actual start time (mapped and scheduled) */
    double asap; 		/* ASAP, as soon as possible */
    double alap;		/* ALAP, as late as possible */
    double mobility;	/* ALAP - ASAP */

	public:

	Task(unsigned int _type) :
		id(-1), type(_type), nc(0), ceff(0), ancestor(NULL),
		successor(NULL), duration(0), start(-1), asap(-1),
		alap(std::numeric_limits<double>::max()), mobility(0) {}

	void assign_processor(const Processor *processor);

	/* In the graph */
	void add_parent(Task *task) { parents.push_back(task); }
	void add_child(Task *task) { children.push_back(task); }

	/* On the same core */
	void set_ancestor(Task *task) { ancestor = task; }
	void set_successor(Task *task) { successor = task; }

	bool is_leaf() const { return children.empty(); }
	bool is_root() const { return parents.empty(); }

	void propagate_start(double time);
	void propagate_asap(double time);
	void propagate_alap(double time);
};

#endif
