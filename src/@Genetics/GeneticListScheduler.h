#ifndef __GENETIC_LIST_SCHEDULER_H__
#define __GENETIC_LIST_SCHEDULER_H__

#include <eo>

#ifdef REAL_RANK
#include <es.h>
#else
#include <eoInt.h>
#endif

#include <ga/eoBitOp.h>

#include <map>

#include "Common.h"
#include "Hotspot.h"
#include "MD5Digest.h"

template<class chromosome_t>
class eslabGenerationalMonitor;

template<class chromosome_t>
class GeneticListScheduler
{
	protected:

	typedef	eoPop<chromosome_t> population_t;
	typedef typename chromosome_t::Fitness fitness_t;
	typedef std::map<MD5Digest, fitness_t, MD5DigestComparator> cache_t;

	friend class eslabGenerationalMonitor<chromosome_t>;

	public:

	struct stats_t {
		size_t generations;
		size_t evaluations;
		size_t cache_hits;
		size_t deadline_misses;

		priority_t priority;
		schedule_t schedule;
		fitness_t fitness;

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

		tunning_t() { defaults(); }
		tunning_t(const char *filename);

		private:

		void defaults();
	};

	GeneticListScheduler(Graph *_graph, Hotspot *_hotspot,
		const tunning_t &_tunning = tunning_t());

	schedule_t &solve(const priority_t &priority = priority_t());

	inline stats_t get_stats() const { return stats; }

	protected:

	fitness_t evaluate(const chromosome_t &chromosome);

	virtual fitness_t evaluate_schedule(const schedule_t &schedule) = 0;
	virtual void evaluate_chromosome(chromosome_t &chromosome) = 0;
	virtual void process(eoPop<chromosome_t> &population,
		eoContinue<chromosome_t> &continuator,
		eoTransform<chromosome_t> &transform) = 0;

	Graph *graph;
	Hotspot *hotspot;

	tunning_t tunning;
	stats_t stats;

	cache_t cache;

	double sampling_interval;
};

template<class chromosome_t>
class eslabTransform: public eoTransform<chromosome_t>
{
	public:

	eslabTransform(
		eoQuadOp<chromosome_t> &_crossover, double _crossover_rate,
		eoMonOp<chromosome_t> &_mutate, double _mutation_rate);

	void operator()(eoPop<chromosome_t> &population);

	private:

	eoQuadOp<chromosome_t> &crossover;
	double crossover_rate;

	eoMonOp<chromosome_t> &mutate;
	double mutation_rate;
};

template<class chromosome_t>
class eslabNPtsBitCrossover : public eoQuadOp<chromosome_t>
{
	public:

	eslabNPtsBitCrossover(size_t _points);

	bool operator()(chromosome_t &one, chromosome_t &another);

	private:

	size_t points;
};

template<class chromosome_t>
class eslabUniformRangeMutation: public eoMonOp<chromosome_t>
{
	rank_t max;
	rank_t min;
	size_t points;

	public:

	eslabUniformRangeMutation(rank_t _min, rank_t _max, size_t _points);

	bool operator()(chromosome_t& chromosome);
};

template<class chromosome_t>
class eslabGenerationalMonitor: public eoMonitor
{
	eoPop<chromosome_t> &population;
	GeneticListScheduler<chromosome_t> *scheduler;

	typename GeneticListScheduler<chromosome_t>::tunning_t &tunning;
	typename GeneticListScheduler<chromosome_t>::stats_t &stats;
	size_t last_evaluations;

	public:

	eslabGenerationalMonitor(GeneticListScheduler<chromosome_t> *_scheduler,
		eoPop<chromosome_t> &_population) :
		scheduler(_scheduler), population(_population),
		tunning(_scheduler->tunning), stats(_scheduler->stats),
		last_evaluations(0) {}

	inline void start();
	inline void finish();

	virtual eoMonitor& operator()();
};

#include "GeneticListScheduler.hpp"

#endif
