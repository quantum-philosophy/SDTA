#include "Evaluation.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"
#include "Hotspot.h"
#include "DynamicPower.h"
#include "Lifetime.h"
#include "Schedule.h"

price_t Evaluation::process(const Schedule &schedule, bool shallow) const
{
	double difference = graph.get_deadline() - schedule.get_duration();

	if (difference < 0)
		return price_t(difference, std::numeric_limits<double>::max());

	double sampling_interval = hotspot.sampling_interval();

	matrix_t dynamic_power, temperature, total_power;

#ifndef FAKE_EVALUATION
	DynamicPower::compute(architecture, graph, schedule,
		sampling_interval, dynamic_power);

	(void)hotspot.solve(architecture, dynamic_power,
		temperature, total_power);
#else
	DynamicPower::compute(architecture, graph, schedule,
		sampling_interval, temperature);
#endif

	double lifetime = Lifetime::predict(temperature, sampling_interval);

	double energy = 0;

#ifndef FAKE_EVALUATION
	if (!shallow) {
		size_t total_count = total_power.cols() * total_power.rows();
		const double *ptr = total_power.pointer();
		for (int i = 0; i < total_count; i++, ptr++) energy += *ptr;
		energy *= sampling_interval;
	}
#endif

	return price_t(lifetime, energy);
}

price_t Evaluation::process(const layout_t &layout,
	const priority_t &priority, bool shallow) const
{
	const Schedule schedule = scheduler.process(layout, priority);

	return process(schedule, shallow);
}
