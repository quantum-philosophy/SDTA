#include "Genetics.h"

template<class CT>
double Chromosome::distance(const CT &one, const CT &another)
{
	size_t size = one.size();

#ifndef SHALLOW_CHECK
	if (another.size() != size)
		throw std::runtime_error("The chromosomes have different length.");
#endif

	size_t count = 0;

	for (size_t i = 0; i < size; i++)
		if (one[i] != another[i]) count++;

	return double(count) / double(size);
}

template<class CT>
bool Chromosome::equal(const CT &one, const CT &another)
{
	size_t size = one.size();

#ifndef SHALLOW_CHECK
	if (another.size() != size)
		throw std::runtime_error("The chromosomes have different length.");
#endif

	for (size_t i = 0; i < size; i++)
		if (one[i] != another[i]) return false;

	return true;
}

template<class CT>
size_t eslabPop<CT>::unique() const
{
	size_t population_size = this->size();

	if (!population_size) return 0;

	std::vector<bool> done(population_size, false);

	size_t count = 0;

	for (size_t i = 0; i < population_size; i++) {
		if (done[i]) continue;

		count++;

		for (size_t j = i + 1; j < population_size; j++)
			if (Chromosome::equal((*this)[i], (*this)[j])) done[j] = true;
	}

	return count;
}

template<class CT>
double eslabPop<CT>::diversity() const
{
	size_t population_size = this->size();

	if (!population_size) return 0;

	double value = 0;
	size_t total = 0;

	for (size_t i = 0; i < population_size - 1; i++)
		for (size_t j = i + 1; j < population_size; j++) {
			value += Chromosome::distance((*this)[i], (*this)[j]);
			total++;
		}

	return value / (double)total;
}
