#ifndef __EVOLUTION_STATS_H__
#define __EVOLUTION_STATS_H__

#include "Genetics.h"

class EvolutionStats
{
	public:

	size_t generations;
	size_t evaluations;
	size_t deadline_misses;

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
		evaluations = 0;
		deadline_misses = 0;

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

	GenericEvolutionStats() :
		EvolutionStats(), population(NULL) {}

	void watch(const population_t &_population, bool _silent = false)
	{
		reset();

		population = &_population;
		silent = _silent;
	}

	eoMonitor& operator()();

	inline void evaluate()
	{
		if (!silent) std::cout << "." << std::flush;
		evaluations++;
	}

	inline void miss_deadline()
	{
		if (!silent) std::cout << "!" << std::flush;
		deadline_misses++;
	}

	protected:

	virtual void process()
	{
	}

	const population_t *population;
	bool silent;
};

std::ostream &operator<< (std::ostream &o, const EvolutionStats &stats);

#include "EvolutionStats.hpp"

#endif
