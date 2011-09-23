#ifndef __TRAINING_H__
#define __TRAINING_H__

template<class CT>
class PeerTraining: public eoMonOp<CT>
{
	size_t max_lessons;
	size_t stall_lessons;
	eoEvalFunc<CT> &evaluate;
	const constrains_t &constrains;
	const rate_t &rate;

	public:

	PeerTraining(size_t _max_lessons, size_t _stall_lessons,
		eoEvalFunc<CT> &_evaluate, const constrains_t &_constrains,
		const rate_t &_rate) :

		max_lessons(_max_lessons), stall_lessons(_stall_lessons),
		evaluate(_evaluate), constrains(_constrains), rate(_rate) {}

	bool operator()(CT &one);
};

template<class CT>
class Training: public eoMonOp<CT>
{
	eoMonOp<CT> *train;

	const TrainingTuning &tuning;
	EvolutionStats &stats;
	const rate_t rate;

	public:

	Training(eoEvalFunc<CT> &_evaluate, const constrains_t &constrains,
		const TrainingTuning &_tuning, EvolutionStats &_stats) :

		train(NULL), tuning(_tuning), stats(_stats),
		rate(tuning.min_rate, tuning.scale, tuning.exponent, stats.generations)
	{
		if (tuning.method == "peer")
			train = new PeerTraining<CT>(tuning.max_lessons,
				tuning.stall_lessons, _evaluate, constrains, rate);

		else throw std::runtime_error("The training method is unknown.");
	}

	~Training()
	{
		__DELETE(train);
	}

	inline bool operator()(CT &one)
	{
		stats.training_rate = rate.get();
		return (*train)(one);
	}
};

#include "Training.hpp"

#endif
