#ifndef __MULTI_OBJECTIVE_GLS_H__
#define __MULTI_OBJECTIVE_GLS_H__

#include <moeo>

#define AGING_OBJECTIVE  0
#define ENERGY_OBJECTIVE 1

class eslabObjectiveVectorTraits: public moeoObjectiveVectorTraits
{
	public:

	static bool minimizing(int i) { return i == EGING_OBJECTIVE; }
	static bool maximizing(int i) { return i == AGING_OBJECTIVE; }
	static unsigned int nObjectives () { return 2; }
};

#ifdef REAL_RANK

typedef moeoRealObjectiveVector<eslabObjectiveVectorTraits> elsabObjectiveVector;

class chromosome_t: public moeoRealVector<eslabObjectiveVector, double, double>
{
	public:

	chromosome_t(size_t _size) :
		moeoRealVector<eslabObjectiveVector, double, double>(_size) {}
};

#else
#endif

class MultiGLS;

class eslabMultiEvaluate: public moeoEvalFunc<chromosome_t>
{
	public:

	eslabMultiEvaluate(MultiGLS *_ls) : ls(_ls) {}

	void operator()(chromosome_t &chromosome)
	{
		if (chromosome.invalidObjectiveVector())
			chromosome.objectiveVector(ls->evaluate(chromosome));
	}

	private:

	MultiGLS *ls;
};

class MultiGLS: public GeneticListScheduler
{
	friend struct eslabMultiEvaluate;

	public:

	eslabObjectiveVector evaluate(const chromosome_t &chromosome);
};

#endif
