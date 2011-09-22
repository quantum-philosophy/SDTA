#ifndef __SELECTION_H__
#define __SELECTION_H__

template <class CT>
class RouletteSelection: public eoSelectOne<CT>
{
	public:

	typedef CT chromosome_t;
	typedef eoPop<chromosome_t> population_t;

	RouletteSelection() : eoSelectOne<chromosome_t>() {}

	void setup(const population_t &population);

	const chromosome_t &operator()(const population_t &population);

	private:

	size_t population_size;
	std::vector<size_t> rank;
	std::vector<size_t> children;
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

template<class CT>
class Selection: public eoSelect<CT>
{
	const SelectionTuning &tuning;

	public:

	Selection(const SelectionTuning &_tuning) : tuning(_tuning)
	{
		if (tuning.ratio < 0 || tuning.ratio > 1)
			throw std::runtime_error("The selection ratio is invalid.");

		if (tuning.method == "roulette")
			method = new RouletteSelection<CT>();

		else if (tuning.method == "tournament")
			method = new TournamentSelection<CT>(tuning.tournament_size);

		else throw std::runtime_error("The selection method is unknown.");
	}

	~Selection()
	{
		delete method;
	}

	void operator()(const eoPop<CT> &source, eoPop<CT> &destination)
	{
		size_t size = tuning.ratio * (double)source.size();

		destination.resize(size);

		method->setup(source);

		for (size_t i = 0; i < size; i++)
			destination[i] = (*method)(source);
	}

	private:

	eoSelectOne<CT> *method;
};

#include "Selection.hpp"

#endif
