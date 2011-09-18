#include <stdexcept>

#include "ListScheduler.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"
#include "Schedule.h"

Schedule ListScheduler::process(const Architecture *architecture,
	const Graph *graph, const layout_t layout, const priority_t &priority)
{
	size_t task_count = graph->task_count;
	size_t processor_count = architecture->processor_count;

#ifndef SHALLOW_CHECK
	if (priority.size() != task_count)
		throw std::runtime_error("The priority vector is bad.");

	if (layout.size() != task_count)
		throw std::runtime_error("The layout vector is bad.");
#endif

	const task_vector_t &tasks = graph->tasks;
	const processor_vector_t &processors = architecture->processors;

	bool empty;
	tid_t id, cid;
	pid_t pid;
	const Task *task, *child;
	const Processor *processor;
	size_t i, count;
	double start, duration, finish;

	std::vector<bool> processed(task_count, false);
	std::vector<bool> scheduled(task_count, false);

	std::vector<double> processor_time(processor_count, 0);
	std::vector<double> task_time(task_count, 0);
	std::vector<list_schedule_t> pool(processor_count);

	Schedule schedule(architecture, graph);

	for (id = 0; id < task_count; id++) {
		task = tasks[id];
		if (task->is_root()) {
			push(pool[layout[id]], priority, id);
			processed[id] = true;
		}
	}

	do {
		empty = true;

		/* Check all the processors */
		for (pid = 0; pid < processor_count; pid++) {
			if (pool[pid].empty()) continue;
			empty = false;

			/* Get the next task */
			id = pull(pool[pid], priority);
			task = tasks[id];

			processor = processors[pid];

			/* Calculate its start time and duration */
			start = std::max(processor_time[pid], task_time[id]);
			duration = processor->calc_duration(task->type);
			finish = start + duration;

			processor_time[pid] = finish;

			/* Append to the schedule */
			schedule.append(pid, id, start, duration);
			scheduled[id] = true;

			/* Append children, but only those which are ready,
			 * and ensure absence of repetitions.
			 */
			count = task->children.size();
			for (i = 0; i < count; i++) {
				child = task->children[i];
				cid = child->id;

				/* Shift the child in time with respect to the parent */
				task_time[cid] = std::max(task_time[cid], finish);

				/* Prevent from considering the child once again */
				if (processed[cid]) continue;

				/* All parents should be scheduled */
				if (!ready(child, scheduled)) continue;

				push(pool[layout[cid]], priority, cid);
				processed[cid] = true;
			}
		}
	} while(!empty);

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

inline bool ListScheduler::ready(const Task *task, const bit_string_t &scheduled)
{
	size_t parent_count = task->parents.size();

	for (size_t i = 0; i < parent_count; i++)
		if (!scheduled[task->parents[i]->id]) return false;

	return true;
}
