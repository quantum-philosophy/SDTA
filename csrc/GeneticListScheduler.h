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
	double crossover_rate;
	size_t crossover_points;

	/* Mutate */
	double mutation_rate;

	bool verbose;
	bool cache;

	std::string dump_evolution;

	GLSTuning() { defaults(); }
	GLSTuning(const char *filename);

	protected:

	void defaults();
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

	GeneticListScheduler(Graph *_graph, Hotspot *_hotspot,
		const GLSTuning &_tuning = GLSTuning());

	schedule_t solve(const priority_t &priority = priority_t());

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
	public:

	eslabNPtsBitCrossover(size_t _points, double _rate);

	bool operator()(chromosome_t &one, chromosome_t &another);

	private:

	size_t points;
	double rate;
};

template<class chromosome_t>
class eslabUniformRangeMutation: public eoMonOp<chromosome_t>
{
	rank_t min;
	rank_t range;
	double rate;

	public:

	eslabUniformRangeMutation(rank_t _min, rank_t _max, double _rate);

	bool operator()(chromosome_t& chromosome);
};

template<class chromosome_t>
class GLSStats: public eoMonitor
{
	eoPop<chromosome_t> *population;

	bool silent;
	size_t last_executions;

	public:

	size_t generations;
	size_t evaluations;
	size_t cache_hits;
	size_t deadline_misses;

	typename chromosome_t::Fitness best_fitness;
	typename chromosome_t::Fitness worst_fitness;

	GLSStats() : population(NULL) {}

	void watch(eoPop<chromosome_t> &_population, bool _silent = false)
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
				<< best_fitness << " " << worst_fitness
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

#include "GeneticListScheduler.hpp"

#endif
