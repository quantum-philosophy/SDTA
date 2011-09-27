#include "Evaluation.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"
#include "Hotspot.h"
#include "DynamicPower.h"
#include "Lifetime.h"
#include "Schedule.h"

price_t Evaluation::process(const Schedule &schedule, bool shallow)
{
	double difference = graph.get_deadline() - schedule.get_duration();

	if (difference < 0) {
		deadline_misses++;

		return price_t(difference, std::numeric_limits<double>::max());
	}

	MD5Digest digest(schedule.order);

	price_t price, *value;

	if ((value = recall(digest))) {
		cache_hits++;

		price = *value;
		free(value);
	}
	else {
		cache_misses++;

		price = compute(schedule, shallow);
		remember(digest, price);
	}

	return price;
}

price_t Evaluation::compute(const Schedule &schedule, bool shallow) const
{
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

price_t *Evaluation::recall(const MD5Digest &key) const
{
	char *value;
	size_t read;
	uint32_t flags;
	memcached_return_t rc;

	value = memcached_get(memcache, (const char *)key.data, MD5_LENGTH,
		&read, &flags, &rc);

	if (!value) return NULL;

	if (read != sizeof(price_t))
		throw std::runtime_error("Cannot read from memcached.");

	return (price_t *)value;
}

void Evaluation::remember(const MD5Digest &key, const price_t &price) const
{
	memcached_return_t rc;

	rc = memcached_set(memcache, (const char *)key.data, MD5_LENGTH,
		(const char *)&price, sizeof(price_t), (time_t)0, (uint32_t)0);

	if (rc != MEMCACHED_SUCCESS)
		throw std::runtime_error("Cannot interact with memcached.");
}
