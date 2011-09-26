#include "ListScheduler.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"
#include "Schedule.h"

Schedule ListScheduler::process(const layout_t &layout,
	const priority_t &priority, void *data) const
{
	size_t processor_count = processors.size();
	size_t task_count = tasks.size();

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

	Schedule schedule(processor_count, task_count);

	for (id = 0; id < task_count; id++) {
		task = tasks[id];
		if (task->is_root()) {
			push(pool, layout, priority, id, data);
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
			id = pull(pool, layout, priority, pid, data);
			task = tasks[id];

			processor = processors[pid];

			/* Calculate its start time and duration */
			start = std::max(processor_time[pid], task_time[id]);
			duration = processor->calc_duration(task->get_type());
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

				push(pool, layout, priority, cid, data);
				processed[cid] = true;
			}
		}
	}
	while(!empty);

	return schedule;
}

bool ListScheduler::ready(const Task *task, const bit_string_t &scheduled) const
{
	size_t parent_count = task->parents.size();

	for (size_t i = 0; i < parent_count; i++)
		if (!scheduled[task->parents[i]->id]) return false;

	return true;
}

void DeterministicListScheduler::push(pool_t &pool, const layout_t &layout,
	const priority_t &priority, tid_t id, void *data) const
{
	list_schedule_t &local_pool = pool[layout[id]];

	list_schedule_t::iterator it;
	rank_t new_priority = priority[id];

	for (it = local_pool.begin(); it != local_pool.end(); it++)
		if (new_priority < priority[*it]) break;

	local_pool.insert(it, id);
}

tid_t DeterministicListScheduler::pull(pool_t &pool, const layout_t &layout,
	const priority_t &priority, pid_t pid, void *data) const
{
	list_schedule_t &local_pool = pool[pid];

	tid_t id;

	/* The first one always has the highest priority, although,
	 * it might not be alone.
	 */
	list_schedule_t::iterator it = local_pool.begin();

	id = *it;
	local_pool.erase(it);

	return id;
}

void StochasticListScheduler::push(pool_t &pool, const layout_t &layout,
	const priority_t &priority, tid_t id, void *data) const
{
	list_schedule_t &local_pool = pool[layout[id]];

	list_schedule_t::iterator it;
	rank_t new_priority = priority[id];

	for (it = local_pool.begin(); it != local_pool.end(); it++)
		if (new_priority < priority[*it]) break;

	local_pool.insert(it, id);
}

tid_t StochasticListScheduler::pull(pool_t &pool, const layout_t &layout,
	const priority_t &priority, pid_t pid, void *data) const
{
	list_schedule_t &local_pool = pool[pid];

	tid_t id;

	/* The first one always has the highest priority, although,
	 * it might not be alone.
	 */
	list_schedule_t::iterator it = local_pool.begin();

	size_t peers, i, choice;
	rank_t highest_priority = priority[*it];

	for (it++, peers = 0; it != local_pool.end(); it++) {
		if (priority[*it] == highest_priority) {
			peers++;
			continue;
		}
		else if (priority[*it] > highest_priority) break;

#ifndef SHALLOW_CHECK
		throw std::runtime_error("The pool is broken.");
#endif
	}

	it = local_pool.begin();

	if (peers > 0) {
		choice = Random::number(peers + 1);
		for (i = 0; i < choice; i++) it++;
	}

	id = *it;
	local_pool.erase(it);

	return id;
}

void RandomGeneratorListScheduler::push(pool_t &pool, const layout_t &layout,
	const priority_t &dummy, tid_t id, void *data) const
{
	list_schedule_t &local_pool = pool[layout[id]];

	local_pool.push_back(id);
}

tid_t RandomGeneratorListScheduler::pull(pool_t &pool, const layout_t &layout,
	const priority_t &dummy, pid_t pid, void *data) const
{
	list_schedule_t &local_pool = pool[pid];

	tid_t id;

	list_schedule_t::iterator it = local_pool.begin();

	size_t choice = Random::number(local_pool.size());
	for (size_t i = 0; i < choice; i++) it++;

	id = *it;
	local_pool.erase(it);

	return id;
}
