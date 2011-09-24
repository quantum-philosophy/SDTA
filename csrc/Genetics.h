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

class Chromosome
{
	public:

	template<class CT>
	static double distance(const CT &one, const CT &another);

	template<class CT>
	static bool equal(const CT &one, const CT &another);

	template<class CT>
	static void order(CT &one);
};

template<class FT>
class eslabChromosome
{
	friend class Chromosome;

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

#include "Genetics.hpp"

#endif
