#include <stdexcept>
#include <limits>

#include "GeneticListScheduler.h"
#include "Graph.h"
#include "Task.h"
#include "ListScheduler.h"
#include "DynamicPower.h"
#include "Lifetime.h"

schedule_t GeneticListScheduler::solve()
{
	size_t task_count = graph->task_count;

	if (task_count == 0) throw std::runtime_error("The graph is empty.");

	task_vector_t &tasks = graph->tasks;

	rng.reseed(tunning.seed);

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

	return ListScheduler::process(graph, chromosome);

	/* ... fill the first part with pure mobility chromosomes */
	size_t create_count = tunning.mobility_ratio * tunning.population_size;
	for (size_t i = 0; i < create_count; i++)
		population.push_back(chromosome);

	/* ... others are randomly generated */
	create_count = tunning.population_size - create_count;
	for (size_t i = 0; i < create_count; i++) {
		for (size_t j = 0; j < task_count; j++)
			chromosome[j] = eo::random(min_mobility, max_mobility);
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
	eoCheckPoint<chromosome_t> checkpoint(continuator);
	eoMatlabMonitor monitor(population);
	checkpoint.add(monitor);

	eoEasyEA<chromosome_t> gga(checkpoint, evaluate, select, transform, replace);

	gga(population);

	return ListScheduler::process(graph, population.best_element());
}

double GeneticListScheduler::evaluate(const chromosome_t &chromosome) const
{
	/* Make a new schedule */
	schedule_t schedule = ListScheduler::process(graph);

	graph->assign_schedule(schedule);

	if (graph->duration > graph->deadline)
		return std::numeric_limits<double>::min();

	matrix_t dynamic_power, temperature, total_power;

	/* The graph is rescheduled now, obtain the dynamic power profile */
	DynamicPower::compute(graph, tunning.sampling_interval, dynamic_power);

	/* Now, we can get the temperature profile, and the total power profile
	 * including the leakage part.
	 */
	unsigned int it = hotspot->solve(graph->architecture,
		dynamic_power, temperature, total_power);

	return Lifetime::predict(temperature, tunning.sampling_interval);
}
