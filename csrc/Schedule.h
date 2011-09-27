#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include "common.h"

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
	friend class Evaluation;
	friend std::ostream &operator<< (std::ostream &o,
		const Schedule &schedule);

	size_t processor_count;
	size_t task_count;
	size_t append_count;

	std::vector<LocalSchedule> schedules;

	mapping_t mapping;
	order_t order;

	double duration;

	public:

	Schedule() : processor_count(0), task_count(0), duration(0) {}

	Schedule(size_t _processor_count, size_t _task_count) :

		processor_count(_processor_count), task_count(_task_count), append_count(0),
		schedules(std::vector<LocalSchedule>(processor_count)),
		mapping(mapping_t(task_count, 0)), order(order_t(task_count, 0)),
		duration(0) {}

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

	inline pid_t map(tid_t id) const
	{
#ifndef SHALLOW_CHECK
		if (id >= mapping.size())
			throw std::runtime_error("Cannot find the task.");
#endif

		return mapping[id];
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

		mapping[tid] = pid;

#ifndef SHALLOW_CHECK
		if (append_count + 1 > task_count)
			throw std::runtime_error("There are too many tasks.");
#endif

		order[append_count++] = tid;
	}

	inline double get_duration() const
	{
		return duration;
	}

	const order_t &get_order() const
	{
		return order;
	}

	void reorder(const order_t &order);
};

std::ostream &operator<< (std::ostream &o, const Schedule &schedule);

#endif
