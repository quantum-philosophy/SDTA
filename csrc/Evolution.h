#ifndef __EVOLUTION_H__
#define __EVOLUTION_H__

#include "common.h"

#include "Genetics.h"

#include "Tuning.h"
#include "EvolutionStats.h"

#include "Continuation.h"

#include "Crossover.h"
#include "Mutation.h"
#include "Transformation.h"

#include "ListScheduler.h"
#include "Evaluation.h"

template<class CT>
class eslabCheckPoint;

/******************************************************************************/
/* Evolution                                                                  */
/******************************************************************************/

class BasicEvolution
{
	public:

	virtual BasicEvolutionStats &solve(const layout_t &layout,
		const priority_t &priority) = 0;
};

template<class CT, class PT, class ST>
class Evolution: public BasicEvolution
{
	protected:

	const Architecture &architecture;
	const Graph &graph;

	const BasicListScheduler &scheduler;
	Evaluation &evaluation;

	const EvolutionTuning tuning;
	const constrains_t constrains;

	const size_t chromosome_length;

	public:

	typedef CT chromosome_t;
	typedef	PT population_t;
	typedef ST stats_t;
	typedef typename chromosome_t::fitness_t fitness_t;

	Evolution(const Architecture &_architecture,
		const Graph &_graph, const BasicListScheduler &_scheduler,
		Evaluation &_evaluation, const EvolutionTuning &_tuning,
		const constrains_t &_constrains) :

		architecture(_architecture), graph(_graph),
		scheduler(_scheduler), evaluation(_evaluation),
		tuning(_tuning), constrains(_constrains),
		chromosome_length((constrains.fixed_layout() ? 1 : 2) * graph.size()),
		stats(_evaluation)
	{
		if (chromosome_length == 0)
			throw std::runtime_error("The length cannot be zero.");
	}

	stats_t &solve(const layout_t &layout, const priority_t &priority);

	inline price_t assess(const chromosome_t &chromosome,
		Evaluation &evaluation) const
	{
		return evaluation.process(schedule(chromosome));
	}

	inline price_t assess(const Schedule &schedule) const
	{
		return evaluation.process(schedule);
	}

	inline Schedule schedule(const chromosome_t &chromosome) const
	{
		if (constrains.fixed_layout()) {
			return scheduler.process(constrains.layout, chromosome);
		}
		else {
			layout_t layout;
			priority_t priority;
			GeneEncoder::split(chromosome, priority, layout);
			return scheduler.process(layout, priority);
		}
	}

	protected:

	void populate(population_t &population, const layout_t &layout,
		const priority_t &priority);

	inline void evaluate(chromosome_t &chromosome)
	{
		if (!chromosome.invalid()) return;

		price_t price;

		if (constrains.fixed_layout()) {
			Schedule schedule = scheduler.process(constrains.layout, chromosome);
			price = evaluation.process(schedule);
		}
		else {
			layout_t layout;
			priority_t priority;

			GeneEncoder::split(chromosome, priority, layout);

			Schedule schedule = scheduler.process(layout, priority);
			price = evaluation.process(schedule);
		}

		chromosome.set_price(price);
	}

	virtual void process(population_t &population) = 0;

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
