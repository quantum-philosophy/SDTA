#ifndef __CROSSOVER_H__
#define __CROSSOVER_H__

template<class CT>
class eslabCrossover: public eoQuadOp<CT>
{
	public:

	eslabCrossover(const rate_t &_rate) : rate(_rate) {}

	inline bool operator()(CT &one, CT &another)
	{
		return perform(one, another, rate.get());
	}

	protected:

	virtual bool perform(CT &one, CT &another, double rate) = 0;

	const rate_t &rate;
};

template<class CT>
class eslabNPtsBitCrossover: public eslabCrossover<CT>
{
	size_t points;

	public:

	eslabNPtsBitCrossover(size_t _points, const rate_t &_rate) :
		eslabCrossover<CT>(_rate), points(_points)
	{
		if (points < 1)
			std::runtime_error("The number of crossover points is invalid.");
	}

	protected:

	bool perform(CT &one, CT &another, double rate);
};

template<class CT>
class eslabPeerCrossover: public eslabCrossover<CT>
{
	const constrains_t &constrains;

	public:

	eslabPeerCrossover(const constrains_t &_constrains, const rate_t &_rate) :
		eslabCrossover<CT>(_rate), constrains(_constrains) {}

	protected:

	bool perform(CT &one, CT &another, double rate);
};

#include "Crossover.hpp"

#endif
