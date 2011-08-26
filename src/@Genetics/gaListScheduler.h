#ifndef __GA_LIST_SCHEDULER_H__
#define __GA_LIST_SCHEDULER_H__

#include <stdexcept>
#include <iostream>
#include <sstream>

#include <eo>
#include <es.h>
#include <ga/eoBitOp.h>

#include "gaCommon.h"

struct gaListSchedulerEvalFuncPtr;

class gaListScheduler
{
	friend struct gaListSchedulerEvalFuncPtr;

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

	gaListScheduler() : options(gaListScheduler::defaultOptions()) {}
	gaListScheduler(options_t _options) : options(_options) {}

	void solve(void);

	protected:

	options_t options;

	double evaluate(const chromosome_t &chromosome) const;
};

struct gaListSchedulerEvalFuncPtr: public eoEvalFunc<chromosome_t>
{
	gaListSchedulerEvalFuncPtr(const gaListScheduler *_ls)
		: eoEvalFunc<chromosome_t>(), ls(_ls) {}

	virtual void operator() (chromosome_t &chromosome)
	{
		if (chromosome.invalid())
			chromosome.fitness(ls->evaluate(chromosome));
	}

	private:

	const gaListScheduler *ls;
};

#include "gaListScheduler.hpp"

#endif
