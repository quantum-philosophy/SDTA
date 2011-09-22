#ifndef __TRAINING_H__
#define __TRAINING_H__

template<class CT>
class Training: public eoMonOp<CT>
{
	typedef bool (Training<CT>::*method_t)(CT &, double);

	eoEvalFunc<CT> &evaluate;
	const constrains_t &constrains;
	const TrainingTuning &tuning;
	const rate_t rate;
	EvolutionStats &stats;
	method_t method;

	public:

	Training(eoEvalFunc<CT> &_evaluate, const constrains_t &_constrains,
		const TrainingTuning &_tuning, EvolutionStats &_stats) :

		evaluate(_evaluate), constrains(_constrains), tuning(_tuning),
		rate(tuning.min_rate, tuning.scale, tuning.exponent, _stats.generations),
		stats(_stats)
	{
		if (tuning.method == "peer") method = &Training::peer;
		else throw std::runtime_error("The training method is unknown.");
	}

	inline bool operator()(CT &one)
	{
		return (this->*method)(one, stats.training_rate = rate.get());
	}

	private:

	bool peer(CT &one, double rate);
};

#include "Training.hpp"

#endif
