#ifndef __MULTI_OBJECTIVE_GLS_H__
#define __MULTI_OBJECTIVE_GLS_H__

#include <moeo>
#include "GeneticListScheduler.h"

#define AGING_OBJECTIVE  0
#define ENERGY_OBJECTIVE 1

class eslabObjectiveVectorTraits: public moeoObjectiveVectorTraits
{
	public:

	static bool minimizing(int i) { return i == ENERGY_OBJECTIVE; }
	static bool maximizing(int i) { return i == AGING_OBJECTIVE; }
	static unsigned int nObjectives () { return 2; }
};

#ifdef REAL_RANK
#else

class eslabObjectiveVector: public moeoRealObjectiveVector<eslabObjectiveVectorTraits>
{
	public:

    eslabObjectiveVector(double _value = 0.0) :
		moeoRealObjectiveVector<eslabObjectiveVectorTraits>(_value) {}

	operator price_t() const
	{
		return price_t((*this)[AGING_OBJECTIVE], (*this)[ENERGY_OBJECTIVE]);
	}
};

class eslabMOChromosome: public moeoIntVector<eslabObjectiveVector,
	/* Fitness */ double, /* Diversity */ double>
{
	public:

	typedef eslabObjectiveVector Fitness;

	eslabMOChromosome(size_t _size = 0) :
		moeoIntVector<eslabObjectiveVector, double, double>(_size) {}
};

class MultiObjectiveGLSStats: public GenericGLSStats<eslabMOChromosome>
{
	typedef eslabMOChromosome chromosome_t;

	size_t last_executions;

	public:

	std::vector<price_t> pareto_optima;

	void display(std::ostream &o) const;

	protected:

	void reset();
	void process();
};

#endif

class MultiObjectiveGLS: public GenericGLS<eslabMOChromosome, MultiObjectiveGLSStats>
{
	protected:

	class evaluate_t: public moeoEvalFunc<chromosome_t>
	{
		public:

		evaluate_t(MultiObjectiveGLS *_ls) : ls(_ls) {}

		void operator()(chromosome_t &chromosome)
		{
			if (chromosome.invalidObjectiveVector())
				chromosome.objectiveVector(ls->evaluate(chromosome));
		}

		private:

		MultiObjectiveGLS *ls;
	};

	evaluate_t evaluator;

	public:

	MultiObjectiveGLS(Graph *_graph, Hotspot *_hotspot,
		const GLSTuning &_tuning = GLSTuning()) :
		GenericGLS<chromosome_t, stats_t>(_graph, _hotspot, _tuning),
		evaluator(this) {}

	protected:

	fitness_t evaluate_schedule(const schedule_t &schedule);
	void evaluate_chromosome(chromosome_t &chromosome);
	void process(eoPop<chromosome_t> &population,
		eoContinue<chromosome_t> &continuator,
		eoTransform<chromosome_t> &transform);
};

#endif
