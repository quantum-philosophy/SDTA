#ifndef __GENETIC_LIST_SCHEDULER_H__
#define __GENETIC_LIST_SCHEDULER_H__

#include <stdexcept>
#include <iostream>
#include <sstream>

#include <eo>
#include <es.h>
#include <ga/eoBitOp.h>

#include "TaskGraph.h"
#include "Hotspot.h"
#include "Genetic.h"

struct GeneticListSchedulerEvalFuncPtr;

class GeneticListScheduler
{
	friend struct GeneticListSchedulerEvalFuncPtr;

	Graph *graph;
	Hotspot *hotspot;

	double evaluate(const chromosome_t &chromosome) const;

	public:

	typedef struct _options_t {
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
	} options_t;

	static options_t defaultOptions(void)
	{
		options_t o;

		o.seed = 0;

		/* Continuator */
		o.population_size = 100;
		o.chromosome_length = 10;
		o.min_generations = 10;
		o.max_generations = 1000;
		o.stall_generations = 20;

		/* Select */
		o.tournament_size = 3;

		/* Crossover */
		o.crossover_rate = 0.8;
		o.crossover_points = 2;

		/* Mutate */
		o.mutation_rate = 0.1;
		o.mutation_points = 2;

		/* Evolve */
		o.generation_gap = 0.5;

		return o;
	}

	GeneticListScheduler(Graph *_graph, Hotspot *_hotspot) :
		graph(_graph), hotspot(_hotspot),
		options(GeneticListScheduler::defaultOptions()) {}
	GeneticListScheduler(options_t _options) : options(_options) {}

	void solve(void);

	private:

	options_t options;
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

#include "GeneticListScheduler.hpp"

#endif
