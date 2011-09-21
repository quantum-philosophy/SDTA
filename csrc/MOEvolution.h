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
};

class eslabMOPop: public eslabPop<eslabMOChromosome>
{
	public:

	price_t best_lifetime() const;
	price_t best_energy() const;
};

class MOEvolutionStats: public GenericEvolutionStats<eslabMOChromosome, eslabMOPop>
{
	size_t last_executions;

	public:

	price_t best_lifetime;
	price_t best_energy;

	std::vector<price_t> pareto_optima;

	void display(std::ostream &o) const;

	protected:

	void reset();
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
			evolution.assess(chromosome);
		}

		private:

		MOEvolution &evolution;
	};

	public:

	MOEvolution(size_t _chromosome_length, const Evaluation &_evaluation,
		const EvolutionTuning &_tuning, const constrains_t &_constrains) :

		GenericEvolution<chromosome_t, population_t, stats_t>(
			_chromosome_length, _evaluation, _tuning, _constrains) {}

	protected:

	inline void assess(chromosome_t &chromosome)
	{
		if (!chromosome.invalidObjectiveVector()) return;

		price_t price;

		if (tuning.include_mapping) {
			layout_t layout;
			priority_t priority;
			GeneEncoder::split(chromosome, layout, priority);
			price = evaluation.process(layout, priority);
		}
		else {
			price = evaluation.process(layout, chromosome);
		}

		if (price.lifetime <= 0) stats.miss_deadline();
		else stats.evaluate();

		fitness_t fitness;
		fitness[LIFETIME_OBJECTIVE] = price.lifetime;
		fitness[ENERGY_OBJECTIVE] = price.energy;

		chromosome.objectiveVector(fitness);
	}

	void process(population_t &population,
		eslabCheckPoint<chromosome_t> &checkpoint,
		eoTransform<chromosome_t> &transform);
};

class eslabMOEvolutionMonitor: public eslabEvolutionMonitor<eslabMOChromosome>
{
	public:

	eslabMOEvolutionMonitor(population_t &_population, const std::string &_filename) :
		eslabEvolutionMonitor<eslabMOChromosome>(_population, _filename) {}

	eoMonitor& operator()();
};

class eslabMOStallContinue:
	public eslabStallContinue<eslabMOChromosome, eslabMOPop>
{
	price_t last_fitness;

	public:

	eslabMOStallContinue(size_t _min_generations, size_t _stall_generations) :
		eslabStallContinue<eslabMOChromosome, eslabMOPop>(
			_min_generations, _stall_generations) {}

	void reset();

	protected:

	bool improved(const eslabMOPop &population);
};

#endif
