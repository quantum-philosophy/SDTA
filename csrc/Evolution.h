#ifndef __EVOLUTION_H__
#define __EVOLUTION_H__

#include "common.h"

#include "Genetics.h"

#include "EvolutionTuning.h"
#include "EvolutionStats.h"

#include "Continuation.h"

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
/* Monitoring                                                                 */
/******************************************************************************/

template<class CT>
class eslabCheckPoint: public eoContinue<CT>
{
	public:

	eslabCheckPoint(eoContinue<CT> &continuator)
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

	inline void add(eoContinue<CT> &continuator)
	{
		continuators.push_back(&continuator);
	}

	inline void add(eoMonitor &monitor)
	{
		monitors.push_back(&monitor);
	}

	private:

	std::vector<eoContinue<CT> *> continuators;
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
