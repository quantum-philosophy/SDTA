#ifndef __EVALUATION_H__
#define __EVALUATION_H__

#include "common.h"
#include "ListScheduler.h"

class Evaluation
{
	public:

	Evaluation(const Architecture &_architecture, const Graph &_graph,
		const Hotspot &_hotspot) :

		architecture(_architecture), graph(_graph), hotspot(_hotspot) {}

	price_t process(const Schedule &schedule, bool shallow = false) const;
	price_t process(const layout_t &layout, const priority_t &priority,
		bool shallow = false) const;

	template<class CT>
	void assess(CT &chromosome, const layout_t &layout,
		bool shallow = false) const
	{
		const Schedule schedule = ListScheduler::process(
			architecture, graph, layout, chromosome);

		price_t price = process(schedule, shallow);

		chromosome.assess(schedule, price);
	}

	template<class CT>
	void assess(CT &chromosome, bool shallow = false) const
	{
		layout_t layout;
		priority_t priority;

		GeneEncoder::split(chromosome, layout, priority);

		const Schedule schedule = ListScheduler::process(
			architecture, graph, layout, priority);

		price_t price = process(schedule, shallow);

		chromosome.assess(schedule, price);
	}

	private:

	const Architecture &architecture;
	const Graph &graph;
	const Hotspot &hotspot;
};

#endif
