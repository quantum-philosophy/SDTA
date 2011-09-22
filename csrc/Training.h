#ifndef __TRAINING_H__
#define __TRAINING_H__

template<class CT>
class Training: public eoAlgo<CT>, public eoMonOp<CT>
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

	inline void operator()(eoPop<CT> &population)
	{
		double current_rate = stats.training_rate = rate.get();

		if (current_rate == 0) return;

		size_t size = population.size();

		for (size_t i = 0; i < size; i++)
			(void)(this->*method)(population[i], current_rate);
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
