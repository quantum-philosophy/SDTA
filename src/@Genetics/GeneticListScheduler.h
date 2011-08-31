#ifndef __GENETIC_LIST_SCHEDULER_H__
#define __GENETIC_LIST_SCHEDULER_H__

#include <eo>
#include <es.h>
#include <ga/eoBitOp.h>
#include <map>

#include "Common.h"
#include "Hotspot.h"
#include "MD5Digest.h"

typedef double gene_t;
typedef eoReal<gene_t> chromosome_t;
typedef std::map<MD5Digest, double, MD5DigestComparator> cache_t;

struct GeneticListSchedulerEvalFuncPtr;
class eoGenerationalMonitor;

class GeneticListScheduler
{
	friend struct GeneticListSchedulerEvalFuncPtr;
	friend class eoGenerationalMonitor;

	Graph *graph;
	Hotspot *hotspot;
	cache_t cache;

	double evaluate(const chromosome_t &chromosome);

	public:

	struct stats_t {
		size_t generations;
		size_t evaluations;
		size_t cache_hits;
		size_t deadline_misses;
		double best_fitness;

		stats_t() :
			generations(0),
			evaluations(0),
			cache_hits(0),
			deadline_misses(0),
			best_fitness(0) {}
	};

	struct tunning_t {
		int seed;

		double mobility_ratio;

		size_t population_size;
		size_t min_generations;
		size_t max_generations;
		size_t stall_generations;

		size_t tournament_size;

		double crossover_rate;
		size_t crossover_points;

		double mutation_rate;
		size_t mutation_points;

		double generation_gap;

		bool verbose;

		tunning_t();
		tunning_t(const char *filename);

		private:

		void defaults();
	};

	GeneticListScheduler(Graph *_graph, Hotspot *_hotspot,
		const tunning_t &_tunning = tunning_t());

	schedule_t solve();

	inline stats_t get_stats() const { return stats; }

	private:

	tunning_t tunning;
	stats_t stats;

	double sampling_interval;
};

struct GeneticListSchedulerEvalFuncPtr: public eoEvalFunc<chromosome_t>
{
	GeneticListSchedulerEvalFuncPtr(GeneticListScheduler *_ls)
		: eoEvalFunc<chromosome_t>(), ls(_ls) {}

	virtual void operator() (chromosome_t &chromosome)
	{
		if (chromosome.invalid())
			chromosome.fitness(ls->evaluate(chromosome));
	}

	private:

	GeneticListScheduler *ls;
};

class eoUniformRangeMutation: public eoMonOp<chromosome_t>
{
	gene_t max;
	gene_t min;
	unsigned int points;

	public:

	eoUniformRangeMutation(gene_t _min, gene_t _max, unsigned _points)
		: max(_max), min(_min), points(_points) {}

	eoUniformRangeMutation(gene_t _min, gene_t _max)
		: max(_max), min(_min), points(1) {}

	virtual std::string className() const { return "eoUniformRangeMutation"; }

	bool operator()(chromosome_t& chromosome);
};

class eoGenerationalMonitor: public eoMonitor
{
	eoPop<chromosome_t> &population;
	GeneticListScheduler *scheduler;

	public:

	eoGenerationalMonitor(GeneticListScheduler *_scheduler,
		eoPop<chromosome_t> &_population) :
		scheduler(_scheduler), population(_population) {}

	virtual std::string className() const { return "eoGenerationalMonitor"; }

	virtual eoMonitor& operator()(void);
};

std::ostream &operator<< (std::ostream &o,
	const GeneticListScheduler::tunning_t &tunning);

std::ostream &operator<< (std::ostream &o,
	const GeneticListScheduler::stats_t &stats);

#endif
