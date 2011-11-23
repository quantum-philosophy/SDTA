#ifndef __CROSSOVER_H__
#define __CROSSOVER_H__

#include "ListScheduler.h"
#include "Helper.h"

template<class CT>
class UniformCrossover: public eoQuadOp<CT>
{
	const rate_t &rate;

	public:

	UniformCrossover(const rate_t &_rate) : rate(_rate) {}

	bool operator()(CT &one, CT &another);
};

template<class CT>
class NPointCrossover: public eoQuadOp<CT>
{
	size_t points;
	const rate_t &rate;

	public:

	NPointCrossover(size_t _points, const rate_t &_rate) :
		points(_points), rate(_rate)
	{
		if (points < 1)
			throw std::runtime_error("The number of crossover points should be at least one.");
	}

	bool operator()(CT &one, CT &another);
};

template<class CT>
class PeerCrossover: public eoQuadOp<CT>
{
	const constrains_t &constrains;
	const rate_t &rate;

	public:

	PeerCrossover(const constrains_t &_constrains, const rate_t &_rate) :
		constrains(_constrains), rate(_rate) {}

	bool operator()(CT &one, CT &another);
};

template<class CT>
class Crossover: public eoQuadOp<CT>
{
	const CrossoverTuning &tuning;
	BasicEvolutionStats &stats;
	const rate_t rate;

	method_list_t method_list;
	std::vector<eoQuadOp<CT> *> methods;

	public:

	Crossover(const Architecture &architecture, const Graph &graph,
		const constrains_t &constrains, const CrossoverTuning &_tuning,
		BasicEvolutionStats &_stats) :

		tuning(_tuning), stats(_stats),
		rate(tuning.min_rate, tuning.scale, tuning.exponent, stats.generations)
	{
		method_list = Helper::method_list(_tuning.method);
		size_t count = method_list.size();

		if (count == 0)
			throw std::runtime_error("The crossover method is not selected.");

		for (size_t i = 0; i < count; i++) {
			eoQuadOp<CT> *one;

			if (method_list[i].name == "uniform")
				one = new UniformCrossover<CT>(rate);

			else if (method_list[i].name == "npoint")
				one = new NPointCrossover<CT>(tuning.points, rate);

			else throw std::runtime_error("The crossover method is unknown.");

			methods.push_back(one);
		}
	}

	~Crossover()
	{
		size_t count = methods.size();
		for (size_t i = 0; i < count; i++) __DELETE(methods[i]);
	}

	inline bool operator()(CT &one, CT &another)
	{
		stats.crossover_rate = rate.get();
		return (*methods[method_list.choose()])(one, another);
	}
};

#include "Crossover.hpp"

#endif
