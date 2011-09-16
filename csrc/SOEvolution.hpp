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
	int pos, last_pos = -1;
	size_t chromosome_length;

	evaluate(population);
	chromosome_t chromosome = select(population);

	chromosome_length = chromosome.size();
	population.reserve(population.size());

	do {
		population.clear();
		population.push_back(chromosome);

		do { pos = eo::random(chromosome_length); }
		while (pos == last_pos);

		const constrain_t &constrain = constrains[pos];
		rank_t best_rank = chromosome[pos];

		for (rank_t rank = constrain.min; rank <= constrain.max; rank++) {
			if (rank == best_rank) continue;
			chromosome[pos] = rank;
			chromosome.invalidate();
			population.push_back(chromosome);
		}

		evaluate(population);
		chromosome = select(population);

		last_pos = pos;
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
