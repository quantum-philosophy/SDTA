#ifndef __MO_EVOLUTION_H__
#define __MO_EVOLUTION_H__

#include <moeo>
#include "Evolution.h"

#define LIFETIME_OBJECTIVE  0
#define ENERGY_OBJECTIVE 1

class eslabObjectiveVectorTraits: public moeoObjectiveVectorTraits
{
	public:

	static bool minimizing(int i) { return i == ENERGY_OBJECTIVE; }
	static bool maximizing(int i) { return i == LIFETIME_OBJECTIVE; }
	static unsigned int nObjectives () { return 2; }
};

class eslabObjectiveVector: public moeoRealObjectiveVector<eslabObjectiveVectorTraits>
{
	public:

    eslabObjectiveVector(double _value = 0.0) :
		moeoRealObjectiveVector<eslabObjectiveVectorTraits>(_value) {}

	operator price_t() const
	{
		return price_t((*this)[LIFETIME_OBJECTIVE], (*this)[ENERGY_OBJECTIVE]);
	}
};

class eslabMOChromosome: public eslabChromosome<eslabObjectiveVector>,
#ifdef REAL_RANK
	public moeoRealVector<eslabObjectiveVector, double, double>
#else
	public moeoIntVector<eslabObjectiveVector, double, double>
#endif
{
	public:

	typedef eslabObjectiveVector fitness_t;

	eslabMOChromosome(size_t _size = 0) :
#ifdef REAL_RANK
		moeoRealVector<eslabObjectiveVector, double, double>(_size) {}
#else
		moeoIntVector<eslabObjectiveVector, double, double>(_size) {}
#endif

	inline bool bad() const
	{
		return objectiveVector()[LIFETIME_OBJECTIVE] <= 0;
	}

	protected:

	inline void assign_fitness(const price_t &price)
	{
		eslabObjectiveVector fitness;
		fitness[LIFETIME_OBJECTIVE] = price.lifetime;
		fitness[ENERGY_OBJECTIVE] = price.energy;

		this->objectiveVector(fitness);
	}
};

class eslabMOPop: public eslabPop<eslabMOChromosome>
{
	public:

	price_t best_lifetime() const;
	price_t best_energy() const;
};

class MOEvolutionStats: public GenericEvolutionStats<eslabMOChromosome, eslabMOPop>
{
	public:

	price_t best_lifetime;
	price_t best_energy;

	std::vector<price_t> pareto_optima;

	void display(std::ostream &o) const;

	protected:

	void process();
};

class MOEvolution:
	public GenericEvolution<eslabMOChromosome, eslabMOPop, MOEvolutionStats>
{
	protected:

	class evaluate_t: public moeoEvalFunc<chromosome_t>
	{
		public:

		evaluate_t(MOEvolution &_evolution) : evolution(_evolution) {}

		void operator()(chromosome_t &chromosome)
		{
			evolution.evaluate(chromosome);
		}

		private:

		MOEvolution &evolution;
	};

	public:

	MOEvolution(const Architecture &_architecture,
		const Graph &_graph, const Evaluation &_evaluation,
		const EvolutionTuning &_tuning, const constrains_t &_constrains) :

		GenericEvolution<chromosome_t, population_t, stats_t>(
			_architecture, _graph, _evaluation, _tuning, _constrains) {}

	protected:

	inline void evaluate(chromosome_t &chromosome)
	{
		if (!chromosome.invalidObjectiveVector()) return;

		if (tuning.include_mapping) evaluation.assess(chromosome);
		else evaluation.assess(chromosome, layout);

		if (chromosome.bad()) stats.miss_deadline();
		else stats.evaluate();
	}

	inline void evaluate(chromosome_t &chromosome, const Schedule &schedule)
	{
		if (!chromosome.invalidObjectiveVector()) return;

		if (tuning.include_mapping) evaluation.assess(chromosome, schedule);
		else evaluation.assess(chromosome, schedule);

		if (chromosome.bad()) stats.miss_deadline();
		else stats.evaluate();
	}

	void process(population_t &population,
		eslabCheckPoint<chromosome_t> &checkpoint);
};

class eslabMOEvolutionMonitor: public eslabEvolutionMonitor<eslabMOChromosome>
{
	public:

	eslabMOEvolutionMonitor(population_t &_population, const std::string &_filename) :
		eslabEvolutionMonitor<eslabMOChromosome>(_population, _filename) {}

	eoMonitor& operator()();
};

#endif
