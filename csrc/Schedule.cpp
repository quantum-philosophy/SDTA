#include "Schedule.h"

std::ostream &operator<< (std::ostream &o, const Schedule &schedule)
{
	size_t processor_count = schedule.size();

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
