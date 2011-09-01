#include "MultiGLS.h"

void MultiGLS::evolve(population_t &populatino,
	eoContinue &continuator, eoTransform &transform)
{
	eslabEvaluate evaluate(this);

	moeoNSGAII<chromosome_t> ga(continuator, evaluate, transform);

	ga(population);
}

eslabObjectiveVector MultiGLS::evaluate(const chromosome_t &chromosome)
{
	eslabObjectiveVector objective;

	objective[AGING_OBJECTIVE] = aging;
	objective[ENERGY_OBJECTIVE] = energy;

	return objective;
}
