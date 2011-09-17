#include <stdexcept>

#include "ListScheduler.h"
#include "Graph.h"
#include "Task.h"

schedule_t ListScheduler::process(const Graph *graph, const priority_t &priority)
{
	tid_t id;
	size_t index = 0, task_count = graph->task_count;

#ifndef SHALLOW_CHECK
	if (priority.size() != task_count)
		throw std::runtime_error("The priority vector is bad.");
#endif

	list_schedule_t pool;
	schedule_t schedule(task_count);
	std::vector<bool> scheduled(task_count, false);
	std::vector<bool> processed(task_count, false);

	/* Obtain all roots and place them into the list according to
	 * their priority (the lower number, the higher priority).
	 */
	for (id = 0; id < task_count; id++)
		if (graph->tasks[id]->is_root()) {
			insert_into_pool(pool, id, priority);
			processed[id] = true;
		}

	while (!pool.empty()) {
		/* The pool is always sorted by priority */
		id = pool.front();
		Task *task = graph->tasks[id];
		pool.pop_front();

		/* Append to the schedule */
		schedule[index++] = id;
		scheduled[id] = true;

		/* Append new tasks, but only ready ones, and ensure absence
		 * of any repetitions.
		 */
		size_t children_count = task->children.size();
		for (size_t i = 0; i < children_count; i++) {
			Task *child = task->children[i];

			/* Prevent from doing it once again */
			if (processed[child->id]) continue;

			/* All parents should be scheduled */
			bool ready = true;
			size_t parent_count = child->parents.size();
			for (size_t j = 0; j < parent_count; j++)
				if (!scheduled[child->parents[j]->id]) {
					ready = false;
					break;
				}
			if (!ready) continue;

			insert_into_pool(pool, child->id, priority);
			processed[child->id] = true;
		}
	}

	return schedule;
}

inline void ListScheduler::insert_into_pool(list_schedule_t &pool, tid_t id,
	const priority_t &priority)
{
	size_t equal = 0;
	list_schedule_t::iterator it;
	rank_t new_priority = priority[id], another_priority;

	/* Find a place */
	for (it = pool.begin(); it != pool.end(); it++) {
		another_priority = priority[*it];

		/* Looking for a lower priority (larger number) */
		if (new_priority < another_priority) break;
		else if (new_priority == another_priority) {
			equal++;
			continue;
		}

#ifndef SHALLOW_CHECK
		if (equal > 0)
			throw std::runtime_error("Something went wrong with the LS.");
#endif
	}

	/* Insert! */
#ifndef DETERMINISTIC_LIST_SCHEDULER
	size_t go_back = Random::number(equal + 1);
	for (size_t i = 0; i < go_back; i++) it--;
#endif
	pool.insert(it, id);
}
