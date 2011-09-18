#include "Schedule.h"
#include "Hotspot.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"

price_t Schedule::evaluate(const Hotspot &hotspot) const
{
	double sampling_interval = hotspot->sampling_interval();

	matrix_t dynamic_power, temperature, total_power;

	DynamicPower::compute(architecture, graph, *this,
		sampling_interval, dynamic_power);

	(void)hotspot.solve(architecture, dynamic_power,
		temperature, total_power);

	double lifetime = Lifetime::predict(temperature, sampling_interval);

	size_t total_count = total_power.cols() * total_power.rows();
	const double *ptr = total_power.pointer();

	double energy = 0;
	for (int i = 0; i < total_count; i++, ptr++) energy += *ptr;
	energy *= sampling_interval;

	return price_t(lifetime, energy);
}

priority_t Schedule::calc_priority() const
{
	priority_t priority(task_count);
	vector_t mobility = calc_mobility();

#ifndef SHALLOW_CHECK
	if (mobility.size() != task_count)
		throw std::runtime_error("The mobility vector is invalid.");
#endif

	std::vector<std::pair<double, const Task *> > pairs(task_count);

	for (size_t i = 0; i < task_count; i++) {
		pairs[i].first = mobility[i];
		pairs[i].second = graph->tasks[i];
	}

	std::stable_sort(pairs.begin(), pairs.end(),
		Comparator<const Task *>::pair);

	for (size_t i = 0; i < task_count; i++)
		priority[pairs[i].second->id] = i;

	return priority;
}

std::ostream &operator<< (std::ostream &o, const Schedule &schedule)
{
	size_t processor_count = schedule.size();

	o
		<< "Global schedule: " << std::endl
		<< "  Duration: " << schedule.duration() << std::endl
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
