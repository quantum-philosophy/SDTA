#ifndef __MUTATION_H__
#define __MUTATION_H__

template<class CT>
class Mutation: public eoMonOp<CT>
{
	typedef bool (Mutation<CT>::*method_t)(CT &, double);

	const constrains_t &constrains;
	const MutationTuning &tuning;
	const rate_t rate;
	EvolutionStats &stats;
	method_t method;

	public:

	Mutation(const constrains_t &_constrains,
		const MutationTuning &_tuning, EvolutionStats &_stats) :

		constrains(_constrains), tuning(_tuning),
		rate(tuning.min_rate, tuning.scale, tuning.exponent, _stats.generations),
		stats(_stats)
	{
		if (tuning.method == "uniform") method = &Mutation<CT>::uniform;
		else if (tuning.method == "peer") method = &Mutation<CT>::uniform;
		else throw std::runtime_error("The mutation method is unknown.");
	}

	inline bool operator()(CT &one)
	{
		return (this->*method)(one, stats.mutation_rate = rate.get());
	}

	private:

	bool uniform(CT &one, double rate);
	bool peer(CT &one, double rate);
};

#include "Mutation.hpp"

#endif
