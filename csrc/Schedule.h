#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include "common.h"

#ifdef REAL_RANK
#error Not implemented yet
#else
typedef int step_t;
typedef std::vector<step_t> trace_t;
#endif

struct ScheduleItem
{
	tid_t id;
	double start;
	double duration;

	ScheduleItem(tid_t _id, double _start, double _duration) :
		id(_id), start(_start), duration(_duration) {}
};

typedef std::vector<ScheduleItem> LocalSchedule;

class Schedule
{
	friend class GeneEncoder;
#ifndef WITHOUT_MEMCACHED
	friend class MemcachedEvaluation;
#endif
	friend std::ostream &operator<< (std::ostream &o,
		const Schedule &schedule);

	size_t processor_count;
	size_t task_count;
	size_t append_count;

	std::vector<LocalSchedule> schedules;

	double duration;

	/* The first half is for the schedule order,
	 * the second one is for the mapping.
	 */
	size_t trace_length;
	trace_t trace;

	public:

	Schedule() : processor_count(0), task_count(0), append_count(0) {}

	Schedule(size_t _processor_count, size_t _task_count) :

		processor_count(_processor_count), task_count(_task_count), append_count(0),
		schedules(std::vector<LocalSchedule>(processor_count)), duration(0),
		trace_length(2 * task_count), trace(trace_length, 0) {}

	inline bool empty() const
	{
		return append_count != task_count;
	}

	inline size_t processors() const
	{
		return processor_count;
	}

	inline size_t tasks() const
	{
		return task_count;
	}

	inline const step_t *point_order() const
	{
		return &trace[0];
	}

	inline const step_t *point_mapping() const
	{
		return &trace[task_count];
	}

	inline step_t *point_order()
	{
		return &trace[0];
	}

	inline step_t *point_mapping()
	{
		return &trace[task_count];
	}

	inline pid_t map(tid_t id) const
	{
#ifndef SHALLOW_CHECK
		if (id >= task_count)
			throw std::runtime_error("Cannot find the task.");
#endif

		return point_mapping()[id];
	}

	inline const LocalSchedule &operator[](pid_t pid) const
	{
		return schedules[pid];
	}

	inline void append(pid_t pid, tid_t tid, double start, double duration)
	{
		schedules[pid].push_back(ScheduleItem(tid, start, duration));

		double end = start + duration;
		if (this->duration < end) this->duration = end;

		point_mapping()[tid] = pid;

#ifndef SHALLOW_CHECK
		if (append_count + 1 > task_count)
			throw std::runtime_error("There are too many tasks.");
#endif

		point_order()[append_count++] = tid;
	}

	inline double get_duration() const
	{
		return duration;
	}

	inline const priority_t get_priority() const
	{
		priority_t priority;

		for (size_t i = 0; i < task_count; i++)
			priority[trace[i]] = (rank_t)i;

		return priority;
	}

	void reorder(const order_t &order);
};

std::ostream &operator<< (std::ostream &o, const Schedule &schedule);

#endif
