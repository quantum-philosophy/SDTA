#ifndef __TASK_H__
#define __TASK_H__

#include <limits>
#include "common.h"

class Task
{
	friend class Graph;
	friend class Architecture;
	friend class ListScheduler;

	template<class chromosome_t, class stats_t>
	friend class GenericGLS;

	friend class DynamicPower;
	friend std::ostream &operator<< (std::ostream &, const Task *);

	tid_t id;
	unsigned int type;

	/* In the graph */
	task_vector_t parents;
	task_vector_t children;

	/* On the same core */
	const Processor *processor;
	Task *ancestor;
	Task *successor;

	/* Assigned on mapping */
    double duration;	/* Execution time */
	double power; 		/* Consuming power */

    double start;		/* Actual start time (mapped and scheduled) */
    double asap; 		/* ASAP, as soon as possible */
    double alap;		/* ALAP, as late as possible */
    double mobility;	/* ALAP - ASAP */

	public:

	Task(unsigned int _type) :
		id(-1), type(_type), ancestor(NULL), successor(NULL),
		duration(0), power(0), start(-1), asap(-1),
		alap(std::numeric_limits<double>::max()), mobility(0) {}

	void assign_processor(const Processor *processor);

	/* In the graph */
	inline void add_parent(Task *task) { parents.push_back(task); }
	inline void add_child(Task *task) { children.push_back(task); }

	/* On the same core */
	inline void set_ancestor(Task *task) { ancestor = task; }
	inline void set_successor(Task *task) { successor = task; }
	inline void set_order(Task *_ancestor = NULL, Task *_successor = NULL)
	{
		ancestor = _ancestor;
		successor = _successor;
		start = -1;
	}

	inline bool is_leaf() const { return children.empty(); }
	inline bool is_root() const { return parents.empty(); }

	static bool compare_mobility(const Task *one, const Task *another)
	{
		return one->mobility < another->mobility;
	}

	private:

	void propagate_start(double time);
	void propagate_asap(double time);
	void propagate_alap(double time);
};

#endif
