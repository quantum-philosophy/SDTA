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

		tunning_t() :
			/* Randomness */
			seed(0),

			/* Create */
			mobility_ratio(0.5),

			/* Continuator */
			population_size(25),
			min_generations(0),
			max_generations(100),
			stall_generations(20),

			/* Select */
			tournament_size(3),

			/* Crossover */
			crossover_rate(0.8),
			crossover_points(2),

			/* Mutate */
			mutation_rate(0.05),
			mutation_points(2),

			/* Evolve */
			generation_gap(0.5) {}
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

	bool operator()(chromosome_t& chromosome) {

		unsigned int length = chromosome.size();
		unsigned int point;
		bool hasChanged = false;
		gene_t last;

		for (unsigned int i = 0; i < points; i++) {
			point = rng.random(length);
			last = chromosome[point];
			chromosome[point] = eo::random(min, max);
			if (last != chromosome[point]) hasChanged = true;
		}

		return hasChanged;
	}
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

	virtual eoMonitor& operator()(void)
	{
		scheduler->stats.generations++;
		return *this;
	}
};

std::ostream &operator<< (std::ostream &o,
	const GeneticListScheduler::tunning_t &tunning);

std::ostream &operator<< (std::ostream &o,
	const GeneticListScheduler::stats_t &stats);

#endif
