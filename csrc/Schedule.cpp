#include "Schedule.h"
#include "Helper.h"

void Schedule::reorder(const order_t &order)
{
	size_t i, j, count;

	if (order.size() != task_count)
		throw std::runtime_error("Cannot reorder the schedule.");

	for (pid_t pid = 0; pid < processor_count; pid++) {
		count = schedules[pid].size();
		for (i = 0; i < count; i++) {
			ScheduleItem &item = schedules[pid][i];

			for (j = 0; j < task_count; j++)
				if (order[j] == item.id) {
					item.id = j;
					break;
				}

			if (j == task_count)
				throw std::runtime_error("Cannot reorder the schedule.");
		}
	}

	Helper::permute(point_order(), order);
	Helper::permute(point_mapping(), order);
}

std::ostream &operator<< (std::ostream &o, const Schedule &schedule)
{
	o
		<< std::setprecision(6) << std::setiosflags(std::ios::fixed)

		<< "  Duration: " << schedule.get_duration() << std::endl
		<< "  "
			<< std::setw(4) << "id" << " ( "
			<< std::setw(4) << "proc" << " : "
			<< std::setw(10) << "start" << " : "
			<< std::setw(10) << "duration" << " )" << std::endl;

	size_t i, count;
	size_t task_count = schedule.tasks();
	size_t processor_count = schedule.processors();

	for (tid_t id = 0; id < task_count; id++) {
		pid_t pid = schedule.map(id);
		const LocalSchedule &local_schedule = schedule[pid];
		count = local_schedule.size();

		for (i = 0; i < count; i++)
			if (id == local_schedule[i].id) break;

		const ScheduleItem &item = local_schedule[i];

		o	<< "  "
			<< std::setw(4) << id << " ( "
			<< std::setw(4) << pid << " : "
			<< std::setw(10) << item.start << " : "
			<< std::setw(10) << item.duration << " )" << std::endl;
	}

	o << std::endl << "Processor load:" << std::endl;

	for (i = 0; i < processor_count; i++) {
		count = schedule[i].size();
		o	<< std::setw(4) << i << " -> " << count << " tasks ("
			<< std::setprecision(0) << (double(count) / double(task_count) * 100)
			<< "%)" << std::endl;
	}

	return o;
}
