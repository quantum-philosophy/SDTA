#ifndef __SO_EVOLUTION_H__
#define __SO_EVOLUTION_H__

#include "common.h"
#include "Evolution.h"
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

	inline bool bad() const
	{
		return fitness() <= 0;
	}

	protected:

	inline void assign_fitness(const price_t &price)
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

class SOEvolutionStats: public GenericEvolutionStats<eslabSOChromosome, eslabSOPop>
{
	public:

	double best_lifetime;
	double worst_lifetime;

	size_t last_evaluations;
	size_t last_deadline_misses;

	chromosome_t best_chromosome;

	void display(std::ostream &o) const;

	SOEvolutionStats() : last_evaluations(0), last_deadline_misses(0) {}

	protected:

	void process();
};

class SOEvolution:
	public GenericEvolution<eslabSOChromosome, eslabSOPop, SOEvolutionStats>
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
		const Graph &_graph, const Evaluation &_evaluation,
		const EvolutionTuning &_tuning, const constrains_t &_constrains) :

		GenericEvolution<chromosome_t, population_t, stats_t>(
			_architecture, _graph, _evaluation, _tuning, _constrains) {}

	protected:

	inline void evaluate(chromosome_t &chromosome)
	{
		if (!chromosome.invalid()) return;

		if (tuning.include_mapping) evaluation.assess(chromosome, true);
		else evaluation.assess(chromosome, layout, true);

		if (chromosome.bad()) stats.miss_deadline();
		else stats.evaluate();
	}

	inline void evaluate(chromosome_t &chromosome, const Schedule &schedule)
	{
		if (!chromosome.invalid()) return;

		if (tuning.include_mapping) evaluation.assess(chromosome, schedule, true);
		else evaluation.assess(chromosome, schedule, true);

		if (chromosome.bad()) stats.miss_deadline();
		else stats.evaluate();
	}

	void process(population_t &population,
		eslabCheckPoint<chromosome_t> &checkpoint);
};

template<class CT>
class eslabAlgorithm: public eoAlgo<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	eslabAlgorithm(
		eoContinue<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate_one) :

		continuator(_continuator), evaluate_one(_evaluate_one) {}

	protected:

	inline void evaluate(population_t &population) const
	{
		apply<chromosome_t>(evaluate_one, population);
	}

	eoContinue<chromosome_t> &continuator;
	eoEvalFunc<chromosome_t> &evaluate_one;
};

template<class CT>
class eslabSOGeneticAlgorithm: public eslabAlgorithm<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	eslabSOGeneticAlgorithm(
		eoContinue<chromosome_t> &_continuator,
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

class eslabSOEvolutionMonitor: public eslabEvolutionMonitor<eslabSOChromosome>
{
	public:

	eslabSOEvolutionMonitor(population_t &_population, const std::string &_filename) :
		eslabEvolutionMonitor<eslabSOChromosome>(_population, _filename) {}

	eoMonitor& operator()();
};

#include "SOEvolution.hpp"

#endif
