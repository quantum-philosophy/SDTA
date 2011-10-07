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
class ListScheduleTraining:
	public ListScheduler<TrainingPool>,
	public eoMonOp<CT>
{
	size_t max_lessons;
	size_t stall_lessons;
	Evaluation &evaluation;
	const layout_t *layout;
	const rate_t &rate;

	public:

	ListScheduleTraining(const Architecture &architecture, const Graph &graph,
		size_t _max_lessons, size_t _stall_lessons, Evaluation &_evaluation,
		const constrains_t &constrains, const rate_t &_rate) :

		ListScheduler<TrainingPool>(architecture, graph),
		max_lessons(_max_lessons), stall_lessons(_stall_lessons),
		evaluation(_evaluation),
		layout(constrains.fixed_layout() ? &constrains.get_layout() : NULL),
		rate(_rate) {}

	bool operator()(CT &chromosome);
};

template<class CT>
class Training: public eoMonOp<CT>, public eoAlgo<CT>
{
	eoMonOp<CT> *train;

	const TrainingTuning &tuning;
	BasicEvolutionStats &stats;
	const rate_t rate;

	public:

	Training(const Architecture &architecture, const Graph &graph,
		const constrains_t &constrains, Evaluation &evaluation,
		eoEvalFunc<CT> &evaluate, const TrainingTuning &_tuning,
		BasicEvolutionStats &_stats) :

		train(NULL), tuning(_tuning), stats(_stats),
		rate(tuning.min_rate, tuning.scale, tuning.exponent, stats.generations)
	{
		if (tuning.method == "peer")
			train = new PeerTraining<CT>(tuning.max_lessons,
				tuning.stall_lessons, evaluate, constrains, rate);

		else if (tuning.method == "list_schedule")
			train = new ListScheduleTraining<CT>(architecture, graph,
				tuning.max_lessons, tuning.stall_lessons, evaluation,
				constrains, rate);

		else throw std::runtime_error("The training method is unknown.");
	}

	~Training()
	{
		__DELETE(train);
	}

	inline bool operator()(CT &one)
	{
		stats.training_rate = rate.get();
		(void)(*train)(one);
		return false;
	}

	inline void operator()(eoPop<CT> &population)
	{
		stats.training_rate = rate.get();
		size_t size = population.size();
		for (size_t i = 0; i < size; i++) (*train)(population[i]);
	}
};

#include "Training.hpp"

#endif
