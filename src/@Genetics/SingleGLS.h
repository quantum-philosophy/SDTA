#ifndef __SINGLE_GLS_H__
#define __SINGLE_GLS_H__

#ifdef REAL_RANK
#include <es.h>
#else
#include <eoInt.h>
#endif

#ifdef REAL_RANK
typedef eoReal<double> single_chromosome_t;
#else
typedef eoInt<double> single_chromosome_t;
#endif

class eslabSingleEvaluate;

class SingleGLS: public GeneticListScheduler<single_chromosome_t, double>
{
	friend struct eslabEvaluate;
};

/******************************************************************************/

class eslabEvolution: public eoAlgo<chromosome_t>
{
	public:

	eslabEvolution(eoContinue<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate_one, eoSelect<chromosome_t> &_select,
		eoTransform<chromosome_t> &_transform, eoReplacement<chromosome_t> &_replace) :
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

/******************************************************************************/

class eslabElitismMerge: public eoMerge<chromosome_t>
{
	public:

	eslabElitismMerge(double _rate);

	void operator()(const population_t &source, population_t &destination);

	private:

	double rate;
};

/******************************************************************************/

class eslabSingleEvaluate: public eoEvalFunc<chromosome_t>
{
	eslabSingleEvaluate(GeneticListScheduler *_ls)
		: eoEvalFunc<chromosome_t>(), ls(_ls) {}

	virtual void operator() (chromosome_t &chromosome)
	{
		if (chromosome.invalid())
			chromosome.fitness(ls->evaluate(chromosome));
	}

	private:

	SingleGLS *ls;
};

#endif
