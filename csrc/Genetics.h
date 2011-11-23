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

#ifdef EXTENDED_STATS
	template<class CT>
	static bool equal(const CT &one, const CT &another)
	{
		size_t count = one.size();

		for (size_t i = 0; i < count; i++)
			if (one[i] != another[i]) return false;

		return true;
	}
#endif
};

class GeneEncoder;

template<class CT, class PT, class ST>
class Evolution;

template<class FT>
class eslabChromosome
{
	template<class CT, class PT, class ST>
	friend class Evolution;

	protected:

	price_t price;

	public:

	inline void set_price(const price_t &price)
	{
		this->price = price;
		set_fitness(price);
	}

	inline const price_t &get_price() const
	{
		return price;
	}

	protected:

	virtual void set_fitness(const price_t &price) = 0;
};

template<class CT>
class eslabPop: public eoPop<CT>
{
	public:

	typedef typename CT::fitness_t fitness_t;

#ifdef EXTENDED_STATS
	size_t unique() const;
	double diversity() const;
#endif
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
	static inline void reallocate(CT &chromosome, const Schedule &schedule)
	{
		const step_t * const mapping = schedule.point_mapping();
		const size_t task_count = schedule.task_count;

#ifndef SHALLOW_CHECK
		if (2 * task_count != chromosome.size())
			throw std::runtime_error("The chromosome is too short for reallocation.");
#endif

		for (size_t i = 0; i < task_count; i++)
			chromosome[task_count + i] = (rank_t)mapping[i];
	}

	template<class CT>
	static inline void reorder(CT &chromosome, const Schedule &schedule)
	{
		const step_t * const order = schedule.point_order();
		const size_t task_count = schedule.task_count;

		for (size_t i = 0; i < task_count; i++)
			chromosome[order[i]] = (rank_t)i;
	}
};

#include "Genetics.hpp"

#endif
