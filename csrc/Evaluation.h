#ifndef __EVALUATION_H__
#define __EVALUATION_H__

#include "common.h"

class Evaluation
{
	public:

	Evaluation(const Architecture &_architecture, const Graph &_graph,
		const Hotspot &_hotspot) :

		architecture(_architecture), graph(_graph), hotspot(_hotspot) {}

	price_t process(const Schedule &schedule, bool shallow = false) const;
	price_t process(const layout_t &layout, const priority_t &priority,
		bool shallow = false) const;

	private:

	const Architecture &architecture;
	const Graph &graph;
	const Hotspot &hotspot;
};

#endif
