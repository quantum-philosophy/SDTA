#include "Transformation.h"

template<class CT>
void Transformation<CT>::operator()(population_t &population)
{
	size_t i;
	size_t population_size = population.size();
	size_t crossover_count = population_size / 2;

	bit_string_t changes(population_size, false);
	bool changed;

	/* 1. Crossover */
	for (i = 0; i < crossover_count; i++) {
		changed = crossover(population[2 * i], population[2 * i + 1]);
		changes[2 * i] = changed;
		changes[2 * i + 1] = changed;
	}

	/* 2. Mutation */
	for (i = 0; i < population_size; i++) {
		changed = mutate(population[i]);
		changes[i] = changes[i] || changed;
	}

	/* 3. Invalidation */
	for (i = 0; i < population_size; i++)
		if (changes[i]) population[i].invalidate();
}
