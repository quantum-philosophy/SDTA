#ifndef __CONTINUATION_H__
#define __CONTINUATION_H__

template<class CT>
class Continuation: public eoContinue<CT>
{
	typedef typename CT::fitness_t fitness_t;

	const ContinuationTuning &tuning;

	size_t generations;
	size_t stall_generations;
	fitness_t last_fitness;

	public:

	Continuation(const ContinuationTuning &_tuning) : tuning(_tuning)
	{
		reset();
	}

	virtual bool operator()(const eoPop<CT> &population)
	{
		generations++;

		if (generations < tuning.min_generations) return true;
		if (generations >= tuning.max_generations) return false;

		fitness_t current_fitness = population.best_element().fitness();

		if (current_fitness > last_fitness) {
			stall_generations = 0;
			last_fitness = current_fitness;
			return true;
		}

		stall_generations++;

		if (stall_generations >= tuning.stall_generations) return false;

		return true;
	}

	void reset()
	{
		generations = 0;
		stall_generations = 0;
		last_fitness = fitness_t();
	}
};

#endif
