#ifndef __GENETIC_LIST_SCHEDULER_H__
#define __GENETIC_LIST_SCHEDULER_H__

#include <eo>
#include <es.h>
#include <ga/eoBitOp.h>

#include "Common.h"
#include "Hotspot.h"

typedef double gene_t;
typedef eoReal<gene_t> chromosome_t;

struct GeneticListSchedulerEvalFuncPtr;

class GeneticListScheduler
{
	friend struct GeneticListSchedulerEvalFuncPtr;

	Graph *graph;
	Hotspot *hotspot;

	double evaluate(const chromosome_t &chromosome) const;

	public:

	struct tunning_t {
		double sampling_interval;

		int seed;

		double mobility_ratio;

		size_t population_size;
		size_t chromosome_length;
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
			sampling_interval(1e-4),

			/* Randomness */
			seed(0),

			/* Create */
			mobility_ratio(0.5),

			/* Continuator */
			population_size(100),
			chromosome_length(10),
			min_generations(10),
			max_generations(1000),
			stall_generations(20),

			/* Select */
			tournament_size(3),

			/* Crossover */
			crossover_rate(0.8),
			crossover_points(2),

			/* Mutate */
			mutation_rate(0.1),
			mutation_points(2),

			/* Evolve */
			generation_gap(0.5) {}
	};

	GeneticListScheduler(Graph *_graph, Hotspot *_hotspot,
		const tunning_t &_tunning = tunning_t()) :
		graph(_graph), hotspot(_hotspot), tunning(_tunning) {}

	schedule_t solve();

	private:

	tunning_t tunning;
};

struct GeneticListSchedulerEvalFuncPtr: public eoEvalFunc<chromosome_t>
{
	GeneticListSchedulerEvalFuncPtr(const GeneticListScheduler *_ls)
		: eoEvalFunc<chromosome_t>(), ls(_ls) {}

	virtual void operator() (chromosome_t &chromosome)
	{
		if (chromosome.invalid())
			chromosome.fitness(ls->evaluate(chromosome));
	}

	private:

	const GeneticListScheduler *ls;
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
