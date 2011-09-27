#ifndef __EVALUATION_H__
#define __EVALUATION_H__

#include "common.h"
#include "Genetics.h"

#include <libmemcached/memcached.h>
#include "MD5Digest.h"

class Evaluation
{
	const Architecture &architecture;
	const Graph &graph;
	const Hotspot &hotspot;

	memcached_st *memcache;

	public:

	size_t cache_hits;
	size_t cache_misses;
	size_t deadline_misses;

	Evaluation(const Architecture &_architecture, const Graph &_graph,
		const Hotspot &_hotspot) :

		architecture(_architecture), graph(_graph), hotspot(_hotspot),
		cache_hits(0), cache_misses(0), deadline_misses(0)
	{
		const char *config_string = "--SERVER=localhost";
		memcache = memcached(config_string, strlen(config_string));

		if (!memcache)
			throw std::runtime_error("Cannot allocate memcached.");
	}

	~Evaluation()
	{
		memcached_free(memcache);
	}

	price_t process(const Schedule &schedule, bool shallow = false);

	private:

	price_t compute(const Schedule &schedule, bool shallow) const;
	price_t *recall(const MD5Digest &key) const;
	void remember(const MD5Digest &key, const price_t &price) const;
};

#endif
