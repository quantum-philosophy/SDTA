#ifndef __SELECTION_H__
#define __SELECTION_H__

template <class CT>
class DominanceRouletteSelection: public eoSelectOne<CT>
{
	public:

	void setup(const eoPop<CT> &population);

	const CT &operator()(const eoPop<CT> &population);

	private:

	size_t population_size;
	std::vector<size_t> rank;
	std::vector<size_t> children;
};

template <class CT>
class RankRouletteSelection: public eoSelectOne<CT>
{
	public:

	RankRouletteSelection(double _factor = -0.5) : factor(_factor) {}

	void setup(const eoPop<CT> &population);

	const CT &operator()(const eoPop<CT> &population);

	private:

	double factor;
	double total;
	std::vector<double> chance;
};

template <class CT>
class TournamentSelection: public eoSelectOne<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	TournamentSelection(size_t _size) :
		eoSelectOne<chromosome_t>(), size(_size)
	{
		if (size < 2)
			throw std::runtime_error("The tournament size should be at least 2.");
	}

	const chromosome_t &operator()(const population_t &population);

	private:

	const size_t size;
};

template <class CT>
class UniformSelection: public eoSelectOne<CT>
{
	public:

	inline const CT &operator()(const eoPop<CT> &population)
	{
		return population[Random::number(population.size())];
	}
};

template<class CT>
class Selection: public eoSelect<CT>
{
	eoSelectOne<CT> *select;

	const SelectionTuning &tuning;

	public:

	Selection(const SelectionTuning &_tuning) :
		select(NULL), tuning(_tuning)
	{
		if (tuning.ratio < 0 || tuning.ratio > 1)
			throw std::runtime_error("The selection ratio is invalid.");

		if (tuning.method == "dominance_roulette")
			select = new DominanceRouletteSelection<CT>();

		else if (tuning.method == "rank_roulette")
			select = new RankRouletteSelection<CT>(tuning.ranking_factor);

		else if (tuning.method == "tournament")
			select = new TournamentSelection<CT>(tuning.tournament_size);

		else if (tuning.method == "uniform")
			select = new UniformSelection<CT>();

		else throw std::runtime_error("The selection method is unknown.");
	}

	~Selection()
	{
		__DELETE(select);
	}

	inline void operator()(const eoPop<CT> &source, eoPop<CT> &destination)
	{
		size_t size = tuning.ratio * (double)source.size();

		destination.resize(size);

		select->setup(source);

		for (size_t i = 0; i < size; i++)
			destination[i] = (*select)(source);
	}

	inline void append(const eoPop<CT> &source, eoPop<CT> &destination, size_t size)
	{
		select->setup(source);

		for (size_t i = 0; i < size; i++)
			destination.push_back((*select)(source));
	}
};

#include "Selection.hpp"

#endif
