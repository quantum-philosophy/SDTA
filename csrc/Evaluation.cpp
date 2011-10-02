#include "Evaluation.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"
#include "Hotspot.h"
#include "DynamicPower.h"
#include "Schedule.h"

price_t Evaluation::process(const Schedule &schedule)
{
	evaluations++;

	double difference = graph.get_deadline() - schedule.get_duration();

	if (difference < 0) {
		deadline_misses++;
		return price_t(difference, std::numeric_limits<double>::max());
	}

	return compute(schedule);
}

price_t Evaluation::compute(const Schedule &schedule)
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

	double lifetime = this->lifetime.predict(temperature, sampling_interval);
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

std::ostream &operator<<(std::ostream &o, const Evaluation &e)
{
	o
		<< std::setiosflags(std::ios::fixed)
		<< std::setprecision(0)
		<< "Evaluations: " << e.evaluations << std::endl
		<< "  Deadline misses: " << e.deadline_misses
			<< " (" << double(e.deadline_misses) / double(e.evaluations) * 100
			<< "%)" << std::endl
		<< "  Cache hits: " << e.cache_hits
			<< " (" << double(e.cache_hits) / double(e.evaluations) * 100
			<< "%)" << std::endl;

	return o;
}

#ifndef WITHOUT_MEMCACHED

price_t MemcachedEvaluation::compute(const Schedule &schedule)
{
	Digest digest((const unsigned char *)&schedule.trace[0],
		sizeof(step_t) * (extended ? schedule.trace_length : schedule.task_count));

	price_t price, *value;

	if ((value = recall(digest))) {
		cache_hits++;

		price = *value;
		free(value);

#ifdef VERIFY_CACHING
		price_t real_price = Evaluation::compute(schedule);
		if (price != real_price)
			throw std::runtime_error("The caching is broken.");
#endif
	}
	else {
		price = Evaluation::compute(schedule);
		remember(digest, price);
	}

	return price;
}

price_t *MemcachedEvaluation::recall(const Digest &key)
{
	char *value;
	size_t read;
	uint32_t flags;
	memcached_return_t rc;

	value = memcached_get(memcache, (const char *)key.data, Digest::LENGTH,
		&read, &flags, &rc);

	if (!value) return NULL;

	if (read != sizeof(price_t))
		throw std::runtime_error("Cannot read from memcached.");

	return (price_t *)value;
}

void MemcachedEvaluation::remember(const Digest &key, const price_t &price)
{
	memcached_return_t rc;

	rc = memcached_set(memcache, (const char *)key.data, Digest::LENGTH,
		(const char *)&price, sizeof(price_t), (time_t)0, (uint32_t)0);

	if (rc != MEMCACHED_SUCCESS)
		throw std::runtime_error("Cannot interact with memcached.");
}

#endif
