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
#include "Schedule.h"
#include "ListScheduler.h"

template<class FT>
class eslabChromosome {};

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

	typedef typename CT::fitness_t fitness_t;

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

template<class CT>
class eslabContinue: public eoContinue<CT>
{
	public:

	virtual void reset() = 0;
};

template<class CT>
class eslabCheckPoint: public eoContinue<CT>
{
	public:

	eslabCheckPoint(eslabContinue<CT> &continuator)
	{
		continuators.push_back(&continuator);
	}

	bool operator()(const eoPop<CT> &population)
	{
		size_t monitor_count, continuator_count, i;

		monitor_count = monitors.size();
		continuator_count = continuators.size();

		for (i = 0; i < monitor_count; i++) (*monitors[i])();

		bool go_on = true;

		for (i = 0; i < continuator_count; i++)
			if (!(*continuators[i])(population)) go_on = false;

		/* Say goodbye */
		if (!go_on) {
			for (i = 0; i < monitor_count; i++)
				monitors[i]->lastCall();
		}

		return go_on;
	}

	inline void add(eslabContinue<CT> &continuator)
	{
		continuators.push_back(&continuator);
	}

	inline void add(eoMonitor &monitor)
	{
		monitors.push_back(&monitor);
	}

	inline void reset()
	{
		size_t count = continuators.size();
		for (size_t i = 0; i < count; i++)
			continuators[i]->reset();
	}

	private:

	std::vector<eslabContinue<CT> *> continuators;
	std::vector<eoMonitor *> monitors;
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

	virtual EvolutionStats &solve(const layout_t &layout,
		const priority_t &priority) = 0;
};

template<class CT, class PT, class ST>
class GenericEvolution: public Evolution
{
	public:

	typedef CT chromosome_t;
	typedef	PT population_t;
	typedef ST stats_t;
	typedef typename chromosome_t::fitness_t fitness_t;

	GenericEvolution(const Architecture &_architecture,
		const Graph &_graph, const Hotspot &_hotspot,
		const EvolutionTuning &_tuning = EvolutionTuning());

	void update(std::istream &stream);

	stats_t &solve(const layout_t &layout, const priority_t &priority);

	inline Schedule calc_schedule(const chromosome_t &chromosome) const
	{
		if (tuning.include_mapping) {
			eslabDualGeneEncoder<chromosome_t> dual(chromosome);

			return ListScheduler::process(architecture, graph,
				dual.layout(), dual.priority());
		}
		else {
			return ListScheduler::process(architecture, graph,
				layout /* use the default one */, chromosome);
		}
	}

	protected:

	void populate(population_t &population, const layout_t &layout,
		const priority_t &priority);

	virtual fitness_t evaluate(const chromosome_t &chromosome) = 0;
	virtual void assess(chromosome_t &chromosome) = 0;
	virtual void process(population_t &population,
		eslabCheckPoint<chromosome_t> &checkpoint,
		eoTransform<chromosome_t> &transform) = 0;

	const Architecture &architecture;
	const Graph &graph;
	const Hotspot &hotspot;
	layout_t layout;

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
class eslabCrossover: public eoQuadOp<CT>
{
	double min_rate;
	double scale;
	double exponent;

	GenericEvolutionStats<CT, PT> &stats;

	public:

	eslabCrossover(double _min_rate, double _scale, double _exponent,
		GenericEvolutionStats<CT, PT> &_stats) :

		min_rate(_min_rate), scale(_scale), exponent(_exponent), stats(_stats)
	{
		if (min_rate < 0 || min_rate > 1)
			std::runtime_error("The mutation minimal rate is invalid.");
	}

	bool operator()(CT &one, CT &another)
	{
		double rate = stats.crossover_rate = std::max(min_rate,
			scale * std::exp(exponent * (double)stats.generations));

		return perform(one, another, rate);
	}

	protected:

	virtual bool perform(CT &one, CT &another, double rate) = 0;
};

template<class CT, class PT = eslabPop<CT> >
class eslabNPtsBitCrossover: public eslabCrossover<CT, PT>
{
	size_t points;

	public:

	eslabNPtsBitCrossover(size_t _points, double _min_rate, double _scale,
		double _exponent, GenericEvolutionStats<CT, PT> &_stats) :

