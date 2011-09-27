#ifndef __EVOLUTION_STATS_H__
#define __EVOLUTION_STATS_H__

#include "common.h"
#include "Genetics.h"
#include "Evaluation.h"

class EvolutionStats
{
	public:

	size_t generations;

	double crossover_rate;
	double mutation_rate;
	double training_rate;

	EvolutionStats()
	{
		reset();
	}

	virtual inline void reset()
	{
		generations = 0;

		crossover_rate = 0;
		mutation_rate = 0;
		training_rate = 0;
	}

	virtual void display(std::ostream &o) const;
};

template<class CT, class PT = eslabPop<CT> >
class GenericEvolutionStats: public EvolutionStats, public eoMonitor
{
	public:

	typedef CT chromosome_t;
	typedef PT population_t;

	GenericEvolutionStats(const Evaluation &_evaluation) :
		EvolutionStats(), evaluation(_evaluation), population(NULL) {}

	void watch(const population_t &_population, bool _silent = false)
	{
		reset();

		population = &_population;
		silent = _silent;
	}

	eoMonitor& operator()();

	protected:

	virtual void process()
	{
	}

	virtual void display(std::ostream &o) const;

	const Evaluation &evaluation;
	const population_t *population;
	bool silent;
};

std::ostream &operator<< (std::ostream &o, const EvolutionStats &stats);

#include "EvolutionStats.hpp"

#endif
