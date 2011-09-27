#include "Selection.h"

template<class CT>
void RouletteSelection<CT>::setup(const population_t &population)
{
	population_size = population.size();
	rank.resize(population_size);
	children.resize(population_size);

	size_t i, j;
	double fitness;

	for (i = 0; i < population_size; i++) {
		fitness = population[i].fitness();

		rank[i] = 0;
		children[i] = 0;

		for (j = 0; j < population_size; j++)
			if (population[j].fitness() < fitness) rank[i]++;
	}
}

template<class CT>
const CT &RouletteSelection<CT>::operator()(const population_t &population)
{
	size_t i;
	double total = 0;
	std::vector<double> chance(population_size);

	for (i = 0; i < population_size; i++) {
		chance[i] = double(rank[i]) / double(children[i] + 1);
		total += chance[i];
	}

	double roulette = Random::uniform(total);

	i = 0;
	while ((roulette -= chance[i]) > 0) i++;

	children[i]++;

	return population[i];
}

template<class CT>
const CT &TournamentSelection<CT>::operator()(const population_t &population)
{
	size_t population_size = population.size();
	size_t best = Random::number(population_size);
	size_t i, next;

	/* NOTE: Here we have 1, since we have already taken one above */
	for (i = 1; i < size; i++) {
		next = Random::number(population_size);
		if (population[best].fitness() < population[next].fitness())
			best = next;
	}

	return population[best];
}
