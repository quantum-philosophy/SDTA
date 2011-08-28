#include <iostream>

#include "GeneticListScheduler.h"
#include "Graph.h"
#include "ListScheduler.h"

GeneticListScheduler::GeneticListScheduler(Graph *_graph, Hotspot *_hotspot,
	const tunning_t &_tunning): graph(_graph), hotspot(_hotspot),
	tunning(_tunning)
{
}

void GeneticListScheduler::solve(void)
{
	rng.reseed(tunning.seed);

	/* Continuator */
	eoGenContinue<chromosome_t> gen_cont(tunning.max_generations);
	eoSteadyFitContinue<chromosome_t> steady_cont(tunning.min_generations,
		tunning.stall_generations);
	eoCombinedContinue<chromosome_t> continuator(gen_cont, steady_cont);

	/* Evaluate */
	GeneticListSchedulerEvalFuncPtr evaluate(this);

	eoPop<chromosome_t> population;

	for (unsigned int i = 0; i < tunning.population_size; i++) {
		chromosome_t chromosome;

		for (unsigned int j = 0; j < tunning.chromosome_length; j++) {
			gene_t gene = rng.uniform();
			chromosome.push_back(gene);
		}

		evaluate(chromosome);
		population.push_back(chromosome);
	}

	population.sort();

	std::cout << "Initial population" << std::endl << population << std::endl;

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

eoPop<chromosome_t> GeneticListScheduler::create_population() const
{
	eoPop<chromosome_t> population;

	for (size_t i = 0; i < tunning.population_size; i++) {
		chromosome_t chromosome;

		for (size_t j = 0; j < tunning.chromosome_length; j++) {
			gene_t gene = rng.uniform();
			chromosome.push_back(gene);
		}

		evaluate(chromosome);
		population.push_back(chromosome);
	}
}
