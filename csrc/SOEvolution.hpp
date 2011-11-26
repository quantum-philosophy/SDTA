#include "SOEvolution.h"

/******************************************************************************/
/* Genetic Algorithm                                                          */
/******************************************************************************/

template<class CT>
void eslabSOGeneticAlgorithm<CT>::operator()(population_t &population)
{
#ifndef SHALLOW_CHECK
	size_t population_size = population.size();;
#endif

	population_t offspring;

	/* Initial evaluation */
#ifdef PRECISE_TIMEOUT
	if (!evaluate(population)) return;
#else
	evaluate(population);
#endif

	do {
		/* Select */
		select(population, offspring);

		/* Transform = Crossover + Mutate */
		transform(offspring);

		/* Evaluate newcomers */
#ifdef PRECISE_TIMEOUT
		if (!evaluate(offspring)) {
			typename population_t::iterator it;

			for (it = offspring.begin(); it != offspring.end();) {
				if (it->invalid()) it = offspring.erase(it);
				else it++;
			}

			replace(population, offspring);
			return;
		}
#else
		evaluate(offspring);
#endif

		/* Evolve */
		replace(population, offspring);

#ifndef SHALLOW_CHECK
		if (population.size() != population_size)
			throw std::runtime_error("The size of the population changes.");
#endif
	}
	while (continuator(population));
}
