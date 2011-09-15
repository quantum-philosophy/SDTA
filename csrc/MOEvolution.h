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

class eslabMOChromosome:
#ifdef REAL_RANK
	public moeoRealVector<eslabObjectiveVector,
#else
	public moeoIntVector<eslabObjectiveVector,
#endif
	/* Fitness */ double, /* Diversity */ double>
{
	public:

	typedef eslabObjectiveVector Fitness;

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

		evaluate_t(MOEvolution *_ls) : ls(_ls) {}

		void operator()(chromosome_t &chromosome)
		{
			if (chromosome.invalidObjectiveVector())
				chromosome.objectiveVector(ls->evaluate(chromosome));
		}

		private:

		MOEvolution *ls;
	};

	evaluate_t evaluator;

	public:

	MOEvolution(Architecture *_architecture, Graph *_graph,
		Hotspot *_hotspot, const EvolutionTuning &_tuning = EvolutionTuning()) :

		GenericEvolution<chromosome_t, population_t, stats_t>(_architecture,
			_graph, _hotspot, _tuning), evaluator(this) {}

	protected:

	fitness_t evaluate(const chromosome_t &chromosome);
	void evaluate_chromosome(chromosome_t &chromosome);
	void process(population_t &population,
		eoCheckPoint<chromosome_t> &checkpoint,
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
