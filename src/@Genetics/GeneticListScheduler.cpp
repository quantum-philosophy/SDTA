#include <stdexcept>
#include <iostream>

#include "GeneticListScheduler.h"
#include "Graph.h"
#include "Task.h"
#include "ListScheduler.h"

void GeneticListScheduler::solve(Graph *graph, Hotspot *hotspot)
{
	size_t task_count = graph->task_count;

	if (task_count == 0) throw std::runtime_error("The graph is empty.");

	task_vector_t &tasks = graph->tasks;

	this->graph = graph;
	this->hotspot = hotspot;

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

	population.sort();

	std::cout << "Initial population" << std::endl << population << std::endl;

	return;

	/* Select */
	eoDetTournamentSelect<chromosome_t> selectOne(tunning.tournament_size);
	eoSelectPerc<chromosome_t> select(selectOne);

	/* Crossover */
	eoNPtsBitXover<chromosome_t> crossover(tunning.crossover_points);

	/* Mutate */
	gene_t min_gene = 0.0;
	gene_t max_gene = 1.0;
	eoUniformRangeMutation mutation(min_gene, max_gene,
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

	population.sort();
	std::cout << "Final population" << std::endl << population << std::endl;
}

double GeneticListScheduler::evaluate(const chromosome_t &chromosome) const
{
	unsigned length = chromosome.size();
	double sum = 0;

	for (unsigned i = 0; i < length; i++)
		sum += chromosome[i] * chromosome[i];

	return -sum;
}
