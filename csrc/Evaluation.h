#ifndef __EVALUATION_H__
#define __EVALUATION_H__

#include "common.h"
#include "Genetics.h"

#ifndef WITHOUT_MEMCACHED
#include <libmemcached/memcached.h>
#include "MD5Digest.h"
#endif

class Evaluation
{
	const Architecture &architecture;
	const Graph &graph;
	const Hotspot &hotspot;

	public:

	size_t evaluations;
	size_t deadline_misses;
	size_t cache_hits;

	Evaluation(const Architecture &_architecture, const Graph &_graph,
		const Hotspot &_hotspot) :

		architecture(_architecture), graph(_graph), hotspot(_hotspot),
		evaluations(0), deadline_misses(0), cache_hits(0) {}

	price_t process(const Schedule &schedule, bool shallow = false);

	protected:

	virtual price_t compute(const Schedule &schedule, bool shallow);
};

std::ostream &operator<<(std::ostream &o, const Evaluation &e);

#ifndef WITHOUT_MEMCACHED

class MemcachedEvaluation: public Evaluation
{
	memcached_st *memcache;

	public:

	MemcachedEvaluation(const std::string &config,
		const Architecture &_architecture, const Graph &_graph,
		const Hotspot &_hotspot) :

		Evaluation(_architecture, _graph, _hotspot)
	{
		memcache = memcached(config.c_str(), config.length());

		if (!memcache)
			throw std::runtime_error("Cannot allocate memcached.");
	}

	~MemcachedEvaluation()
	{
		memcached_free(memcache);
	}

	protected:

	price_t compute(const Schedule &schedule, bool shallow);

	price_t *recall(const MD5Digest &key) const;
	void remember(const MD5Digest &key, const price_t &price) const;
};

#endif

#endif
