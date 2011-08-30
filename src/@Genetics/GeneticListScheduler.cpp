#include <stdexcept>
#include <limits>

#include "GeneticListScheduler.h"
#include "Graph.h"
#include "Task.h"
#include "ListScheduler.h"
#include "DynamicPower.h"
#include "Lifetime.h"

GeneticListScheduler::GeneticListScheduler(Graph *_graph, Hotspot *_hotspot,
	const tunning_t &_tunning) :
	graph(_graph), hotspot(_hotspot), tunning(_tunning)
{
	rng.reseed(tunning.seed);
	sampling_interval = hotspot->sampling_interval();
}

schedule_t GeneticListScheduler::solve()
{
	size_t task_count = graph->task_count;

	if (task_count == 0) throw std::runtime_error("The graph is empty.");

	/* Reset */
	evaluation_count = 0;
	cache_hit_count = 0;
	deadline_miss_count = 0;
	cache.clear();

	task_vector_t &tasks = graph->tasks;

	/* Continuator */
	eoGenContinue<chromosome_t> gen_cont(tunning.max_generations);
	eoSteadyFitContinue<chromosome_t> steady_cont(tunning.min_generations,
		tunning.stall_generations);
	eoCombinedContinue<chromosome_t> continuator(gen_cont, steady_cont);

	/* Evaluate */
	GeneticListSchedulerEvalFuncPtr evaluate(this);

	/* Create */
	eoPop<chromosome_t> population;

	chromosome_t chromosome(task_count);

	/* ... first, collect mobility */
	gene_t current_mobility;
	gene_t min_mobility;
	gene_t max_mobility;
	chromosome[0] = min_mobility = max_mobility = tasks[0]->mobility;

	for (size_t i = 1; i < task_count; i++) {
		current_mobility = tasks[i]->mobility;
		if (min_mobility > current_mobility) min_mobility = current_mobility;
		if (max_mobility < current_mobility) max_mobility = current_mobility;
		chromosome[i] = current_mobility;
	}

	/* ... evaluate it */
	evaluate(chromosome);

	/* ... fill the first part with pure mobility chromosomes */
	size_t create_count = tunning.mobility_ratio * tunning.population_size;
	for (size_t i = 0; i < create_count; i++)
		population.push_back(chromosome);

	/* ... others are randomly generated */
	create_count = tunning.population_size - create_count;
	for (size_t i = 0; i < create_count; i++) {
		for (size_t j = 0; j < task_count; j++)
			chromosome[j] = eo::random(min_mobility, max_mobility);
		chromosome.invalidate();
		evaluate(chromosome);
		population.push_back(chromosome);
	}

	/* Select */
	eoDetTournamentSelect<chromosome_t> selectOne(tunning.tournament_size);
	eoSelectPerc<chromosome_t> select(selectOne);

	/* Crossover */
	eoNPtsBitXover<chromosome_t> crossover(tunning.crossover_points);

	/* Mutate */
	eoUniformRangeMutation mutation(min_mobility, max_mobility,
		tunning.mutation_points);

	/* Evolve */
	eoElitism<chromosome_t> merge(tunning.generation_gap);
	eoTruncate<chromosome_t> reduce;
	eoMergeReduce<chromosome_t> replace(merge, reduce);

	/* Transform */
	eoSGATransform<chromosome_t> transform(crossover, tunning.crossover_rate,
		mutation, tunning.mutation_rate);

	/* Monitor */
	/*
	eoCheckPoint<chromosome_t> checkpoint(continuator);
	eoMatlabMonitor monitor(population);
	checkpoint.add(monitor);

	eoEasyEA<chromosome_t> gga(checkpoint, evaluate, select, transform, replace);
	*/

	eoEasyEA<chromosome_t> gga(continuator, evaluate, select, transform, replace);

	gga(population);

	chromosome = population.best_element();

	std::cout << "Evaluations: " << evaluation_count << std::endl;
	std::cout << "Cache hits: " << cache_hit_count << std::endl;
	std::cout << "Deadline misses: " << deadline_miss_count << std::endl;

	std::cout << "Best lifetime: " << chromosome.fitness() << std::endl;

	return ListScheduler::process(graph, chromosome);
}

double GeneticListScheduler::evaluate(const chromosome_t &chromosome)
{
	evaluation_count++;

	/* Make a new schedule */
	schedule_t schedule = ListScheduler::process(graph, chromosome);

	MD5Digest key(schedule);
	cache_t::const_iterator it = cache.find(key);

	if (it != cache.end()) {
		cache_hit_count++;
		return it->second;
	}

	graph->assign_schedule(schedule);

	double fitness;

	if (graph->duration > graph->deadline) {
		deadline_miss_count++;
		fitness = std::numeric_limits<double>::min();
	}
	else {
		matrix_t dynamic_power, temperature, total_power;

		/* The graph is rescheduled now, obtain the dynamic power profile */
		DynamicPower::compute(graph, sampling_interval, dynamic_power);

		/* Now, we can get the temperature profile, and the total power profile
		 * including the leakage part.
		 */
		unsigned int iterations = hotspot->solve(graph->architecture,
			dynamic_power, temperature, total_power);

		fitness = Lifetime::predict(temperature, sampling_interval);
	}

	cache[key] = fitness;

	return fitness;
}
