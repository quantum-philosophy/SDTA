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

class GeneticListScheduler
{
	friend struct GeneticListSchedulerEvalFuncPtr;

	Graph *graph;
	Hotspot *hotspot;
	cache_t cache;
	size_t evaluation_count;
	size_t cache_hit_count;
	size_t deadline_miss_count;

	double evaluate(const chromosome_t &chromosome);

	public:

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

	private:

	tunning_t tunning;
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

class eoMatlabMonitor: public eoMonitor
{
	eoPop<chromosome_t> &population;

	public:

	eoMatlabMonitor(eoPop<chromosome_t> &_population) : population(_population) {}

	virtual std::string className() const { return "eoMatlabMonotor"; }

	virtual eoMonitor& operator()(void)
	{
		return *this;
	}
};

#endif
