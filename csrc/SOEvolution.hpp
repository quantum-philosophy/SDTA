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

#ifndef SHALLOW_CHECK
		if (population.size() != population_size)
			throw std::runtime_error("The size of the population changes.");
#endif
	}
	while (continuator(population));
}

/******************************************************************************/
/* Merge                                                                      */
/******************************************************************************/

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

/******************************************************************************/
/* Replacement                                                                */
/******************************************************************************/

template<class CT>
void SimilarReplacement<CT>::operator()(eoPop<CT> &parents, eoPop<CT> &offspring)
{
	size_t parent_size = parents.size();
	size_t offspring_size = offspring.size();

	bit_string_t replaced(parent_size, false);
	std::vector<size_t> places(parent_size);

	double distance, min_distance;
	size_t i, j, count;

	offspring.sort();

	for (i = 0; i < offspring_size; i++) {
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

		i = Random::number(count);

		parents[places[i]] = chromosome;
		replaced[places[i]] = true;
	}
}
