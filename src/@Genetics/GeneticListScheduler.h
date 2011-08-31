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

		double mobility_ratio;

		size_t population_size;
		size_t min_generations;
		size_t max_generations;
		size_t stall_generations;

		double elitism_rate;
		size_t tournament_size;

		double crossover_rate;
		size_t crossover_points;

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

class eslabFastforwardEA: public eoAlgo<chromosome_t>
{
	public:

	eslabFastforwardEA(eoContinue<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate,
		eoSelect<chromosome_t> &_select_elite,
		eoSelect<chromosome_t> &_select_parents,
		eoQuadOp<chromosome_t> &_crossover,
		eoMonOp<chromosome_t> &_mutate);

	virtual void operator()(eoPop<chromosome_t> &population);

	private:

	void transform(eoPop<chromosome_t> &population) const;

	eoContinue<chromosome_t> &continuator;
	eoEvalFunc<chromosome_t> &evaluate;
	eoSelect<chromosome_t> &select_elite;
	eoSelect<chromosome_t> &select_parents;
	eoQuadOp<chromosome_t> &crossover;
	eoMonOp<chromosome_t> &mutate;
};

/******************************************************************************/

class eslabElitismSelect: public eoSelect<chromosome_t>
{
	public:

	eslabElitismSelect(size_t _count);

	virtual void operator()(const eoPop<chromosome_t> &source,
		eoPop<chromosome_t> &destination);

	private:

	size_t count;
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
	double rate;

	public:

	eslabUniformRangeMutation(gene_t _min, gene_t _max,
		size_t _points, double _rate);

	bool operator()(chromosome_t& chromosome);
};

/******************************************************************************/

class eslabGenerationalMonitor: public eoMonitor
{
	eoPop<chromosome_t> &population;
	GeneticListScheduler *scheduler;

	GeneticListScheduler::tunning_t &tunning;
	GeneticListScheduler::stats_t &stats;

	size_t last_evaluations;

	public:

	eslabGenerationalMonitor(GeneticListScheduler *_scheduler,
		eoPop<chromosome_t> &_population);

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
