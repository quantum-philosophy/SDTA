#ifndef __GENETIC_LIST_SCHEDULER_H__
#define __GENETIC_LIST_SCHEDULER_H__

#include <eo>
#include <ga/eoBitOp.h>
#include <map>

#include "Common.h"
#include "Hotspot.h"
#include "MD5Digest.h"

#ifdef REAL_RANK
typedef eoReal<double> chromosome_t;
#else
typedef eoInt<double> chromosome_t;
#endif

typedef	eoPop<chromosome_t> population_t;
typedef std::map<MD5Digest, double, MD5DigestComparator> cache_t;

class eslabGenerationalMonitor;

template<class CHROMOSOME_T, class FITNESS_T>
class GeneticListScheduler
{
	friend class eslabGenerationalMonitor;

	Graph *graph;
	Hotspot *hotspot;
	cache_t cache;

	FITNESS_T evaluate(const CHROMOSOME_T &chromosome);
	FITNESS_T evaluate_schedule(const schedule_t &schedule) = 0;

	public:

	struct stats_t {
		size_t generations;
		size_t evaluations;
		size_t cache_hits;
		size_t deadline_misses;

		priority_t priority;
		schedule_t schedule;
		double fitness;

		stats_t() :
			generations(0),
			evaluations(0),
			cache_hits(0),
			deadline_misses(0),
			fitness(0) {}
	};

	struct tunning_t {
		int seed;

		/* Create */
		double uniform_ratio;
		size_t population_size;

		/* Continue */
		size_t min_generations;
		size_t max_generations;
		size_t stall_generations;

		/* Select */
		double elitism_rate;
		size_t tournament_size;

		/* Crossover */
		double crossover_rate;
		size_t crossover_points;

		/* Mutate */
		double mutation_rate;
		size_t mutation_points;

		bool verbose;
		bool cache;

		tunning_t();
		tunning_t(const char *filename);

		private:

		void defaults();
	};

	GeneticListScheduler(Graph *_graph, Hotspot *_hotspot,
		const tunning_t &_tunning = tunning_t());

	schedule_t &solve(const priority_t &priority = priority_t());

	inline stats_t get_stats() const { return stats; }

	private:

	tunning_t tunning;
	stats_t stats;

	double sampling_interval;
};

/******************************************************************************/

class eslabTransform: public eoTransform<chromosome_t>
{
	public:

	eslabTransform(
		eoQuadOp<chromosome_t> &_crossover, double _crossover_rate,
		eoMonOp<chromosome_t> &_mutate, double _mutation_rate);

	void operator()(population_t &population);

	private:

	eoQuadOp<chromosome_t> &crossover;
	double crossover_rate;

	eoMonOp<chromosome_t> &mutate;
	double mutation_rate;
};

/******************************************************************************/

class eslabNPtsBitCrossover : public eoQuadOp<chromosome_t>
{
	public:

	eslabNPtsBitCrossover(size_t _points);

	bool operator()(chromosome_t &one, chromosome_t &another);

	private:

	size_t points;
};

/******************************************************************************/

class eslabUniformRangeMutation: public eoMonOp<chromosome_t>
{
	rank_t max;
	rank_t min;
	size_t points;

	public:

	eslabUniformRangeMutation(rank_t _min, rank_t _max, size_t _points);

	bool operator()(chromosome_t& chromosome);
};

/******************************************************************************/

class eslabGenerationalMonitor: public eoMonitor
{
	population_t &population;
	GeneticListScheduler *scheduler;

	GeneticListScheduler::tunning_t &tunning;
	GeneticListScheduler::stats_t &stats;

	size_t last_evaluations;

	public:

	eslabGenerationalMonitor(GeneticListScheduler *_scheduler,
		population_t &_population) :
		scheduler(_scheduler), population(_population), tunning(scheduler->tunning),
		stats(scheduler->stats), last_evaluations(0) {}

	inline void start();
	inline void finish();
	virtual eoMonitor& operator()(void);
};

/******************************************************************************/

std::ostream &operator<< (std::ostream &o,
	const GeneticListScheduler::tunning_t &tunning);

std::ostream &operator<< (std::ostream &o,
	const GeneticListScheduler::stats_t &stats);

#endif
