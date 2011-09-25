#ifndef __CROSSOVER_H__
#define __CROSSOVER_H__

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
	eoQuadOp<CT> *crossover;

	const CrossoverTuning &tuning;
	EvolutionStats &stats;
	const rate_t rate;

	public:

	Crossover(const constrains_t &constrains,
		const CrossoverTuning &_tuning, EvolutionStats &_stats) :

		crossover(NULL), tuning(_tuning), stats(_stats),
		rate(tuning.min_rate, tuning.scale, tuning.exponent, stats.generations)
	{
		if (tuning.method == "uniform")
			crossover = new UniformCrossover<CT>(rate);

		else if (tuning.method == "npoint")
			crossover = new NPointCrossover<CT>(tuning.points, rate);

		else if (tuning.method == "peer")
			crossover = new PeerCrossover<CT>(constrains, rate);

		else throw std::runtime_error("The crossover method is unknown.");
	}

	~Crossover()
	{
		__DELETE(crossover);
	}

	inline bool operator()(CT &one, CT &another)
	{
		stats.crossover_rate = rate.get();

		if ((*crossover)(one, another)) {
			one.set_invalid();
			another.set_invalid();
			return true;
		}

		return false;
	}
};

#include "Crossover.hpp"

#endif
