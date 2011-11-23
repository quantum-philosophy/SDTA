#include "Crossover.h"

template<class CT>
bool UniformCrossover<CT>::operator()(CT &one, CT &another)
{
	double rate = this->rate.get();

	if (!Random::flip(rate)) return false;

	size_t i, size = one.size();

#ifndef SHALLOW_CHECK
	if (size != another.size())
		throw std::runtime_error("The chromosomes have different size.");
#endif

	for (i = 0; i < size; i++) {
		if (!Random::flip(0.5)) continue;

		rank_t rank = one[i];
		one[i] = another[i];
		another[i] = rank;
	}

	return true;
}

template<class CT>
bool NPointCrossover<CT>::operator()(CT &one, CT &another)
{
	double rate = this->rate.get();

	if (!Random::flip(rate)) return false;

	size_t i;
	size_t size = one.size();
	size_t select_points = points;

#ifndef SHALLOW_CHECK
	if (size != another.size())
		throw std::runtime_error("The chromosomes have different size.");
#endif

	bit_string_t turn(size, false);

	do {
		i = 1 + Random::number(size - 1);

		if (turn[i]) continue;

		turn[i] = true;
		select_points--;
	}
	while (select_points);

	bool change = false;

	for (i = 1; i < size; i++) {
		if (turn[i]) change = !change;
		if (change) {
			rank_t tmp = one[i];
			one[i] = another[i];
			another[i] = tmp;
		}
	}

	return true;
}
