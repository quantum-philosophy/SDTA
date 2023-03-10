#include "Replacement.h"

template<class CT>
void SimilarityReplacement<CT>::operator()(eoPop<CT> &parents, eoPop<CT> &offspring)
{
	size_t parent_size = parents.size();
	size_t offspring_size = offspring.size();

	bit_string_t replaced(parent_size, false);
	std::vector<size_t> places(parent_size);

	double distance, min_distance;

	int i;
	size_t j, count;

	offspring.sort();

	/* Let the weakest choose first */
	for (i = offspring_size - 1; i >= 0; i--) {
		const CT &chromosome = offspring[i];
		typename CT::fitness_t fitness = chromosome.fitness();

		/* Always less or equal to 1 */
		min_distance = 2;

		/* Number of the most similar */
		count = 0;

		for (j = 0; j < parent_size; j++) {
			if (replaced[j]) continue;

			distance = Chromosome::distance(chromosome, parents[j]);

			if (distance < min_distance) {
				min_distance = distance;
				count = 0;
			}

			if (distance <= min_distance) {
				if (fitness > parents[j].fitness())
					places[count++] = j;
			}
		}

		if (!count) continue;

		j = Random::number(count);

		parents[places[j]] = chromosome;
		replaced[places[j]] = true;
	}
}

template<class CT>
void ElitismMerge<CT>::operator()(const population_t &population,
	population_t &offspring)
{
	size_t population_size = population.size();
	size_t count = (rate < 1) ? (rate * population_size) : rate;

#ifndef SHALLOW_CHECK
	if (count > population_size)
		throw std::runtime_error("The elite size is invalid.");
#endif

	std::vector<const CT *> elite;
	population.nth_element(count, elite);

	for (size_t i = 0; i < count; i++)
		offspring.push_back(*elite[i]);
}
