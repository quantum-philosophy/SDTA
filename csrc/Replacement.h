#ifndef __REPLACEMENT_H__
#define __REPLACEMENT_H__

template <class CT>
class FulfillingReplacement: public eoReplacement<CT>
{
	public:

	FulfillingReplacement(eoMerge<CT> & _merge, eoReduce<CT> &_reduce,
		Selection<CT> &_select) :
		merge(_merge), reduce(_reduce), select(_select) {}

	void operator()(eoPop<CT> &parents, eoPop<CT> &offspring)
	{
		size_t parent_size = parents.size();

		merge(parents, offspring);

		size_t offspring_size = offspring.size();

		int mistmatch = parent_size - offspring_size;

		if (mistmatch > 0) {
			/* Fulfil! */
			select.append(parents, offspring, mistmatch);
		}
		else if (mistmatch < 0) {
			/* Shrink! */
			reduce(offspring, parent_size);
		}

		parents.swap(offspring);
	}

	private:

	eoMerge<CT> &merge;
	eoReduce<CT> &reduce;
	Selection<CT> &select;
};

template <class CT>
class SimilarityReplacement: public eoReplacement<CT>
{
	public:

	SimilarityReplacement() {}

	void operator()(eoPop<CT> &parents, eoPop<CT> &offspring);
};

template<class CT>
class ElitismMerge: public eoMerge<CT>
{
	typedef eoPop<CT> population_t;

	public:

	ElitismMerge(double _rate) : rate(_rate)
	{
		if (rate < 0)
			std::runtime_error("The elitism rate is invalid.");
	}

	void operator()(const population_t &source, population_t &destination);

	private:

	double rate;
};

template <class CT>
class KillerReduction: public eoReduce<CT>
{
	public:

	void operator()(eoPop<CT> &population, unsigned size)
	{
		int kill_count = population.size() - size;

		if (kill_count < 0)
			throw std::runtime_error("Cannot reduce such a small population.");

		for (size_t i = 0; i < kill_count; i++) {
			typename eoPop<CT>::iterator it = population.it_worse_element();
			population.erase(it);
		}
	}
};

#include "Replacement.hpp"

#endif
