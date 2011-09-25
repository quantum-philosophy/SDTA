#include "Transformation.h"

template<class CT>
void Transformation<CT>::operator()(population_t &population)
{
	size_t i;
	size_t population_size = population.size();
	size_t crossover_count = population_size / 2;

	/* NOTE: Every operator invalidates for itself */

	/* 1. Crossover */
	for (i = 0; i < crossover_count; i++)
		(void)crossover(population[2 * i], population[2 * i + 1]);

	/* 2. Mutation */
	for (i = 0; i < population_size; i++)
		(void)mutate(population[i]);

	/* 3. Training */
	for (i = 0; i < population_size; i++)
		(void)train(population[i]);
}
