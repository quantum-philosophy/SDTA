#ifndef __TRANSFORMATION_H__
#define __TRANSFORMATION_H__

template<class CT>
class Transformation: public eoTransform<CT>
{
	typedef eoPop<CT> population_t;

	public:

	Transformation(eoQuadOp<CT> &_crossover, eoMonOp<CT> &_mutate,
		eoMonOp<CT> &_train) :

		crossover(_crossover), mutate(_mutate), train(_train) {}

	void operator()(population_t &population);

	private:

	eoQuadOp<CT> &crossover;
	eoMonOp<CT> &mutate;
	eoMonOp<CT> &train;
};

#include "Transformation.hpp"

#endif
