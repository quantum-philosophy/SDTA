#ifndef __GENETIC_LIST_SCHEDULER_H__
#define __GENETIC_LIST_SCHEDULER_H__

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
#include "Hotspot.h"
#include "MD5Digest.h"

class GLSTuning
{
	public:

	int seed;

	/* Create */
	double uniform_ratio;
	size_t population_size;

	/* Continue */
	size_t min_generations;
	size_t max_generations;
	size_t stall_generations;

	/* Select */
	double elitism_rate;
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

	bool verbose;
	bool cache;
	bool reorder_tasks;

	std::string dump_evolution;

	GLSTuning() { defaults(); }
	GLSTuning(const char *filename);

	protected:

	void defaults();
};

template<class chromosome_t>
class eslabPop: public eoPop<chromosome_t>
{
	public:

	eslabPop(size_t _population_size, size_t _task_count) :
		eoPop<chromosome_t>(), population_size(_population_size),
		task_count(_task_count) {}

	size_t unique() const;
	double diversity() const;

	private:

	size_t population_size, task_count;
};

template<class chromosome_t>
class GLSStats;

template<class chromosome_t>
class GeneticListScheduler
{
	public:

	typedef	eoPop<chromosome_t> population_t;
	typedef typename chromosome_t::Fitness fitness_t;
	typedef std::map<MD5Digest, fitness_t, MD5DigestComparator> cache_t;
	typedef GLSStats<chromosome_t> stats_t;

	GeneticListScheduler(Graph *_graph, Hotspot *_hotspot,
		const GLSTuning &_tuning = GLSTuning());

	schedule_t solve(const priority_t &priority = priority_t());

	inline stats_t get_stats() const { return stats; }

	protected:

	fitness_t evaluate(const chromosome_t &chromosome);

	virtual fitness_t evaluate_schedule(const schedule_t &schedule) = 0;
	virtual void evaluate_chromosome(chromosome_t &chromosome) = 0;
	virtual void process(eoPop<chromosome_t> &population,
		eoContinue<chromosome_t> &continuator,
		eoTransform<chromosome_t> &transform) = 0;

	Graph *graph;
	Hotspot *hotspot;

	GLSTuning tuning;
	GLSStats<chromosome_t> stats;

	cache_t cache;

	double sampling_interval;
};

template<class chromosome_t>
class eslabTransform: public eoTransform<chromosome_t>
{
	public:

	eslabTransform(eoQuadOp<chromosome_t> &_crossover,
		eoMonOp<chromosome_t> &_mutate);

	void operator()(eoPop<chromosome_t> &population);

	private:

	eoQuadOp<chromosome_t> &crossover;
	eoMonOp<chromosome_t> &mutate;
};

template<class chromosome_t>
class eslabNPtsBitCrossover : public eoQuadOp<chromosome_t>
{
	size_t points;

	double min_rate;
	double scale;
	double exponent;

	GLSStats<chromosome_t> &stats;

	public:

	eslabNPtsBitCrossover(size_t _points,
		double _min_rate, double _scale, double _exponent,
		GLSStats<chromosome_t> &_stats);

	bool operator()(chromosome_t &one, chromosome_t &another);
};

template<class chromosome_t>
class eslabUniformRangeMutation: public eoMonOp<chromosome_t>
{
	rank_t min;
	rank_t range;

	double min_rate;
	double scale;
	double exponent;

	GLSStats<chromosome_t> &stats;

	public:

	eslabUniformRangeMutation(rank_t _min, rank_t _max,
		double _min_rate, double _scale, double _exponent,
		GLSStats<chromosome_t> &_stats);

	bool operator()(chromosome_t& chromosome);
};

template<class chromosome_t>
class GLSStats: public eoMonitor
{
	eslabPop<chromosome_t> *population;

	bool silent;
	size_t last_executions;

	public:

	size_t generations;
	size_t evaluations;
	size_t cache_hits;
	size_t deadline_misses;

	double crossover_rate;
	double mutation_rate;

	typename chromosome_t::Fitness best_fitness;
	typename chromosome_t::Fitness worst_fitness;

	GLSStats() : population(NULL) {}

	void watch(eslabPop<chromosome_t> &_population, bool _silent = false)
	{
		population = &_population;
		silent = _silent;

		generations = 0;
		evaluations = 0;
		cache_hits = 0;
		deadline_misses = 0;
		best_fitness = 0;
		worst_fitness = 0;

		last_executions = 0;

		crossover_rate = 0;
		mutation_rate = 0;
	}

	eoMonitor& operator()()
	{
		if (!population)
			throw std::runtime_error("The population is not defined.");

		best_fitness = population->best_element().fitness();
		worst_fitness = population->worse_element().fitness();

		if (!silent) {
			size_t population_size = population->size();
			size_t width = 0;
			size_t executions = cache_hits + deadline_misses + evaluations;

			width = population_size -
				(executions - last_executions) + 1;

			std::cout
				<< std::setw(width) << " "
				<< std::setprecision(2)
				<< best_fitness << " "
				<< std::setw(10)
				<< worst_fitness
				<< std::setprecision(3) << std::setw(8)
				<< " {" << crossover_rate << " " << mutation_rate << "}"
				<< " [" << population->unique() << "/" << population_size << "]"
				<< std::endl
				<< std::setw(4) << generations + 1 << ": ";

			last_executions = executions;
		}

		generations++;

		return *this;
	}

	inline void evaluate()
	{
		if (!silent) std::cout << "." << std::flush;
		evaluations++;
	}

	inline void hit_cache()
	{
		if (!silent) std::cout << "#" << std::flush;
		cache_hits++;
	}

	inline void miss_deadline()
	{
		if (!silent) std::cout << "!" << std::flush;
		deadline_misses++;
	}
};

template<class chromosome_t>
class eslabEvolutionMonitor: public eoMonitor
{
	eoPop<chromosome_t> &population;
	std::ofstream stream;

	public:

	eslabEvolutionMonitor(eoPop<chromosome_t> &_population,
		const std::string &filename);

	virtual eoMonitor& operator()();
};

std::ostream &operator<< (std::ostream &o, const GLSTuning &tuning);

template<class chromosome_t>
std::ostream &operator<< (std::ostream &o, const GLSStats<chromosome_t> &stats);

#include "GeneticListScheduler.hpp"

#endif
