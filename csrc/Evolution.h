#ifndef __EVOLUTION_H__
#define __EVOLUTION_H__

#include <eo>

#ifdef REAL_RANK
#include <es.h>
#else
#include <eoInt.h>
#endif

#include <ga/eoBitOp.h>

#include <map>
#include <iostream>
#include <iomanip>
#include <string>

#include "common.h"

template<class CT>
class eslabGeneEncoder
{
	public:

	inline const priority_t &priority() const { return m_priority; }
	inline const CT &chromosome() const { return m_chromosome; }
	inline operator CT() const { return m_chromosome; }

	protected:

	priority_t m_priority;
	CT m_chromosome;
};

template<class CT>
class eslabMonoGeneEncoder: public eslabGeneEncoder<CT>
{
	public:

	eslabMonoGeneEncoder(const priority_t &_priority)
	{
		encode(_priority);
	}

	eslabMonoGeneEncoder(const CT &_chromosome)
	{
		decode(_chromosome);
	}

	private:

	using eslabGeneEncoder<CT>::m_priority;
	using eslabGeneEncoder<CT>::m_chromosome;

	inline void encode(const priority_t &_priority)
	{
		m_priority = _priority;

		size_t size = _priority.size();

		m_chromosome = CT(size);

		for (size_t i = 0; i < size; i++)
			m_chromosome[i] = _priority[i];
	}

	inline void decode(const CT &_chromosome)
	{
		m_chromosome = _chromosome;

		size_t size = _chromosome.size();

		m_priority = priority_t(size);

		for (size_t i = 0; i < size; i++)
			m_priority[i] = _chromosome[i];
	}
};

template<class CT>
class eslabDualGeneEncoder: public eslabGeneEncoder<CT>
{
	layout_t m_layout;

	public:

	eslabDualGeneEncoder(const priority_t &_priority, const layout_t &_layout)
	{
		encode(_priority, _layout);
	}

	eslabDualGeneEncoder(const CT &_chromosome)
	{
		decode(_chromosome);
	}

	inline const layout_t &layout() const { return m_layout; }

	private:

	using eslabGeneEncoder<CT>::m_priority;
	using eslabGeneEncoder<CT>::m_chromosome;

	inline void encode(const priority_t &_priority, const layout_t &_layout)
	{
		m_priority = _priority;
		m_layout = _layout;

		size_t half = _priority.size();

		if (_layout.size() != half)
			throw std::runtime_error("The layout is wrong.");

		m_chromosome = CT(2 * half);

		for (size_t i = 0; i < half; i++) {
			m_chromosome[i] = _priority[i];
			m_chromosome[half + i] = _layout[i];
		}
	}

	inline void decode(const CT &_chromosome)
	{
		m_chromosome = _chromosome;

		size_t half = _chromosome.size() / 2;

		m_priority = priority_t(half);
		m_layout = layout_t(half);

		for (size_t i = 0; i < half; i++) {
			m_priority[i] = _chromosome[i];
			m_layout[i] = _chromosome[half + i];
		}
	}
};

template<class CT>
class eslabPop: public eoPop<CT>
{
	public:

	typedef typename eoPop<CT>::Fitness fitness_t;

	size_t unique() const;
	double diversity() const;
};

class EvolutionTuning
{
	public:

	/* Prepare */
	int repeat;
	double deadline_ratio;
	bool reorder_tasks;
	bool include_mapping;

	/* Target */
	bool multiobjective;

	/* Randomize */
	int seed;

	/* Create */
	double uniform_ratio;
	size_t population_size;

	/* Continue */
	size_t min_generations;
	size_t max_generations;
	size_t stall_generations;

	/* Select */
	size_t tournament_size;

	/* Crossover */
	double crossover_min_rate;
	double crossover_scale;
	double crossover_exponent;
	size_t crossover_points;

	/* Mutate */
	double mutation_min_rate;
	double mutation_scale;
	double mutation_exponent;

	/* Evolve */
	double elitism_rate;

	/* Output */
	bool verbose;
	std::string dump_evolution;

	EvolutionTuning() { defaults(); }
	EvolutionTuning(const std::string &filename);

	void update(std::istream &stream);

	void display(std::ostream &o) const;

	protected:

	void defaults();
};

class EvolutionStats
{
	public:

	EvolutionStats() {}

	virtual void display(std::ostream &o) const {}
};

template<class CT, class PT = eslabPop<CT> >
class GenericEvolutionStats: public EvolutionStats, public eoMonitor
{
	public:

	typedef CT chromosome_t;
	typedef PT population_t;

	size_t generations;
	size_t evaluations;
	size_t deadline_misses;

	double crossover_rate;
	double mutation_rate;

	GenericEvolutionStats() : population(NULL) {}

	inline void evaluate()
	{
		if (!silent) std::cout << "." << std::flush;
		evaluations++;
	}

