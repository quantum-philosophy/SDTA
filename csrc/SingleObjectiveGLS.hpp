/******************************************************************************/
/* eslabEvolution                                                             */
/******************************************************************************/

template<class chromosome_t>
void eslabEvolution<chromosome_t>::operator()(eoPop<chromosome_t> &population)
{
	size_t population_size = population.size();;
	eoPop<chromosome_t> offspring;

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

template<class chromosome_t>
void eslabEvolution<chromosome_t>::evaluate(
	eoPop<chromosome_t> &population) const
{
	apply<chromosome_t>(evaluate_one, population);
}

/******************************************************************************/
/* eslabElitismMerge                                                          */
/******************************************************************************/

template<class chromosome_t>
eslabElitismMerge<chromosome_t>::eslabElitismMerge(double _rate) : rate(_rate)
{
	if (rate < 0 || rate > 1)
		std::runtime_error("The elitism rate is invalid.");
}

template<class chromosome_t>
void eslabElitismMerge<chromosome_t>::operator()(
	const eoPop<chromosome_t> &population,
	eoPop<chromosome_t> &offspring)
{
	size_t count = rate * population.size();

	std::vector<const chromosome_t *> elite;
	population.nth_element(count, elite);

	for (size_t i = 0; i < count; i++)
		offspring.push_back(*elite[i]);
}
