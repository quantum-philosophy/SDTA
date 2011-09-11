#ifndef __LIST_SCHEDULER_H__
#define __LIST_SCHEDULER_H__

#include "common.h"

class ListScheduler
{
	public:

	ListScheduler() {}

	static schedule_t process(const Graph *graph);
	static schedule_t process(const Graph *graph, const priority_t &priority);

	private:

	static inline void insert_into_pool(list_schedule_t &pool, tid_t id,
		const priority_t &priority);
};

#endif
