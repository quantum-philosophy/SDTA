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
			push(pool, priority, id);
			processed[id] = true;
		}

	while (!pool.empty()) {
		/* The pool is always sorted by priority */
		id = pull(pool, priority);
		Task *task = graph->tasks[id];

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

			push(pool, priority, child->id);
			processed[child->id] = true;
		}
	}

	return schedule;
}

inline void ListScheduler::push(list_schedule_t &pool, const priority_t &priority, tid_t id)
{
	list_schedule_t::iterator it;
	rank_t new_priority = priority[id];

	for (it = pool.begin(); it != pool.end(); it++)
		if (new_priority < priority[*it]) break;

	pool.insert(it, id);
}

inline tid_t ListScheduler::pull(list_schedule_t &pool, const priority_t &priority)
{
#ifndef SHALLOW_CHECK
	if (pool.empty())
		throw std::runtime_error("The pool is empty.");
#endif

	tid_t id;

	/* The first one always has the highest priority, although,
	 * it might not be alone.
	 */
	list_schedule_t::iterator it = pool.begin();

#ifndef DETERMINISTIC_LIST_SCHEDULER
	size_t peers, i, choice;
	rank_t highest_priority = priority[*it];

	for (it++, peers = 0; it != pool.end(); it++) {
		if (priority[*it] == highest_priority) {
			peers++;
			continue;
		}
		else if (priority[*it] > highest_priority) break;

#ifndef SHALLOW_CHECK
		throw std::runtime_error("The pool is broken.");
#endif
	}

	it = pool.begin();

	if (peers > 0) {
		choice = Random::number(peers + 1);
		for (i = 0; i < choice; i++) it++;
	}
#endif

	id = *it;
	pool.erase(it);

	return id;
}
