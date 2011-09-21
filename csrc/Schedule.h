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
	friend std::ostream &operator<< (std::ostream &o,
		const Schedule &schedule);

	size_t processor_count;
	size_t task_count;

	std::vector<LocalSchedule> schedules;

	mapping_t mapping;
	double duration;

	public:

	Schedule() : processor_count(0), task_count(0), duration(0) {}

	Schedule(size_t _processor_count, size_t _task_count) :

		processor_count(_processor_count), task_count(_task_count),
		schedules(std::vector<LocalSchedule>(processor_count)),
		mapping(mapping_t(task_count, -1)), duration(0) {}

	inline size_t size() const
	{
		return processor_count;
	}

	inline const LocalSchedule &operator[](pid_t pid) const
	{
		return schedules[pid];
	}

	inline void append(pid_t pid, tid_t tid, double start, double duration)
	{
		schedules[pid].push_back(ScheduleItem(tid, start, duration));

		if (this->duration < start + duration)
			this->duration = start + duration;

		mapping[tid] = pid;
	}

	inline double get_duration() const
	{
		return duration;
	}

	std::vector<size_t> flatten() const;

	void reorder(const std::vector<size_t> &order);
};

std::ostream &operator<< (std::ostream &o, const Schedule &schedule);

#endif
