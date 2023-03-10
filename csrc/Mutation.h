#ifndef __MUTATION_H__
#define __MUTATION_H__

#include "ListScheduler.h"
#include "Helper.h"

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
class Mutation: public eoMonOp<CT>
{
	const MutationTuning &tuning;
	BasicEvolutionStats &stats;
	const rate_t rate;

	method_list_t method_list;
	std::vector<eoMonOp<CT> *> methods;

	public:

	Mutation(const Architecture &architecture, const Graph &graph,
		const constrains_t &constrains, const MutationTuning &_tuning,
		BasicEvolutionStats &_stats) :

		tuning(_tuning), stats(_stats),
		rate(tuning.min_rate, tuning.scale, tuning.exponent, stats.generations)
	{
		method_list = Helper::method_list(_tuning.method);
		size_t count = method_list.size();

		if (count == 0)
			throw std::runtime_error("The mutation method is not selected.");

		for (size_t i = 0; i < count; i++) {
			eoMonOp<CT> *one;

			if (method_list[i].name == "uniform")
				one = new UniformMutation<CT>(constrains, rate);

			else throw std::runtime_error("The mutation method is unknown.");

			methods.push_back(one);
		}
	}

	~Mutation()
	{
		size_t count = methods.size();
		for (size_t i = 0; i < count; i++) __DELETE(methods[i]);
	}

	inline bool operator()(CT &one)
	{
		stats.mutation_rate = rate.get();
		return (*methods[method_list.choose()])(one);
	}
};

#include "Mutation.hpp"

#endif
