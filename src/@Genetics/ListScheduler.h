#ifndef __LIST_SCHEDULER_H__
#define __LIST_SCHEDULER_H__

#include "Common.h"

typedef std::list<unsigned int> pool_t;

class ListScheduler
{
	public:

	ListScheduler() {}

	static schedule_t process(const Graph *graph);
	static schedule_t process(const Graph *graph, const priority_t &priority);

	private:

	static inline void insert_into_pool(pool_t &pool, tid_t id,
		const priority_t &priority);
};

#endif
