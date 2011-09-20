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

#ifndef SHALLOW_CHECK
		if (population.size() != population_size)
			throw std::runtime_error("The size of the population changes.");
#endif
	}
	while (continuator(population));
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

#ifndef SHALLOW_CHECK
	if (count > population_size)
		throw std::runtime_error("The elite size is invalid.");
#endif

	std::vector<const CT *> elite;
	population.nth_element(count, elite);

	for (size_t i = 0; i < count; i++)
		offspring.push_back(*elite[i]);
}