#ifndef __SO_EVOLUTION_H__
#define __SO_EVOLUTION_H__

#include "common.h"
#include "Evolution.h"

class eslabSOChromosome: public eslabChromosome<double>,
#ifdef REAL_RANK
	public eoReal<double>
#else
	public eoInt<double>
#endif
{
	public:

	typedef double fitness_t;

#ifdef REAL_RANK
	eslabSOChromosome() : eoReal<double>() {}
	eslabSOChromosome(size_t _size) : eoReal<double>(_size) {}
#else
	eslabSOChromosome() : eoInt<double>() {}
	eslabSOChromosome(size_t _size) : eoInt<double>(_size) {}
#endif
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

		evaluate_t(SOEvolution &_evolution) :
			eoEvalFunc<chromosome_t>(), evolution(_evolution) {}

		void operator()(chromosome_t &chromosome)
		{
			evolution.assess(chromosome);
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

	fitness_t evaluate(const chromosome_t &chromosome);

	inline void assess(chromosome_t &chromosome)
	{
		if (chromosome.invalid())
			chromosome.fitness(evaluate(chromosome));
	}

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

template <class CT>
class eslabTournamentSelect: public eoSelectOne<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	eslabTournamentSelect(size_t _size) :
		eoSelectOne<chromosome_t>(), size(_size)
	{
		if (size < 2)
			throw std::runtime_error("The tournament size should be at least 2.");
	}

	const chromosome_t &operator()(const population_t &population)
	{
		size_t population_size = population.size();
		size_t best = Random::number(population_size);
		size_t i, next;

		for (i = 0; i < size; i++) {
			next = Random::number(population_size);
			if (population[best].fitness() < population[next].fitness())
				best = next;
		}

		return population[best];
	}

	private:

	const size_t size;
};

template <class CT>
class eslabRouletteSelect: public eoSelectOne<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	eslabRouletteSelect() : eoSelectOne<chromosome_t>() {}

	const chromosome_t &operator()(const population_t &population)
	{
		return roulette_wheel(population);
	}

	void setup(const population_t &population)
	{
		population_size = population.size();
		rank.resize(population_size);
		children.resize(population_size);

		size_t i, j;
		double fitness;

		for (i = 0; i < population_size; i++) {
			fitness = population[i].fitness();

			rank[i] = 0;
			children[i] = 0;

			for (j = 0; j < population_size; j++)
				if (population[j].fitness() < fitness) rank[i]++;
		}
	}

	private:

	const chromosome_t &roulette_wheel(const population_t &population)
	{
		size_t i;
		double total = 0;
		std::vector<double> chance(population_size);

		for (i = 0; i < population_size; i++) {
			chance[i] = double(rank[i]) / double(children[i] + 1);
			total += chance[i];
		}

		double roulette = Random::uniform(total);

		i = 0;
		while ((roulette -= chance[i]) > 0) i++;

		children[i]++;

		return population[i];
	}

	size_t population_size;
	std::vector<size_t> rank;
	std::vector<size_t> children;
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
