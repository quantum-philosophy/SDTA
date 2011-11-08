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

template<class CT>
bool PeerCrossover<CT>::operator()(CT &one, CT &another)
{
	double rate = this->rate.get();

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

template<class CT>
bool ListScheduleCrossover<CT>::operator()(CT &one, CT &another)
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
		else {
			turn[i] = true;
			select_points--;
		}
	}
	while (select_points);

	Schedule schedule;

	{
		CrossoverPool::data_t data(one, another, turn);

		if (layout) {
			schedule = ListScheduler<CrossoverPool>::process(
				*layout, one, &data);
		}
		else {
			/* Should be encoded in the chromosome */
			layout_t layout;
			priority_t priority;

			GeneEncoder::split(one, priority, layout);

			schedule = ListScheduler<CrossoverPool>::process(
				layout, priority, &data);
		}

		one.set_schedule(schedule);
	}

	{
		CrossoverPool::data_t data(another, one, turn);

		if (layout) {
			schedule = ListScheduler<CrossoverPool>::process(
				*layout, another, &data);
		}
		else {
			/* Should be encoded in the chromosome */
			layout_t layout;
			priority_t priority;

			GeneEncoder::split(another, priority, layout);

			schedule = ListScheduler<CrossoverPool>::process(
				layout, priority, &data);
		}

		another.set_schedule(schedule);
	}

	/* NOTE: We always say that nothing has changed, since
	 * the invalidation takes place in set_schedule. The purpose
	 * is to keep the already computed schedule valid,
	 * but the price becomes invalid.
	 */
	return false;
}
