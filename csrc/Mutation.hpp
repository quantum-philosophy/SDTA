#include "Mutation.h"
#include "Neighborhood.h"

template<class CT>
bool UniformMutation<CT>::operator()(CT &chromosome)
{
	double rate = this->rate.get();

	size_t size = chromosome.size();
	bool changed = false;
	rank_t prev, next;

	for (size_t i = 0; i < size; i++) {
		const constrain_t &constrain = constrains[i];
		if (constrain.tight() || !Random::flip(rate)) continue;

		prev = chromosome[i];
		do next = constrain.random(); while (prev == next);

		chromosome[i] = next;
		changed = true;
	}

	return changed;
}
