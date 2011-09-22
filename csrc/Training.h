#ifndef __TRAINING_H__
#define __TRAINING_H__

template<class CT>
class eslabTraining: public eoMonOp<CT>
{
	public:

	eslabTraining(const rate_t &_rate) : rate(_rate) {}

	inline bool operator()(CT &chromosome)
	{
		return perform(chromosome, rate.get());
	}

	protected:

	virtual inline bool perform(CT &chromosome, double rate)
	{
		return false;
	}

	const rate_t &rate;
};

template<class CT>
class eslabPeerTraining: public eslabTraining<CT>
{
	public:

	eslabPeerTraining(const constrains_t &_constrains, eoEvalFunc<CT> &_evaluate,
		size_t _max_lessons, size_t _max_stall, const rate_t &_rate) :

		eslabTraining<CT>(_rate), constrains(_constrains),
		evaluate(_evaluate), max_lessons(_max_lessons), max_stall(_max_stall) {}

	protected:

	bool perform(CT &chromosome, double rate);

	const constrains_t &constrains;
	eoEvalFunc<CT> &evaluate;

	size_t max_lessons;
	size_t max_stall;
};

#include "Training.hpp"

#endif
