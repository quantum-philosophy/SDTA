#include "Schedule.h"

order_t Schedule::flatten() const
{
	order_t order(task_count);

	size_t index = 0;

	for (pid_t pid = 0; pid < processor_count; pid++) {
		size_t count = schedules[pid].size();
		for (size_t i = 0; i < count; i++)
			order[index++] = schedules[pid][i].id;
	}

	if (index != task_count)
		throw std::runtime_error("Cannot flatten the schedule.");

	return order;
}

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
}

std::ostream &operator<< (std::ostream &o, const Schedule &schedule)
{
	size_t processor_count = schedule.processors();

	o
		<< "  Duration: " << schedule.get_duration() << std::endl
		<< "  "
			<< std::setw(4) << "id" << " ( "
			<< std::setw(4) << "proc" << " : "
			<< std::setw(8) << "start" << " : "
			<< std::setw(8) << "duration" << " )" << std::endl;

	for (pid_t pid = 0; pid < processor_count; pid++) {
		const LocalSchedule &local_schedule = schedule[pid];
		size_t task_count = local_schedule.size();

		for (size_t i = 0; i < task_count; i++) {
			const ScheduleItem &item = local_schedule[i];

			o	<< "  "
				<< std::setw(4) << item.id << " ( "
				<< std::setw(4) << pid << " : "
				<< std::setw(8) << item.start << " : "
				<< std::setw(8) << item.duration << " )" << std::endl;
		}
	}

	return o;
}
