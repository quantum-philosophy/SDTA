#ifndef __EVOLUTION_H__
#define __EVOLUTION_H__

#include "common.h"

#include "Genetics.h"

#include "EvolutionTuning.h"
#include "EvolutionStats.h"

#include "Crossover.h"
#include "Mutation.h"
#include "Training.h"
#include "Transformation.h"

#include "Evaluation.h"

template<class CT>
class eslabCheckPoint;

/******************************************************************************/
/* Evolution                                                                  */
/******************************************************************************/

class Evolution
{
	public:

	virtual EvolutionStats &solve(const layout_t &layout,
		const priority_t &priority) = 0;
};

template<class CT, class PT, class ST>
class GenericEvolution: public Evolution
{
	public:

	typedef CT chromosome_t;
	typedef	PT population_t;
	typedef ST stats_t;
	typedef typename chromosome_t::fitness_t fitness_t;

	GenericEvolution(size_t _chromosome_length,
		const Evaluation &_evaluation,
		const EvolutionTuning &_tuning,
		const constrains_t &_constrains) :

		chromosome_length(_chromosome_length),
		evaluation(_evaluation), tuning(_tuning),
		constrains(_constrains)
	{
		if (chromosome_length == 0)
			throw std::runtime_error("The length cannot be zero.");
	}

	stats_t &solve(const layout_t &layout, const priority_t &priority);

	protected:

	void populate(population_t &population, const layout_t &layout,
		const priority_t &priority);

	eoQuadOp<CT> *choose_crossover();
	eoMonOp<CT> *choose_mutation();
	eoMonOp<CT> *choose_training(eoEvalFunc<CT> &evaluator);

	virtual void evaluate(chromosome_t &chromosome) = 0;
	virtual void process(population_t &population,
		eslabCheckPoint<chromosome_t> &checkpoint) = 0;

	const size_t chromosome_length;
	const Evaluation evaluation;
	const EvolutionTuning tuning;
	const constrains_t constrains;

	layout_t layout;
	stats_t stats;
};

/******************************************************************************/
/* Continuation                                                               */
/******************************************************************************/

template<class CT>
class eslabContinue: public eoContinue<CT>
{
	public:

	virtual void reset() = 0;
};

template<class CT>
class eslabGenContinue: public eslabContinue<CT>
{
	public:

	typedef CT chromosome_t;

	eslabGenContinue(size_t _max_generations) :
		max_generations(_max_generations) { reset(); }

	virtual bool operator()(const eoPop<chromosome_t> &population)
	{
		generations++;

		if (generations >= max_generations) return false;
		return true;
	}

	virtual void reset()
	{
		generations = 0;
	}

	private:

	size_t max_generations;
	size_t generations;
};

template<class CT, class PT>
class eslabStallContinue: public eslabContinue<CT>
{
	public:

	typedef CT chromosome_t;
	typedef PT population_t;
	typedef typename CT::fitness_t fitness_t;

	eslabStallContinue(size_t _stall_generations) :
		stall_generations(_stall_generations)
	{
		reset();
	}

	bool operator()(const eoPop<CT> &_population)
	{
		generations++;

		const PT *population = dynamic_cast<const population_t *>(&_population);

		if (!population)
			throw std::runtime_error("The population has a wrong type.");

		if (improved(*population)) last_improvement = generations;
		else if (generations - last_improvement > stall_generations)
			return false;

		return true;
	}

	virtual void reset()
	{
		generations = 0;
		last_improvement = 0;
	}

	protected:

	virtual bool improved(const population_t &population) = 0;

	size_t stall_generations;
	size_t generations;
	size_t last_improvement;
};

/******************************************************************************/
/* Monitoring                                                                 */
/******************************************************************************/

template<class CT>
class eslabCheckPoint: public eoContinue<CT>
{
	public:

	eslabCheckPoint(eslabContinue<CT> &continuator)
	{
		continuators.push_back(&continuator);
	}

	bool operator()(const eoPop<CT> &population)
	{
		size_t monitor_count, continuator_count, i;

		monitor_count = monitors.size();
		continuator_count = continuators.size();

		for (i = 0; i < monitor_count; i++) (*monitors[i])();

		bool go_on = true;

		for (i = 0; i < continuator_count; i++)
			if (!(*continuators[i])(population)) go_on = false;

		/* Say goodbye */
		if (!go_on) {
			for (i = 0; i < monitor_count; i++)
				monitors[i]->lastCall();
		}

		return go_on;
	}

	inline void add(eslabContinue<CT> &continuator)
	{
		continuators.push_back(&continuator);
	}

	inline void add(eoMonitor &monitor)
	{
		monitors.push_back(&monitor);
	}

	inline void reset()
	{
		size_t count = continuators.size();
		for (size_t i = 0; i < count; i++)
			continuators[i]->reset();
	}

	private:

	std::vector<eslabContinue<CT> *> continuators;
	std::vector<eoMonitor *> monitors;
};

template<class CT>
class eslabEvolutionMonitor: public eoMonitor
{
	public:

	typedef eoPop<CT> population_t;
	typedef typename CT::fitness_t fitness_t;

	eslabEvolutionMonitor(population_t &_population, const std::string &filename);

	virtual eoMonitor& operator()() = 0;

	protected:

	population_t &population;
	std::ofstream stream;
};

#include "Evolution.hpp"

#endif
