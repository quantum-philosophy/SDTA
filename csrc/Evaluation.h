#ifndef __EVALUATION_H__
#define __EVALUATION_H__

#include "common.h"
#include "Genetics.h"

#ifndef WITHOUT_MEMCACHED
#include <libmemcached/memcached.h>
#include "Digest.h"
#include <sstream>
#endif

class Evaluation
{
	const Architecture &architecture;
	const Graph &graph;
	const Hotspot &hotspot;

	const bool shallow;

	public:

	size_t evaluations;
	size_t deadline_misses;
	size_t cache_hits;

	Evaluation(const Architecture &_architecture, const Graph &_graph,
		const Hotspot &_hotspot, bool _shallow) :

		architecture(_architecture), graph(_graph), hotspot(_hotspot),
		shallow(_shallow), evaluations(0), deadline_misses(0), cache_hits(0) {}

	price_t process(const Schedule &schedule);

	protected:

	virtual price_t compute(const Schedule &schedule);
};

std::ostream &operator<<(std::ostream &o, const Evaluation &e);

#ifndef WITHOUT_MEMCACHED

class MemcachedEvaluation: public Evaluation
{
	const bool extended;
	memcached_st *memcache;

	public:

	MemcachedEvaluation(const std::string &config, bool _extended,
		const Architecture &_architecture, const Graph &_graph,
		const Hotspot &_hotspot, bool _shallow) :

		Evaluation(_architecture, _graph, _hotspot, _shallow), extended(_extended)
	{
		memcache = memcached_create(NULL);

		if (!memcache)
			throw std::runtime_error("Cannot allocate memcached.");

		unsigned int port = 11211;
		std::string hostname = "localhost";

		if (!config.empty()) {
			size_t found = config.find_first_of(":");

			if (found != std::string::npos) {
				hostname = config.substr(0, found);
				std::stringstream stream(config.substr(found + 1));
				stream >> port;
				if (port <= 0)
					throw std::runtime_error("Cannot read the port number.");
			}
			else hostname = config;
		}

		(void)memcached_server_add(memcache, hostname.c_str(), port);
	}

	~MemcachedEvaluation()
	{
		memcached_free(memcache);
	}

	protected:

	price_t compute(const Schedule &schedule);

	price_t *recall(const Digest &key);
	void remember(const Digest &key, const price_t &price);
};

#endif

#endif
