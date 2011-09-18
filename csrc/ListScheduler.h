#ifndef __LIST_SCHEDULER_H__
#define __LIST_SCHEDULER_H__

#include "common.h"

class ListScheduler
{
	public:

	ListScheduler() {}

	GlobalSchedule process(const Architecture *architecture,
		const Graph *graph, const layout_t layout, const priority_t &priority);

	static schedule_t process(const Graph *graph, const priority_t &priority);

	private:

	static inline void push(list_schedule_t &pool, const priority_t &priority, tid_t id);
	static inline tid_t pull(list_schedule_t &pool, const priority_t &priority);
	static inline bool ready(const Task *task, const bit_string_t &scheduled);
};

#endif
