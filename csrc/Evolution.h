#ifndef __EVOLUTION_H__
#define __EVOLUTION_H__

#include <eo>

#ifdef REAL_RANK
#include <es.h>
#else
#include <eoInt.h>
#endif

#include <ga/eoBitOp.h>

#include "common.h"
#include "Schedule.h"
#include "ListScheduler.h"
#include "Evaluation.h"

template<class FT>
class eslabChromosome
{
	public:

	typedef FT fitness_t;

	inline void assess(const Schedule &schedule, const price_t &price)
	{
		m_schedule = schedule;
		fit(price);
	}

	inline const Schedule &schedule() const
	{
		return m_schedule;
	}

	virtual bool bad() const = 0;

	protected:

	virtual void fit(const price_t &price) = 0;

	Schedule m_schedule;
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

	/* Train */
	double training_min_rate;
	double training_scale;
	double training_exponent;
	size_t max_lessons;
	size_t stall_lessons;

	/* Evolve */
	double elitism_rate;

	/* Output */
	bool verbose;
	std::string dump_evolution;

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

/******************************************************************************/
/* Evolution Stats                                                            */
/******************************************************************************/

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
	double training_rate;

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

/******************************************************************************/
/* Evolution                                                                  */
/******************************************************************************/

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

	GenericEvolution(size_t _chromosome_length,
		const Evaluation &_evaluation,
		const EvolutionTuning &_tuning,
		const constrains_t &_constrains) :

		chromosome_length(_chromosome_length),
		evaluation(_evaluation), tuning(_tuning),
		constrains(_constrains)
	{
		if (chromosome_length == 0)
			throw std::runtime_error("The length cannot be zero.");
	}

	stats_t &solve(const layout_t &layout, const priority_t &priority);

	protected:

	void populate(population_t &population, const layout_t &layout,
		const priority_t &priority);

	virtual void evaluate(chromosome_t &chromosome) = 0;
	virtual void process(population_t &population,
		eslabCheckPoint<chromosome_t> &checkpoint) = 0;

	const size_t chromosome_length;
	const Evaluation evaluation;
	const EvolutionTuning tuning;
	const constrains_t constrains;

	layout_t layout;
	stats_t stats;
};

/******************************************************************************/
/* Crossover                                                                  */
/******************************************************************************/

template<class CT, class PT>
class eslabCrossover: public eoQuadOp<CT>
{
	public:

	eslabCrossover(const rate_t &_rate,
		GenericEvolutionStats<CT, PT> &_stats) :
		rate(_rate), stats(_stats) {}

	inline bool operator()(CT &one, CT &another)
	{
		double current_rate = stats.crossover_rate = rate.get();
		return perform(one, another, current_rate);
	}

	protected:

	virtual bool perform(CT &one, CT &another, double rate) = 0;

	const rate_t &rate;
	GenericEvolutionStats<CT, PT> &stats;
};

template<class CT, class PT = eslabPop<CT> >
class eslabNPtsBitCrossover: public eslabCrossover<CT, PT>
{
	size_t points;

	public:

	eslabNPtsBitCrossover(size_t _points, const rate_t &_rate,
		GenericEvolutionStats<CT, PT> &_stats) :

		eslabCrossover<CT, PT>(_rate, _stats), points(_points)
	{
		if (points < 1)
			std::runtime_error("The number of crossover points is invalid.");
	}

	protected:

	bool perform(CT &one, CT &another, double rate);
};

template<class CT, class PT = eslabPop<CT> >
class eslabPeerCrossover: public eslabCrossover<CT, PT>
{
	const constrains_t &constrains;

	public:

	eslabPeerCrossover(const constrains_t &_constrains,
		const rate_t &_rate, GenericEvolutionStats<CT, PT> &_stats) :

		eslabCrossover<CT, PT>(_rate, _stats), constrains(_constrains) {}

	protected:

	bool perform(CT &one, CT &another, double rate);
};

/******************************************************************************/
/* Mutation                                                                   */
/******************************************************************************/

template<class CT, class PT = eslabPop<CT> >
class eslabMutation: public eoMonOp<CT>
{
	public:

	eslabMutation(const rate_t &_rate,
		GenericEvolutionStats<CT, PT> &_stats) :
		rate(_rate), stats(_stats) {}

