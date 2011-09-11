#ifndef __SINGLE_OBJECTIVE_GLS_H__
#define __SINGLE_OBJECTIVE_GLS_H__

#include "common.h"
#include "GeneticListScheduler.h"

#ifdef REAL_RANK
typedef eoReal<double> eslabSOChromosome;
#else
typedef eoInt<double> eslabSOChromosome;
#endif

class eslabSOPop: public eslabPop<eslabSOChromosome>
{
	public:

	inline double best_lifetime() const
	{
		return this->nth_element_fitness(0);
	}
};

class SOGLSStats: public GenericGLSStats<eslabSOChromosome, eslabSOPop>
{
	size_t last_executions;

	public:

	double best_lifetime;
	double worst_lifetime;

	chromosome_t best_priority;
	schedule_t best_schedule;

	void display(std::ostream &o) const;

	protected:

	void reset();
	void process();
};

class SingleObjectiveGLS:
	public GenericGLS<eslabSOChromosome, eslabSOPop, SOGLSStats>
{
	protected:

	class evaluate_t: public eoEvalFunc<chromosome_t>
	{
		public:

		evaluate_t(SingleObjectiveGLS *_ls) :
			eoEvalFunc<chromosome_t>(), ls(_ls) {}

		virtual void operator()(chromosome_t &chromosome)
		{
			if (chromosome.invalid())
				chromosome.fitness(ls->evaluate(chromosome));
		}

		private:

		SingleObjectiveGLS *ls;
	};

	evaluate_t evaluator;

	public:

	SingleObjectiveGLS(Graph *_graph, Hotspot *_hotspot,
		const GLSTuning &_tuning = GLSTuning()) :
		GenericGLS<chromosome_t, population_t, stats_t>(_graph, _hotspot, _tuning),
		evaluator(this) { stats = SOGLSStats(); }

	protected:

	fitness_t evaluate_schedule(const schedule_t &schedule);
	void evaluate_chromosome(chromosome_t &chromosome);

	void process(population_t &population,
		eoCheckPoint<chromosome_t> &checkpoint,
		eoTransform<chromosome_t> &transform);
};

template<class chromosome_t>
class eslabEvolution: public eoAlgo<chromosome_t>
{
	typedef eoPop<chromosome_t> population_t;

	public:

	eslabEvolution(eoContinue<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate_one, eoSelect<chromosome_t> &_select,
		eoTransform<chromosome_t> &_transform,
		eoReplacement<chromosome_t> &_replace) :

		continuator(_continuator), evaluate_one(_evaluate_one), select(_select),
		transform(_transform), replace(_replace) {}

	void operator()(population_t &population);

	private:

	inline void evaluate(population_t &population) const;

	eoContinue<chromosome_t> &continuator;
	eoEvalFunc<chromosome_t> &evaluate_one;
	eoSelect<chromosome_t> &select;
	eoTransform<chromosome_t> &transform;
	eoReplacement<chromosome_t> &replace;
};

template<class chromosome_t>
class eslabElitismMerge: public eoMerge<chromosome_t>
{
	typedef eoPop<chromosome_t> population_t;

	public:

	eslabElitismMerge(double _rate);

	void operator()(const population_t &source, population_t &destination);

	private:

	double rate;
};

class eslabSOStallContinue:
	public eslabStallContinue<eslabSOChromosome, eslabSOPop>
{
	fitness_t last_fitness;

	public:

	eslabSOStallContinue(size_t _min_generations, size_t _stall_generations) :
		eslabStallContinue<eslabSOChromosome, eslabSOPop>(
			_min_generations, _stall_generations) {}

	void reset();

	protected:

	bool improved(const eslabSOPop &population);
};

#include "SingleObjectiveGLS.hpp"

#endif
