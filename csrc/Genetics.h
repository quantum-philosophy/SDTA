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

template<class CT, class PT, class ST>
class GenericEvolution;

template<class FT>
class eslabChromosome
{
	friend class GeneEncoder;

	template<class CT, class PT, class ST>
	friend class GenericEvolution;

	protected:

	bool invalid_schedule;
	bool invalid_price;

	Schedule schedule;
	price_t price;

	public:

	eslabChromosome() : invalid_schedule(true), invalid_price(true) {}

	inline bool valid() const
	{
		return !invalid_schedule && !invalid_price;
	}

	inline void set_invalid()
	{
		invalid_schedule = true;
		invalid_price = true;
	}

	inline bool valid_schedule() const
	{
		return !invalid_schedule;
	}

	inline void set_schedule(const Schedule &schedule)
	{
		this->schedule = schedule;
		invalid_schedule = false;

		/* Automatically invalidate the price */
		invalid_price = true;
	}

	inline void set_price(const price_t &price)
	{
		this->price = price;
		set_fitness(price);
		invalid_price = false;
	}

	inline const Schedule &get_schedule() const
	{
#ifndef SHALLOW_CHECK
		if (invalid_schedule)
			throw std::runtime_error("The schedule is invalid.");
#endif

		return schedule;
	}

	inline const price_t &get_price() const
	{
#ifndef SHALLOW_CHECK
		if (invalid_price)
			throw std::runtime_error("The price is invalid.");
#endif

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
	static inline void reallocate(CT &chromosome)
	{
		const Schedule &schedule = chromosome.schedule;
		const mapping_t &mapping = schedule.mapping;
		const size_t task_count = schedule.task_count;

#ifndef SHALLOW_CHECK
		if (2 * task_count != chromosome.size())
			throw std::runtime_error("The chromosome is too short for reallocation.");
#endif

		for (size_t i = 0; i < task_count; i++)
			chromosome[2 * i] = (rank_t)mapping[i];
	}

	template<class CT>
	static inline void reorder(CT &chromosome)
	{
		const Schedule &schedule = chromosome.schedule;
		const order_t &order = schedule.order;
		const size_t task_count = schedule.task_count;

		for (size_t i = 0; i < task_count; i++)
			chromosome[order[i]] = (rank_t)i;
	}
};

#include "Genetics.hpp"

#endif
