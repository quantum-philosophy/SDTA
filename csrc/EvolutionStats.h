#ifndef __EVOLUTION_STATS_H__
#define __EVOLUTION_STATS_H__

#include "common.h"
#include "Genetics.h"
#include "Evaluation.h"

class BasicEvolutionStats
{
	public:

	size_t generations;

	double crossover_rate;
	double mutation_rate;
	double training_rate;

	BasicEvolutionStats()
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
class EvolutionStats: public BasicEvolutionStats, public eoMonitor
{
	public:

	typedef CT chromosome_t;
	typedef PT population_t;

	EvolutionStats(const Evaluation &_evaluation) :
		BasicEvolutionStats(), evaluation(_evaluation), population(NULL),
		last_evaluations(0), last_deadline_misses(0), last_cache_hits(0) {}

	void watch(const population_t &_population, bool _silent = false)
	{
		reset();

		population = &_population;
		silent = _silent;
	}

	virtual eoMonitor& operator()();

	protected:

	const Evaluation &evaluation;
	const population_t *population;
	bool silent;

	private:

	size_t last_evaluations;
	size_t last_deadline_misses;
	size_t last_cache_hits;
};

std::ostream &operator<< (std::ostream &o, const BasicEvolutionStats &stats);

#include "EvolutionStats.hpp"

#endif
