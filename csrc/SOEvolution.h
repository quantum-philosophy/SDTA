#ifndef __SO_EVOLUTION_H__
#define __SO_EVOLUTION_H__

#include "common.h"
#include "Evolution.h"

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

class SOEvolutionStats: public GenericEvolutionStats<eslabSOChromosome, eslabSOPop>
{
	size_t last_executions;

	public:

	double best_lifetime;
	double worst_lifetime;

	chromosome_t best_chromosome;

	void display(std::ostream &o) const;

	protected:

	void reset();
	void process();
};

class SOEvolution:
	public GenericEvolution<eslabSOChromosome, eslabSOPop, SOEvolutionStats>
{
	protected:

	class evaluate_t: public eoEvalFunc<chromosome_t>
	{
		public:

		evaluate_t(SOEvolution *_ls) :
			eoEvalFunc<chromosome_t>(), ls(_ls) {}

		virtual void operator()(chromosome_t &chromosome)
		{
			if (chromosome.invalid())
				chromosome.fitness(ls->evaluate(chromosome));
		}

		private:

		SOEvolution *ls;
	};

	evaluate_t evaluator;

	public:

	SOEvolution(const Architecture &_architecture,
		const Graph &_graph, const Hotspot &_hotspot,
		const EvolutionTuning &_tuning = EvolutionTuning()) :

		GenericEvolution<chromosome_t, population_t, stats_t>(_architecture,
			_graph, _hotspot, _tuning), evaluator(this) {}

	protected:

	fitness_t evaluate_schedule(const Schedule &schedule);
	void evaluate_chromosome(chromosome_t &chromosome);
	void process(population_t &population,
		eslabCheckPoint<chromosome_t> &checkpoint,
		eoTransform<chromosome_t> &transform);
};

template<class CT>
class eslabAlgorithm: public eoAlgo<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	eslabAlgorithm(
		eoContinue<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate_one) :

		continuator(_continuator), evaluate_one(_evaluate_one) {}

	protected:

	inline void evaluate(population_t &population) const
	{
		apply<chromosome_t>(evaluate_one, population);
	}

	eoContinue<chromosome_t> &continuator;
	eoEvalFunc<chromosome_t> &evaluate_one;
};

template<class CT>
class eslabSOGeneticAlgorithm: public eslabAlgorithm<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	eslabSOGeneticAlgorithm(
		eoContinue<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate_one,
		eoSelect<chromosome_t> &_select,
		eoTransform<chromosome_t> &_transform,
		eoReplacement<chromosome_t> &_replace) :

		eslabAlgorithm<chromosome_t>(_continuator, _evaluate_one),
		select(_select), transform(_transform), replace(_replace) {}

	void operator()(population_t &population);

	private:

	eoSelect<chromosome_t> &select;
	eoTransform<chromosome_t> &transform;
	eoReplacement<chromosome_t> &replace;
};

template<class CT>
class eslabSOLocalSearchAlgorithm: public eslabAlgorithm<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	eslabSOLocalSearchAlgorithm(Graph &graph,
		eoContinue<chromosome_t> &_continuator,
		eoEvalFunc<chromosome_t> &_evaluate_one) :

		eslabAlgorithm<CT>(_continuator, _evaluate_one),
		constrains(graph.get_constrains()) {}

	void operator()(population_t &population);

	private:

	const constrains_t &constrains;
	const chromosome_t &select(population_t &population) const;
};

template<class CT>
class eslabElitismMerge: public eoMerge<CT>
{
	typedef eoPop<CT> population_t;

	public:

	eslabElitismMerge(double _rate);

	void operator()(const population_t &source, population_t &destination);

	private:

	double rate;
};

class eslabSOEvolutionMonitor: public eslabEvolutionMonitor<eslabSOChromosome>
{
	public:

	eslabSOEvolutionMonitor(population_t &_population, const std::string &_filename) :
		eslabEvolutionMonitor<eslabSOChromosome>(_population, _filename) {}

	eoMonitor& operator()();
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

#include "SOEvolution.hpp"

#endif
