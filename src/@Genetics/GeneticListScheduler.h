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
typedef	eoPop<chromosome_t> population_t;
typedef std::map<MD5Digest, double, MD5DigestComparator> cache_t;

struct GeneticListSchedulerEvalFuncPtr;
class eslabGenerationalMonitor;

class GeneticListScheduler
{
	friend struct GeneticListSchedulerEvalFuncPtr;
	friend class eslabGenerationalMonitor;

	Graph *graph;
	Hotspot *hotspot;
	cache_t cache;

	double evaluate(const chromosome_t &chromosome);
	inline double evaluate_schedule(const schedule_t &schedule);

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

	schedule_t solve(const priority_t &priority = priority_t());

	inline stats_t get_stats() const { return stats; }

	private:

	tunning_t tunning;
	stats_t stats;

	double sampling_interval;
};

/******************************************************************************/

class eslabEvolution: public eoAlgo<chromosome_t>
{
	public:

	eslabEvolution(eoContinue<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate_one, eoSelect<chromosome_t> &_select,
		eoTransform<chromosome_t> &_transform, eoReplacement<chromosome_t> &_replace) :
		continuator(_continuator), evaluate_one(_evaluate_one), select(_select),
		transform(_transform), replace(_replace) {}

	void operator()(population_t &population);

	private:

	inline void evaluate(population_t &population) const;

	eoContinue<chromosome_t> &continuator;
	eoEvalFunc<chromosome_t> &evaluate_one;
	eoSelect<chromosome_t> &select;
	eoTransform<chromosome_t> &transform;
	eoReplacement<chromosome_t> &replace;
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

class eslabElitismMerge: public eoMerge<chromosome_t>
{
	public:

	eslabElitismMerge(double _rate);

	void operator()(const population_t &source, population_t &destination);

	private:

	double rate;
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
	gene_t max;
	gene_t min;
	size_t points;

	public:

	eslabUniformRangeMutation(gene_t _min, gene_t _max, size_t _points);

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

/******************************************************************************/

std::ostream &operator<< (std::ostream &o,
	const GeneticListScheduler::tunning_t &tunning);

std::ostream &operator<< (std::ostream &o,
	const GeneticListScheduler::stats_t &stats);

#endif
