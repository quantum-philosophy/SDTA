using namespace std;

void GeneticListScheduler::solve(void)
{
	rng.reseed(options.seed);

	/* Continuator */
	eoGenContinue<chromosome_t> gen_cont(options.max_generations);
	eoSteadyFitContinue<chromosome_t> steady_cont(options.min_generations,
		options.stall_generations);
	eoCombinedContinue<chromosome_t> continuator(gen_cont, steady_cont);

	/* Evaluate */
	GeneticListSchedulerEvalFuncPtr evaluate(this);

	eoPop<chromosome_t> population;

	for (unsigned int i = 0; i < options.population_size; i++) {
		chromosome_t chromosome;

		for (unsigned int j = 0; j < options.chromosome_length; j++) {
			gene_t gene = rng.uniform();
			chromosome.push_back(gene);
		}

		evaluate(chromosome);
		population.push_back(chromosome);
	}

	population.sort();

	cout << "Initial population" << endl << population << endl;

	/* Select */
	eoDetTournamentSelect<chromosome_t> selectOne(options.tournament_size);
	eoSelectPerc<chromosome_t> select(selectOne);

	/* Crossover */
	eoNPtsBitXover<chromosome_t> crossover(options.crossover_points);

	/* Mutate */
	gene_t min_gene = 0.0;
	gene_t max_gene = 1.0;
	eoUniformRangeMutation mutation(min_gene, max_gene,
		options.mutation_points);

	/* Evolve */
	eoElitism<chromosome_t> merge(options.generation_gap);
	eoTruncate<chromosome_t> reduce;
	eoMergeReduce<chromosome_t> replace(merge, reduce);

	/* Transform */
	eoSGATransform<chromosome_t> transform(crossover, options.crossover_rate,
		mutation, options.mutation_rate);

	/* Monitor */
	eoCheckPoint<chromosome_t> checkpoint(continuator);
	eoMatlabMonitor monitor(population);
	checkpoint.add(monitor);

	eoEasyEA<chromosome_t> gga(checkpoint, evaluate, select, transform, replace);

	gga(population);

	population.sort();
	cout << "Final population" << endl << population << endl;
}

double GeneticListScheduler::evaluate(const chromosome_t &chromosome) const
{
	unsigned length = chromosome.size();
	double sum = 0;

	for (unsigned i = 0; i < length; i++)
		sum += chromosome[i] * chromosome[i];

	return -sum;
}
