#include "Evaluation.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"
#include "Hotspot.h"
#include "DynamicPower.h"
#include "Lifetime.h"
#include "Schedule.h"
#include "ListScheduler.h"

price_t Evaluation::process(const Schedule &schedule, bool shallow) const
{
	double sampling_interval = hotspot.sampling_interval();

	matrix_t dynamic_power, temperature, total_power;

	DynamicPower::compute(architecture, graph, schedule,
		sampling_interval, dynamic_power);

	(void)hotspot.solve(architecture, dynamic_power,
		temperature, total_power);

	double lifetime = Lifetime::predict(temperature, sampling_interval);

	double energy = 0;

	if (!shallow) {
		size_t total_count = total_power.cols() * total_power.rows();
		const double *ptr = total_power.pointer();
		for (int i = 0; i < total_count; i++, ptr++) energy += *ptr;
		energy *= sampling_interval;
	}

	return price_t(lifetime, energy);
}

price_t Evaluation::process(const layout_t &layout,
	const priority_t &priority, bool shallow) const
{
	Schedule schedule = ListScheduler::process(
		architecture, graph, layout, priority);

	if (schedule.get_duration() > graph.get_deadline())
		return price_t::invalid();

	return process(schedule, shallow);
}
