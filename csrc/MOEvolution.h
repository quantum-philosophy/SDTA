#ifndef __MO_EVOLUTION_H__
#define __MO_EVOLUTION_H__

#include "common.h"

#include <moeo>

#include "Evolution.h"
#include "Continuation.h"

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

	protected:

	inline void set_fitness(const price_t &price)
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

class MOEvolutionStats: public EvolutionStats<eslabMOChromosome, eslabMOPop>
{
	public:

	price_t best_lifetime;
	price_t best_energy;

	std::vector<price_t> pareto_optima;

	void display(std::ostream &o) const;

	MOEvolutionStats(const Evaluation &_evaluation) :
		EvolutionStats<eslabMOChromosome, eslabMOPop>(_evaluation) {}

	virtual eoMonitor& operator()();
};

class MOEvolution:
	public Evolution<eslabMOChromosome, eslabMOPop, MOEvolutionStats>
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
		const Graph &_graph, const BasicListScheduler &_scheduler,
		Evaluation &_evaluation, const EvolutionTuning &_tuning,
		const constrains_t &_constrains) :

		Evolution<chromosome_t, population_t, stats_t>(
			_architecture, _graph, _scheduler, _evaluation, _tuning, _constrains) {}

	MOEvolutionStats &solve(const layout_t &layout, const priority_t &priority);
};

class MOContinuation: public Continuation<eslabMOChromosome>
{
	const size_t stall_generations;
	size_t stall_count;
	double last_lifetime;
	double last_energy;

	public:

	MOContinuation(const ContinuationTuning &_tuning) :
		Continuation<eslabMOChromosome>(_tuning),
		stall_generations(_tuning.stall_generations), stall_count(0),
		last_lifetime(0), last_energy(DBL_MAX) {}

	inline void reset()
	{
		Continuation<eslabMOChromosome>::reset();
		stall_count = 0;
		last_lifetime = 0;
		last_energy = DBL_MAX;
	}

	protected:

	inline bool stop(const eoPop<eslabMOChromosome> &_population)
	{
		const eslabMOPop *population =
			dynamic_cast<const eslabMOPop *>(&_population);

		stall_count++;

		price_t lifetime = population->best_lifetime();

		if (lifetime.lifetime > last_lifetime) {
			last_lifetime = lifetime.lifetime;
			stall_count = 0;
		}

		price_t energy = population->best_energy();

		if (energy.energy < last_energy) {
			last_energy = energy.energy;
			stall_count = 0;
		}

		return (stall_count > stall_generations);
	}
};

class eslabMOEvolutionMonitor: public eslabEvolutionMonitor<eslabMOChromosome>
{
	public:

	eslabMOEvolutionMonitor(population_t &_population, const std::string &_filename) :
		eslabEvolutionMonitor<eslabMOChromosome>(_population, _filename) {}

	eoMonitor& operator()();
};

#endif
