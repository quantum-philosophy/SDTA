#ifndef __GENETICS_H__
#define __GENETICS_H__

#include <eo>

#ifdef REAL_RANK
#include <es.h>
#else
#include <eoInt.h>
#endif

#include <ga/eoBitOp.h>

#include "Schedule.h"

template<class FT>
class eslabChromosome
{
	public:

	typedef FT fitness_t;

	inline void assess(const Schedule &schedule, const price_t &price)
	{
		m_schedule = schedule;
		fit(price);
	}

	inline const Schedule &schedule() const
	{
		return m_schedule;
	}

	virtual bool bad() const = 0;

	protected:

	virtual void fit(const price_t &price) = 0;

	Schedule m_schedule;
};

template<class CT>
class eslabPop: public eoPop<CT>
{
	public:

	typedef typename CT::fitness_t fitness_t;

	size_t unique() const;
	double diversity() const;
};

#endif