	inline bool operator()(CT &chromosome)
	{
		double current_rate = stats.mutation_rate = rate.get();
		return perform(chromosome, current_rate);
	}

	protected:

	virtual bool perform(CT &chromosome, double rate) = 0;

	const rate_t &rate;
	GenericEvolutionStats<CT, PT> &stats;
};

template<class CT, class PT = eslabPop<CT> >
class eslabUniformRangeMutation: public eslabMutation<CT, PT>
{
	const constrains_t &constrains;

	public:

	eslabUniformRangeMutation(const constrains_t &_constrains,
		const rate_t &_rate, GenericEvolutionStats<CT, PT> &_stats) :

		eslabMutation<CT, PT>(_rate, _stats), constrains(_constrains) {}

	protected:

	bool perform(CT &chromosome, double rate);
};

template<class CT, class PT = eslabPop<CT> >
class eslabPeerMutation: public eslabMutation<CT, PT>
{
	public:

	eslabPeerMutation(const constrains_t &_constrains,
		const rate_t &_rate, GenericEvolutionStats<CT, PT> &_stats) :

		eslabMutation<CT, PT>(_rate, _stats), constrains(_constrains) {}

	protected:

	bool perform(CT &chromosome, double rate);

	const constrains_t &constrains;
};

/******************************************************************************/
/* Training                                                                   */
/******************************************************************************/

template<class CT, class PT = eslabPop<CT> >
class eslabTraining: public eoMonOp<CT>
{
	public:

	eslabTraining(const rate_t &_rate,
		GenericEvolutionStats<CT, PT> &_stats) :
		rate(_rate), stats(_stats) {}

	inline bool operator()(CT &chromosome)
	{
		double current_rate = stats.training_rate = rate.get();
		return perform(chromosome, current_rate);
	}

	protected:

	virtual inline bool perform(CT &chromosome, double rate)
	{
		return false;
	}

	const rate_t &rate;
	GenericEvolutionStats<CT, PT> &stats;
};

template<class CT, class PT = eslabPop<CT> >
class eslabPeerTraining: public eslabTraining<CT, PT>
{
	public:

	eslabPeerTraining(const constrains_t &_constrains,
		eoEvalFunc<CT> &_evaluate, size_t _max_lessons, size_t _max_stall,
		const rate_t &_rate, GenericEvolutionStats<CT, PT> &_stats) :

		eslabTraining<CT, PT>(_rate, _stats),
		constrains(_constrains), evaluate(_evaluate),
		max_lessons(_max_lessons), max_stall(_max_stall) {}

	protected:

	bool perform(CT &chromosome, double rate);

	const constrains_t &constrains;
	eoEvalFunc<CT> &evaluate;

	size_t max_lessons;
	size_t max_stall;
};

/******************************************************************************/
/* Transformation                                                             */
/******************************************************************************/

template<class CT>
class eslabTransform: public eoTransform<CT>
{
	typedef eoPop<CT> population_t;

	public:

	eslabTransform(eoQuadOp<CT> &_crossover, eoMonOp<CT> &_mutate,
		eoMonOp<CT> &_train) :

		crossover(_crossover), mutate(_mutate), train(_train) {}

	void operator()(population_t &population);

	private:

	eoQuadOp<CT> &crossover;
	eoMonOp<CT> &mutate;
	eoMonOp<CT> &train;
};

/******************************************************************************/
/* Monitoring                                                                 */
/******************************************************************************/

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

/******************************************************************************/
/* Continuation                                                               */
/******************************************************************************/

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

	eslabStallContinue(size_t _stall_generations) :
		stall_generations(_stall_generations)
	{
		reset();
	}

	bool operator()(const eoPop<CT> &_population)
	{
		generations++;

		const PT *population = dynamic_cast<const population_t *>(&_population);

		if (!population)
			throw std::runtime_error("The population has a wrong type.");

		if (improved(*population)) last_improvement = generations;
		else if (generations - last_improvement > stall_generations)
			return false;

		return true;
	}

	virtual void reset()
	{
		generations = 0;
		last_improvement = 0;
	}

	protected:

	virtual bool improved(const population_t &population) = 0;

	size_t stall_generations;
	size_t generations;
	size_t last_improvement;
};

std::ostream &operator<< (std::ostream &o, const EvolutionTuning &tuning);
std::ostream &operator<< (std::ostream &o, const EvolutionStats &stats);

#include "Evolution.hpp"

#endif
