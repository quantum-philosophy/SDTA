#ifndef __CONTINUATION_H__
#define __CONTINUATION_H__

#include <time.h>

template<class CT>
class Continuation: public eoContinue<CT>
{
	protected:

	typedef typename CT::fitness_t fitness_t;

	const ContinuationTuning &tuning;

	size_t generations;
	time_t start;

	public:

	Continuation(const ContinuationTuning &_tuning) :
		tuning(_tuning)
	{
		reset();
	}

	inline bool timeout() const
	{
		if (tuning.time_limit <= 0) return false;
		return (time(0) - start) >= tuning.time_limit;
	}

	virtual bool operator()(const eoPop<CT> &population)
	{
		generations++;

		if (generations < tuning.min_generations) return true;
		if (generations >= tuning.max_generations) return false;

		if (timeout()) return false;

		return !stop(population);
	}

	virtual void reset()
	{
		generations = 0;
		start = time(0);
	}

	protected:

	virtual bool stop(const eoPop<CT> &population)
	{
		return false;
	}
};

#endif
