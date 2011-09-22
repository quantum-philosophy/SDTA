#ifndef __MUTATION_H__
#define __MUTATION_H__

template<class CT>
class eslabMutation: public eoMonOp<CT>
{
	public:

	eslabMutation(const rate_t &_rate) : rate(_rate) {}

	inline bool operator()(CT &chromosome)
	{
		return perform(chromosome, rate.get());
	}

	protected:

	virtual bool perform(CT &chromosome, double rate) = 0;

	const rate_t &rate;
};

template<class CT>
class eslabUniformRangeMutation: public eslabMutation<CT>
{
	const constrains_t &constrains;

	public:

	eslabUniformRangeMutation(const constrains_t &_constrains, const rate_t &_rate) :
		eslabMutation<CT>(_rate), constrains(_constrains) {}

	protected:

	bool perform(CT &chromosome, double rate);
};

template<class CT>
class eslabPeerMutation: public eslabMutation<CT>
{
	public:

	eslabPeerMutation(const constrains_t &_constrains, const rate_t &_rate) :
		eslabMutation<CT>(_rate), constrains(_constrains) {}

	protected:

	bool perform(CT &chromosome, double rate);

	const constrains_t &constrains;
};

#include "Mutation.hpp"

#endif
