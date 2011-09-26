#include "SOEvolution.h"

/******************************************************************************/
/* Genetic Algorithm                                                          */
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

		/* Train a bit? */
		train(population);

#ifndef SHALLOW_CHECK
		if (population.size() != population_size)
			throw std::runtime_error("The size of the population changes.");
#endif
	}
	while (continuator(population));
}
