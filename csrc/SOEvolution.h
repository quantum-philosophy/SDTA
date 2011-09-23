#ifndef __SO_EVOLUTION_H__
#define __SO_EVOLUTION_H__

#include "common.h"
#include "Evolution.h"
#include "Selection.h"

class eslabSOChromosome: public eslabChromosome<double>,
#ifdef REAL_RANK
	public eoReal<double>
#else
	public eoInt<double>
#endif
{
	public:

	typedef double fitness_t;

	eslabSOChromosome(size_t _size = 0) :
#ifdef REAL_RANK
		eoReal<double>(_size) {}
#else
		eoInt<double>(_size) {}
#endif

	inline bool bad() const
	{
		return fitness() <= 0;
	}

	protected:

	inline void fit(const price_t &price)
	{
		this->fitness(price.lifetime);
	}
};

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
	public:

	double best_lifetime;
	double worst_lifetime;

	chromosome_t best_chromosome;

	void display(std::ostream &o) const;

	protected:

	void process();
};

class SOEvolution:
	public GenericEvolution<eslabSOChromosome, eslabSOPop, SOEvolutionStats>
{
	protected:

	class evaluate_t: public eoEvalFunc<chromosome_t>
	{
		public:

		evaluate_t(SOEvolution &_evolution) :
			eoEvalFunc<chromosome_t>(), evolution(_evolution) {}

		void operator()(chromosome_t &chromosome)
		{
			evolution.evaluate(chromosome);
		}

		private:

		SOEvolution &evolution;
	};

	public:

	SOEvolution(size_t _chromosome_length, const Evaluation &_evaluation,
		const EvolutionTuning &_tuning, const constrains_t &_constrains) :

		GenericEvolution<chromosome_t, population_t, stats_t>(
			_chromosome_length, _evaluation, _tuning, _constrains) {}

	protected:

	inline void evaluate(chromosome_t &chromosome)
	{
		if (!chromosome.invalid()) return;

		if (tuning.include_mapping) evaluation.assess(chromosome, true);
		else evaluation.assess(chromosome, layout, true);

		if (chromosome.bad()) stats.miss_deadline();
		else stats.evaluate();
	}

	void process(population_t &population,
		eslabCheckPoint<chromosome_t> &checkpoint);
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

template <class CT>
class FulfillingReplacement: public eoReplacement<CT>
{
	public:

	FulfillingReplacement(eoMerge<CT> & _merge, eoReduce<CT> &_reduce,
		Selection<CT> &_select) :
		merge(_merge), reduce(_reduce), select(_select) {}

	void operator()(eoPop<CT> &parents, eoPop<CT> &offspring)
	{
		size_t parent_size = parents.size();

		merge(parents, offspring);

		size_t offspring_size = offspring.size();

		int mistmatch = parent_size - offspring_size;

		if (mistmatch > 0) {
			/* Fulfil! */
			select.append(parents, offspring, mistmatch);
		}
		else if (mistmatch < 0) {
			/* Shrink! */
			reduce(offspring, parent_size);
		}

		parents.swap(offspring);
	}

	private:

	eoMerge<CT> &merge;
	eoReduce<CT> &reduce;
	Selection<CT> &select;
};

template <class CT>
class SimilarityReplacement: public eoReplacement<CT>
{
	public:

	SimilarityReplacement() {}

	void operator()(eoPop<CT> &parents, eoPop<CT> &offspring);
};

template<class CT>
class ElitismMerge: public eoMerge<CT>
{
	typedef eoPop<CT> population_t;

	public:

	ElitismMerge(double _rate) : rate(_rate)
	{
		if (rate < 0)
			std::runtime_error("The elitism rate is invalid.");
	}

	void operator()(const population_t &source, population_t &destination);

	private:

	double rate;
};

template <class CT>
class KillerReduction: public eoReduce<CT>
{
	public:

	void operator()(eoPop<CT> &population, unsigned size)
	{
		int kill_count = population.size() - size;

		if (kill_count < 0)
			throw std::runtime_error("Cannot reduce such a small population.");

		for (size_t i = 0; i < kill_count; i++) {
			typename eoPop<CT>::iterator it = population.it_worse_element();
			population.erase(it);
		}
	}
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

	eslabSOStallContinue(size_t _stall_generations) :
		eslabStallContinue<eslabSOChromosome, eslabSOPop>(_stall_generations) {}

	void reset();

	protected:

	bool improved(const eslabSOPop &population);
};

#include "SOEvolution.hpp"

#endif
