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
		int seed;

		unsigned int population_size;
		unsigned int chromosome_length;
		unsigned int min_generations;
		unsigned int max_generations;
		unsigned int stall_generations;

		unsigned tournament_size;

		double crossover_rate;
		unsigned int crossover_points;

		double mutation_rate;
		unsigned int mutation_points;

		double generation_gap;

		tunning_t() :
			seed(0),

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
		const tunning_t &_tunning = tunning_t());

	void solve(void);

	private:

	eoPop<chromosome_t> create_population() const;

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
