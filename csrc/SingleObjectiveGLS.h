#ifndef __SINGLE_OBJECTIVE_GLS_H__
#define __SINGLE_OBJECTIVE_GLS_H__

#include "common.h"
#include "GeneticListScheduler.h"

#ifdef REAL_RANK
class SingleObjectiveGLS: public GenericGLS<eoReal<double> >
{
	typedef eoReal<double> chromosome_t;
#else
class SingleObjectiveGLS: public GenericGLS<eoInt<double> >
{
	typedef eoInt<double> chromosome_t;
#endif

	protected:

	class evaluate_t: public eoEvalFunc<chromosome_t>
	{
		public:

		evaluate_t(SingleObjectiveGLS *_ls) :
			eoEvalFunc<chromosome_t>(), ls(_ls) {}

		virtual void operator() (chromosome_t &chromosome)
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
		GenericGLS<chromosome_t>(_graph, _hotspot, _tuning),
		evaluator(this) {}

	protected:

	fitness_t evaluate_schedule(const schedule_t &schedule);
	void evaluate_chromosome(chromosome_t &chromosome);

	void process(eoPop<chromosome_t> &population,
		eoContinue<chromosome_t> &continuator,
		eoTransform<chromosome_t> &transform);
};

template<class chromosome_t>
class eslabEvolution: public eoAlgo<chromosome_t>
{
	public:

	eslabEvolution(eoContinue<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate_one, eoSelect<chromosome_t> &_select,
		eoTransform<chromosome_t> &_transform,
		eoReplacement<chromosome_t> &_replace) :

		continuator(_continuator), evaluate_one(_evaluate_one), select(_select),
		transform(_transform), replace(_replace) {}

	void operator()(eoPop<chromosome_t> &population);

	private:

	inline void evaluate(eoPop<chromosome_t> &population) const;

	eoContinue<chromosome_t> &continuator;
	eoEvalFunc<chromosome_t> &evaluate_one;
	eoSelect<chromosome_t> &select;
	eoTransform<chromosome_t> &transform;
	eoReplacement<chromosome_t> &replace;
};

template<class chromosome_t>
class eslabElitismMerge: public eoMerge<chromosome_t>
{
	public:

	eslabElitismMerge(double _rate);

	void operator()(const eoPop<chromosome_t> &source,
		eoPop<chromosome_t> &destination);

	private:

	double rate;
};

#include "SingleObjectiveGLS.hpp"

#endif
