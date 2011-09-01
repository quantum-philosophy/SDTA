#ifndef __SINGLE_GLS_H__
#define __SINGLE_GLS_H__

#include "Common.h"
#include "GeneticListScheduler.h"

#ifdef REAL_RANK
class SingleGLS: public GeneticListScheduler<eoReal<double> >
{
	typedef eoReal<double> chromosome_t;
#else
class SingleGLS: public GeneticListScheduler<eoInt<double> >
{
	typedef eoInt<double> chromosome_t;
#endif

	public:

	class evaluate_t: public eoEvalFunc<chromosome_t>
	{
		public:

		evaluate_t(SingleGLS *_ls) :
			eoEvalFunc<chromosome_t>(), ls(_ls) {}

		virtual void operator() (chromosome_t &chromosome)
		{
			if (chromosome.invalid())
				chromosome.fitness(ls->evaluate(chromosome));
		}

		private:

		SingleGLS *ls;
	};

	SingleGLS(Graph *_graph, Hotspot *_hotspot,
		const tunning_t &_tunning = tunning_t()) :
		GeneticListScheduler<chromosome_t>(_graph, _hotspot, _tunning) {}

	protected:

	fitness_t evaluate_schedule(const schedule_t &schedule);

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

std::ostream &operator<< (std::ostream &o, const SingleGLS::tunning_t &tunning);
std::ostream &operator<< (std::ostream &o, const SingleGLS::stats_t &stats);

#endif
