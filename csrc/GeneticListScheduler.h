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

template<class CT>
class eslabPop: public eoPop<CT>
{
	public:

	eslabPop(size_t _population_size, size_t _task_count) :
		eoPop<CT>(), population_size(_population_size),
		task_count(_task_count) {}

	size_t unique() const;
	double diversity() const;

	private:

	size_t population_size, task_count;
};

class GLSTuning
{
	public:

	bool multiobjective;

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

	void display(std::ostream &o) const;

	protected:

	void defaults();
};

class GeneticListSchedulerStats
{
	public:

	GeneticListSchedulerStats() {}

	virtual void display(std::ostream &o) const {}
};

template<class CT>
class GenericGLSStats: public GeneticListSchedulerStats, public eoMonitor
{
	protected:

	eslabPop<CT> *population;

	bool silent;

	public:

	size_t generations;
	size_t evaluations;
	size_t cache_hits;
	size_t deadline_misses;

	double crossover_rate;
	double mutation_rate;

	GenericGLSStats() : population(NULL) {}

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

	void watch(eslabPop<CT> &_population, bool _silent = false);
	eoMonitor& operator()();

	virtual void display(std::ostream &o) const;

	protected:

	virtual void reset() {}
	virtual void process() {}
};

class GeneticListScheduler
{
	public:

	virtual GeneticListSchedulerStats &solve(
		const priority_t &priority = priority_t()) = 0;
};

template<class CT, class ST>
class GenericGLS: public GeneticListScheduler
{
	public:

	typedef CT chromosome_t;
	typedef ST stats_t;
	typedef	eoPop<chromosome_t> population_t;
	typedef typename chromosome_t::Fitness fitness_t;
	typedef std::map<MD5Digest, fitness_t, MD5DigestComparator> cache_t;

	GenericGLS(Graph *_graph, Hotspot *_hotspot,
		const GLSTuning &_tuning = GLSTuning());

	stats_t &solve(const priority_t &priority = priority_t());

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
	stats_t stats;

	cache_t cache;

	double sampling_interval;
};

template<class CT>
class eslabTransform: public eoTransform<CT>
{
	public:

	eslabTransform(eoQuadOp<CT> &_crossover,
		eoMonOp<CT> &_mutate);

	void operator()(eoPop<CT> &population);

	private:

	eoQuadOp<CT> &crossover;
	eoMonOp<CT> &mutate;
};

template<class CT>
class eslabNPtsBitCrossover : public eoQuadOp<CT>
{
	size_t points;

	double min_rate;
	double scale;
	double exponent;

	GenericGLSStats<CT> &stats;

	public:

	eslabNPtsBitCrossover(size_t _points,
		double _min_rate, double _scale, double _exponent,
		GenericGLSStats<CT> &_stats);

	bool operator()(CT &one, CT &another);
};

template<class CT>
class eslabUniformRangeMutation: public eoMonOp<CT>
{
	rank_t min;
	rank_t range;

	double min_rate;
	double scale;
	double exponent;

	GenericGLSStats<CT> &stats;

	public:

	eslabUniformRangeMutation(rank_t _min, rank_t _max,
		double _min_rate, double _scale, double _exponent,
		GenericGLSStats<CT> &_stats);

	bool operator()(CT& chromosome);
};

template<class CT>
class eslabEvolutionMonitor: public eoMonitor
{
	eoPop<CT> &population;
	std::ofstream stream;

	public:

	eslabEvolutionMonitor(eoPop<CT> &_population,
		const std::string &filename);

	virtual eoMonitor& operator()();
};

std::ostream &operator<< (std::ostream &o, const GLSTuning &tuning);
std::ostream &operator<< (std::ostream &o, const GeneticListSchedulerStats &stats);

#include "GeneticListScheduler.hpp"

#endif
