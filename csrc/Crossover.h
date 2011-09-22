#ifndef __CROSSOVER_H__
#define __CROSSOVER_H__

template<class CT>
class Crossover: public eoQuadOp<CT>
{
	typedef bool (Crossover<CT>::*method_t)(CT &, CT &, double);

	const constrains_t &constrains;
	const CrossoverTuning &tuning;
	const rate_t rate;
	EvolutionStats &stats;
	method_t method;

	public:

	Crossover(const constrains_t &_constrains,
		const CrossoverTuning &_tuning, EvolutionStats &_stats) :

		constrains(_constrains), tuning(_tuning),
		rate(tuning.min_rate, tuning.scale, tuning.exponent, _stats.generations),
		stats(_stats)
	{
		if (tuning.method == "uniform") method = &Crossover<CT>::uniform;
		else if (tuning.method == "npoint") method = &Crossover<CT>::npoint;
		else if (tuning.method == "peer") method = &Crossover<CT>::uniform;
		else throw std::runtime_error("The crossover method is unknown.");
	}

	inline bool operator()(CT &one, CT &another)
	{
		return (this->*method)(one, another, stats.crossover_rate = rate.get());
	}

	private:

	bool uniform(CT &one, CT &another, double rate);
	bool npoint(CT &one, CT &another, double rate);
	bool peer(CT &one, CT &another, double rate);
};

#include "Crossover.hpp"

#endif
