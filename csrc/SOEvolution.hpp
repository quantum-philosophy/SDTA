/******************************************************************************/
/* eslabSOGeneticAlgorithm                                                    */
/******************************************************************************/

template<class CT>
void eslabSOGeneticAlgorithm<CT>::operator()(population_t &population)
{
	size_t population_size = population.size();;
	population_t offspring;

	/* Initial evaluation */
	evaluate(population);

	do {
		/* Select */
		select(population, offspring);

		/* Transform = Crossover + Mutate */
		transform(offspring);

		/* Evaluate newcomers */
		evaluate(offspring);

		/* Evolve */
		replace(population, offspring);

		if (population.size() != population_size)
			throw std::runtime_error("The size of the population changes.");
	}
	while (continuator(population));
}

/******************************************************************************/
/* eslabSOLocalSearchAlgorithm                                                */
/******************************************************************************/

template<class CT>
void eslabSOLocalSearchAlgorithm<CT>::operator()(population_t &population)
{
	int pos;
	rank_t rank, last_rank;

	evaluate(population);

	chromosome_t new_one, temp = population.best_element();
	size_t chromosome_length = temp.size();

	const size_t step_count = chromosome_length;

	population.clear();
	population.push_back(temp);

	chromosome_t &best_one = population[0];

	double new_fitness, best_fitness = best_one.fitness();

	do {
		pos = eo::random(chromosome_length);

		const constrain_t &constrain = constrains[pos];

		last_rank = best_one[pos];
		new_one = best_one;

		for (size_t i = 0; i <= step_count; i++) {
			rank = last_rank - step_count / 2 + i;

			if (rank == last_rank ||
				rank < constrain.min ||
				rank > constrain.max) continue;

			new_one[pos] = rank;
			new_one.invalidate();
			evaluate_one(new_one);
			new_fitness = new_one.fitness();

			if (new_fitness > best_fitness) {
				best_fitness = new_fitness;
				best_one[pos] = rank;
				best_one.fitness(best_fitness);
			}
		}
	}
	while (continuator(population));
}

template<class CT>
const CT &eslabSOLocalSearchAlgorithm<CT>::select(
	population_t &population) const
{
	return population.best_element();
}

/******************************************************************************/
/* eslabElitismMerge                                                          */
/******************************************************************************/

template<class CT>
eslabElitismMerge<CT>::eslabElitismMerge(double _rate) : rate(_rate)
{
	if (rate < 0)
		std::runtime_error("The elitism rate is invalid.");
}

template<class CT>
void eslabElitismMerge<CT>::operator()(const population_t &population,
	population_t &offspring)
{
	size_t population_size, count;

	population_size = population.size();
	count = (rate < 1) ? (rate * population_size) : rate;

	if (count > population_size)
		throw std::runtime_error("The elite size is invalid.");

	std::vector<const CT *> elite;
	population.nth_element(count, elite);

	for (size_t i = 0; i < count; i++)
		offspring.push_back(*elite[i]);
}
