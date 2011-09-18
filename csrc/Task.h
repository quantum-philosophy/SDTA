#ifndef __TASK_H__
#define __TASK_H__

#include "common.h"

class Task
{
	friend class Graph;
	friend class Architecture;
	friend class ListScheduler;

	template<class CT, class PT, class ST>
	friend class GenericEvolution;

	friend class DynamicPower;
	friend std::ostream &operator<< (std::ostream &, const Task *);

	tid_t id;
	unsigned int type;

	/* In the graph */
	task_vector_t parents;
	task_vector_t children;

	public:

	Task(unsigned int _type) :
		id(-1), type(_type), ancestor(NULL), successor(NULL) {}

	inline void add_parent(Task *task)
	{
		parents.push_back(task);
	}

	inline void add_child(Task *task)
	{
		children.push_back(task);
	}

	inline bool is_leaf() const
	{
		return children.empty();
	}

	inline bool is_root() const
	{
		return parents.empty();
	}
};

#endif
