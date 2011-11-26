#ifndef __SO_EVOLUTION_H__
#define __SO_EVOLUTION_H__

#include "common.h"

#include "Evolution.h"
#include "Continuation.h"
#include "Selection.h"
#include "Replacement.h"

class eslabSOChromosome: public eslabChromosome<double>,
#ifdef REAL_RANK
	public eoReal<double>
#else
	public eoInt<double>
#endif
{
	public:

	typedef double fitness_t;

	eslabSOChromosome(size_t _size = 0) :
#ifdef REAL_RANK
		eoReal<double>(_size) {}
#else
		eoInt<double>(_size) {}
#endif

	protected:

	inline void set_fitness(const price_t &price)
	{
		this->fitness(price.lifetime);
	}
};

class eslabSOPop: public eslabPop<eslabSOChromosome>
{
	public:

	inline double best_lifetime() const
	{
		return this->nth_element_fitness(0);
	}
};

class SOEvolutionStats: public EvolutionStats<eslabSOChromosome, eslabSOPop>
{
	public:

	price_t best_price;
	chromosome_t best_chromosome;
	Schedule best_schedule;

	void display(std::ostream &o) const;

	SOEvolutionStats(const Evaluation &_evaluation) :
		EvolutionStats<eslabSOChromosome, eslabSOPop>(_evaluation) {}

	virtual eoMonitor& operator()();
};

class SOEvolution:
	public Evolution<eslabSOChromosome, eslabSOPop, SOEvolutionStats>
{
	protected:

	class evaluate_t: public eoEvalFunc<chromosome_t>
	{
		public:

		evaluate_t(SOEvolution &_evolution) :
			eoEvalFunc<chromosome_t>(), evolution(_evolution) {}

		void operator()(chromosome_t &chromosome)
		{
			evolution.evaluate(chromosome);
		}

		private:

		SOEvolution &evolution;
	};

	public:

	SOEvolution(const Architecture &_architecture,
		const Graph &_graph, const BasicListScheduler &_scheduler,
		Evaluation &_evaluation, const EvolutionTuning &_tuning,
		const constrains_t &_constrains) :

		Evolution<chromosome_t, population_t, stats_t>(
			_architecture, _graph, _scheduler, _evaluation, _tuning, _constrains) {}

	SOEvolutionStats &solve(const layout_t &layout, const priority_t &priority);
};

template<class CT>
class eslabAlgorithm: public eoAlgo<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	eslabAlgorithm(
		eslabCheckPoint<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate_one) :

		continuator(_continuator), evaluate_one(_evaluate_one) {}

	protected:

#ifdef PRECISE_TIMEOUT
	inline bool evaluate(population_t &population) const
	{
		size_t size = population.size();
		for (size_t i = 0; i < size; i++) {
			if (continuator.timeout()) return false;
			evaluate_one(population[i]);
		}
		return true;
	}
#else
	inline void evaluate(population_t &population) const
	{
		apply<chromosome_t>(evaluate_one, population);
	}
#endif

	eslabCheckPoint<chromosome_t> &continuator;
	eoEvalFunc<chromosome_t> &evaluate_one;
};

template<class CT>
class eslabSOGeneticAlgorithm: public eslabAlgorithm<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	eslabSOGeneticAlgorithm(
		eslabCheckPoint<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate_one,
		eoSelect<chromosome_t> &_select,
		eoTransform<chromosome_t> &_transform,
		eoReplacement<chromosome_t> &_replace) :

		eslabAlgorithm<chromosome_t>(_continuator, _evaluate_one),
		select(_select), transform(_transform), replace(_replace) {}

	void operator()(population_t &population);

	private:

	eoSelect<chromosome_t> &select;
	eoTransform<chromosome_t> &transform;
	eoReplacement<chromosome_t> &replace;
};

class SOContinuation: public Continuation<eslabSOChromosome>
{
	double last_lifetime;

	public:

	SOContinuation(const ContinuationTuning &_tuning) :
		Continuation<eslabSOChromosome>(_tuning), last_lifetime(0) {}

	protected:

	inline bool improved(const eoPop<eslabSOChromosome> &_population)
	{
		const eslabSOPop *population =
			dynamic_cast<const eslabSOPop *>(&_population);

		double lifetime = population->best_lifetime();

		if (lifetime > last_lifetime) {
			last_lifetime = lifetime;
			return true;
		}

		return false;
	}
};

class eslabSOEvolutionMonitor: public eslabEvolutionMonitor<eslabSOChromosome>
{
	public:

	eslabSOEvolutionMonitor(population_t &_population, const std::string &_filename) :
		eslabEvolutionMonitor<eslabSOChromosome>(_population, _filename) {}

	eoMonitor& operator()();
};

#include "SOEvolution.hpp"

#endif
