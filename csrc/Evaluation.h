#ifndef __EVALUATION_H__
#define __EVALUATION_H__

#include "common.h"
#include "Genetics.h"

class Evaluation
{
	const Architecture &architecture;
	const Graph &graph;
	const Hotspot &hotspot;

	public:

	Evaluation(const Architecture &_architecture, const Graph &_graph,
		const Hotspot &_hotspot) :

		architecture(_architecture), graph(_graph), hotspot(_hotspot) {}

	price_t process(const Schedule &schedule, bool shallow = false) const;
};

#endif
