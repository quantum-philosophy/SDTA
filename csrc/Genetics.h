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
};

class GeneEncoder;

template<class FT>
class eslabChromosome
{
	friend class GeneEncoder;

	public:

	typedef FT fitness_t;

	inline void assess(const Schedule &schedule, const price_t &price)
	{
		m_schedule = schedule;
		assign_fitness(price);
	}

	inline const Schedule &schedule() const
	{
		return m_schedule;
	}

	virtual bool bad() const = 0;

	protected:

	virtual void assign_fitness(const price_t &price) = 0;

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

class GeneEncoder
{
	public:

	template<class CT, class ET>
	static inline void encode(CT &chromosome, const ET &encoding)
	{
		size_t size = encoding.size();

		chromosome.resize(size);

		for (size_t i = 0; i < size; i++)
			chromosome[i] = encoding[i];
	}

	template<class CT, class ET>
	static inline void extend(CT &chromosome, const ET &encoding)
	{
		size_t offset = chromosome.size();
		size_t size = encoding.size();

		chromosome.resize(offset + size);

		for (size_t i = 0; i < size; i++)
			chromosome[offset + i] = encoding[i];
	}

	template<class CT, class ET1, class ET2>
	static inline void split(const CT &chromosome, ET1 &chunk1, ET2 &chunk2,
		size_t size1 = 0, size_t size2 = 0)
	{
		size_t length = chromosome.size();

		if (size1 == 0) {
			size1 = length / 2;
			chunk1.resize(size1);
		}
		else if (size1 > length)
			throw std::runtime_error("Cannot split.");

		if (size2 == 0) {
			size2 = length - size1;
			chunk2.resize(size2);
		}
		else if (size1 + size2 > length)
			throw std::runtime_error("Cannot split.");

		for (size_t i = 0; i < size1; i++)
			chunk1[i] = chromosome[i];

		for (size_t i = 0; i < size2; i++)
			chunk2[i] = chromosome[size1 + i];
	}

	template<class CT>
	static inline void order(CT &chromosome)
	{
		const Schedule &schedule = chromosome.m_schedule;
		const order_t &order = schedule.order;
		const size_t task_count = schedule.task_count;

		for (size_t i = 0; i < task_count; i++)
			chromosome[order[i]] = (rank_t)i;
	}
};

#include "Genetics.hpp"

#endif
