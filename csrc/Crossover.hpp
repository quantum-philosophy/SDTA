#include "Crossover.h"

template<class CT>
bool eslabNPtsBitCrossover<CT>::perform(CT &one, CT &another, double rate)
{
	if (!Random::flip(rate)) return false;

	size_t i;
	size_t size = one.size();
	size_t select_points = points;

#ifndef SHALLOW_CHECK
	if (size != another.size())
		throw std::runtime_error("The chromosomes have different size.");
#endif

	std::vector<bool> turn_points(size, false);

	do {
		i = 1 + Random::number(size - 1);

		if (turn_points[i]) continue;
		else {
			turn_points[i] = true;
			select_points--;
		}
	}
	while (select_points);

	bool change = false;

	for (i = 1; i < size; i++) {
		if (turn_points[i]) change = !change;
		if (change) {
			rank_t tmp = one[i];
			one[i] = another[i];
			another[i] = tmp;
		}
	}

	return true;
}

template<class CT>
bool eslabPeerCrossover<CT>::perform(CT &one, CT &another, double rate)
{
	if (!Random::flip(rate)) return false;

	size_t first, peer, peer_count, size = one.size();

#ifndef SHALLOW_CHECK
	if (size != another.size())
		throw std::runtime_error("The chromosomes have different size.");
#endif

	first = Random::number(size);
	peer_count = constrains[first].peers.size();

	rank_t rank;

	rank = one[first];
	one[first] = another[first];
	another[first] = rank;

	for (size_t i = 0; i < peer_count; i++) {
		peer = constrains[first].peers[i];
		rank = one[peer];
		one[peer] = another[peer];
		another[peer] = rank;
	}

	return true;
}
