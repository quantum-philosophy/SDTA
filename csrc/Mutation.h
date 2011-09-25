#ifndef __MUTATION_H__
#define __MUTATION_H__

#include "ListScheduler.h"

template<class CT>
class UniformMutation: public eoMonOp<CT>
{
	const constrains_t &constrains;
	const rate_t &rate;

	public:

	UniformMutation(const constrains_t &_constrains, const rate_t &_rate) :
		constrains(_constrains), rate(_rate) {}

	bool operator()(CT &one);
};

template<class CT>
class PeerMutation: public eoMonOp<CT>
{
	const constrains_t &constrains;
	const rate_t &rate;

	public:

	PeerMutation(const constrains_t &_constrains, const rate_t &_rate) :
		constrains(_constrains), rate(_rate) {}

	bool operator()(CT &one);
};

template<class CT>
class Mutation: public eoMonOp<CT>
{
	eoMonOp<CT> *mutate;

	const MutationTuning &tuning;
	EvolutionStats &stats;
	const rate_t rate;

	public:

	Mutation(const Architecture &architecture, const Graph &graph,
		const constrains_t &constrains, const MutationTuning &_tuning,
		EvolutionStats &_stats) :

		mutate(NULL), tuning(_tuning), stats(_stats),
		rate(tuning.min_rate, tuning.scale, tuning.exponent, stats.generations)
	{
		if (tuning.method == "uniform")
			mutate = new UniformMutation<CT>(constrains, rate);

		else if (tuning.method == "peer")
			mutate = new PeerMutation<CT>(constrains, rate);

		else if (tuning.method == "list_schedule")
			mutate = new ListScheduleMutation<CT>(constrains, rate,
				architecture, graph);

		else throw std::runtime_error("The mutation method is unknown.");
	}

	~Mutation()
	{
		__DELETE(mutate);
	}

	inline bool operator()(CT &one)
	{
		stats.mutation_rate = rate.get();

		if ((*mutate)(one)) {
			one.set_invalid();
			return true;
		}

		return false;
	}
};

#include "Mutation.hpp"

#endif