	inline void miss_deadline()
	{
		if (!silent) std::cout << "!" << std::flush;
		deadline_misses++;
	}

	void watch(population_t &_population, bool _silent = false);
	eoMonitor& operator()();

	virtual void display(std::ostream &o) const;

	protected:

	virtual void reset() {}
	virtual void process() {}

	population_t *population;
	bool silent;
};

class Evolution
{
	public:

	virtual EvolutionStats &solve(
		const priority_t &priority = priority_t(),
		const layout_t &layout = layout_t()) = 0;
};

template<class CT, class PT, class ST>
class GenericEvolution: public Evolution
{
	public:

	typedef CT chromosome_t;
	typedef	PT population_t;
	typedef ST stats_t;
	typedef typename chromosome_t::Fitness fitness_t;

	GenericEvolution(Architecture *_architecture, Graph *_graph, Hotspot *_hotspot,
		const EvolutionTuning &_tuning = EvolutionTuning());

	void update(std::istream &stream);

	stats_t &solve(const priority_t &priority = priority_t(),
		const layout_t &layout = layout_t());

	protected:

	void populate(population_t &population, priority_t priority, layout_t layout);

	virtual fitness_t evaluate(const chromosome_t &chromosome) = 0;
	virtual void evaluate_chromosome(chromosome_t &chromosome) = 0;
	virtual void process(population_t &population,
		eoCheckPoint<chromosome_t> &checkpoint,
		eoTransform<chromosome_t> &transform) = 0;

	Architecture *architecture;
	Graph *graph;
	Hotspot *hotspot;

	stats_t stats;

	const EvolutionTuning tuning;
	const size_t task_count;
	const size_t chromosome_length;
	const constrains_t constrains;
	const double sampling_interval;
};

template<class CT>
class eslabTransform: public eoTransform<CT>
{
	typedef eoPop<CT> population_t;

	public:

	eslabTransform(eoQuadOp<CT> &_crossover,
		eoMonOp<CT> &_mutate);

	void operator()(population_t &population);

	private:

	eoQuadOp<CT> &crossover;
	eoMonOp<CT> &mutate;
};

template<class CT, class PT = eslabPop<CT> >
class eslabNPtsBitCrossover : public eoQuadOp<CT>
{
	size_t points;

	double min_rate;
	double scale;
	double exponent;

	GenericEvolutionStats<CT, PT> &stats;

	public:

	eslabNPtsBitCrossover(size_t _points, double _min_rate, double _scale,
		double _exponent, GenericEvolutionStats<CT, PT> &_stats);

	bool operator()(CT &one, CT &another);
};

template<class CT, class PT = eslabPop<CT> >
class eslabUniformRangeMutation: public eoMonOp<CT>
{
	double min_rate;
	double scale;
	double exponent;

	GenericEvolutionStats<CT, PT> &stats;

	const constrains_t &constrains;

	public:

	eslabUniformRangeMutation(const constrains_t &constrains, double _min_rate,
		double _scale, double _exponent, GenericEvolutionStats<CT, PT> &_stats);

	bool operator()(CT& chromosome);
};

template<class CT>
class eslabEvolutionMonitor: public eoMonitor
{
	public:

	typedef eoPop<CT> population_t;
	typedef typename eoPop<CT>::Fitness fitness_t;

	eslabEvolutionMonitor(population_t &_population, const std::string &filename);

	virtual eoMonitor& operator()() = 0;

	protected:

	population_t &population;
	std::ofstream stream;
};

template<class CT, class PT>
class eslabStallContinue: public eoContinue<CT>
{
	public:

	typedef CT chromosome_t;
	typedef PT population_t;
	typedef typename CT::Fitness fitness_t;

	eslabStallContinue(size_t _min_generations, size_t _stall_generations) :
		min_generations(_min_generations), stall_generations(_stall_generations)
	{
		reset();
	}

	bool operator()(const eoPop<CT> &_population)
	{
		generations++;

		const PT *population = dynamic_cast<const population_t *>(&_population);

		if (!population)
			throw std::runtime_error("The population has a wrong type.");

		if (steady_state) {
			if (improved(*population)) last_improvement = generations;
			else if (generations - last_improvement > stall_generations)
				return false;
		}
		else if (generations > min_generations) {
			steady_state = true;
			last_improvement = generations;
		}

		return true;
	}

	virtual void reset()
	{
		steady_state = false;
		generations = 0;
		last_improvement = 0;
	}

	protected:

	virtual bool improved(const population_t &population) = 0;

	size_t min_generations;
	size_t stall_generations;

	bool steady_state;
	size_t generations;
	size_t last_improvement;
};

std::ostream &operator<< (std::ostream &o, const EvolutionTuning &tuning);
std::ostream &operator<< (std::ostream &o, const EvolutionStats &stats);

#include "Evolution.hpp"

#endif