		eslabCrossover<CT, PT>(_min_rate, _scale, _exponent, _stats),
		points(_points)
	{
		if (points < 1)
			std::runtime_error("The number of crossover points is invalid.");
	}

	protected:

	virtual bool perform(CT &one, CT &another, double rate);
};

template<class CT, class PT = eslabPop<CT> >
class eslabPeerCrossover: public eslabCrossover<CT, PT>
{
	const constrains_t &constrains;

	public:

	eslabPeerCrossover(const constrains_t &_constrains, double _min_rate,
		double _scale, double _exponent, GenericEvolutionStats<CT, PT> &_stats) :

		eslabCrossover<CT, PT>(_min_rate, _scale, _exponent, _stats),
		constrains(_constrains) {}

	protected:

	bool perform(CT &one, CT &another, double rate);
};

template<class CT, class PT = eslabPop<CT> >
class eslabMutation: public eoMonOp<CT>
{
	double min_rate;
	double scale;
	double exponent;

	GenericEvolutionStats<CT, PT> &stats;

	public:

	eslabMutation(double _min_rate, double _scale, double _exponent,
		GenericEvolutionStats<CT, PT> &_stats) :

		min_rate(_min_rate), scale(_scale), exponent(_exponent), stats(_stats)
	{
		if (min_rate < 0 || min_rate > 1)
			std::runtime_error("The mutation minimal rate is invalid.");
	}

	bool operator()(CT &chromosome)
	{
		double rate = stats.mutation_rate = std::max(min_rate,
			scale * std::exp(exponent * (double)stats.generations));

		return perform(chromosome, rate);
	}

	protected:

	virtual bool perform(CT &chromosome, double rate) = 0;
};

template<class CT, class PT = eslabPop<CT> >
class eslabUniformRangeMutation: public eslabMutation<CT, PT>
{
	const constrains_t &constrains;

	public:

	eslabUniformRangeMutation(const constrains_t &_constrains, double _min_rate,
		double _scale, double _exponent, GenericEvolutionStats<CT, PT> &_stats) :

		eslabMutation<CT, PT>(_min_rate, _scale, _exponent, _stats),
		constrains(_constrains) {}

	protected:

	bool perform(CT &chromosome, double rate);
};

template<class CT, class PT = eslabPop<CT> >
class eslabPeerMutation: public eslabMutation<CT, PT>
{
	const constrains_t &constrains;

	public:

	eslabPeerMutation(const constrains_t &_constrains, double _min_rate,
		double _scale, double _exponent, GenericEvolutionStats<CT, PT> &_stats) :

		eslabMutation<CT, PT>(_min_rate, _scale, _exponent, _stats),
		constrains(_constrains) {}

	protected:

	bool perform(CT &chromosome, double rate);
};

template<class CT, class PT = eslabPop<CT> >
class eslabLearning: public eoMonOp<CT>
{
	public:

	typedef CT chromosome_t;
	typedef PT population_t;

	eslabLearning() {}

	bool operator()(chromosome_t &chromosome);
};

template<class CT>
class eslabEvolutionMonitor: public eoMonitor
{
	public:

	typedef eoPop<CT> population_t;
	typedef typename CT::fitness_t fitness_t;

	eslabEvolutionMonitor(population_t &_population, const std::string &filename);

	virtual eoMonitor& operator()() = 0;

	protected:

	population_t &population;
	std::ofstream stream;
};

template<class CT>
class eslabGenContinue: public eslabContinue<CT>
{
	public:

	typedef CT chromosome_t;

	eslabGenContinue(size_t _max_generations) :
		max_generations(_max_generations) { reset(); }

	virtual bool operator()(const eoPop<chromosome_t> &population)
	{
		generations++;

		if (generations >= max_generations) return false;
		return true;
	}

	virtual void reset()
	{
		generations = 0;
	}

	private:

	size_t max_generations;
	size_t generations;
};

template<class CT, class PT>
class eslabStallContinue: public eslabContinue<CT>
{
	public:

	typedef CT chromosome_t;
	typedef PT population_t;
	typedef typename CT::fitness_t fitness_t;

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
