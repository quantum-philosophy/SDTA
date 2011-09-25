#ifndef __LIST_SCHEDULER_H__
#define __LIST_SCHEDULER_H__

#include "common.h"
#include "Genetics.h"
#include "Architecture.h"
#include "Graph.h"

typedef std::list<tid_t> list_schedule_t;

class ListScheduler
{
	const processor_vector_t &processors;
	const task_vector_t &tasks;

	public:

	ListScheduler(const Architecture &architecture, const Graph &graph) :
		processors(architecture.processors), tasks(graph.tasks) {}

	Schedule process(const layout_t &layout, const priority_t &priority) const;

	protected:

	virtual void push(list_schedule_t &pool,
		const priority_t &priority, tid_t id) const = 0;
	virtual tid_t pull(list_schedule_t &pool,
		const priority_t &priority) const = 0;

	bool ready(const Task *task, const bit_string_t &scheduled) const;
};

class DeterministicListScheduler: public ListScheduler
{
	public:

	DeterministicListScheduler(const Architecture &architecture,
		const Graph &graph) : ListScheduler(architecture, graph) {}

	protected:

	void push(list_schedule_t &pool, const priority_t &priority, tid_t id) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority) const;
};

class StochasticListScheduler: public ListScheduler
{
	public:

	StochasticListScheduler(const Architecture &architecture,
		const Graph &graph) : ListScheduler(architecture, graph) {}

	void push(list_schedule_t &pool, const priority_t &priority, tid_t id) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority) const;
};

class RandomGeneratorListScheduler: public ListScheduler
{
	public:

	RandomGeneratorListScheduler(const Architecture &architecture,
		const Graph &graph) : ListScheduler(architecture, graph) {}

	void push(list_schedule_t &pool, const priority_t &priority, tid_t id) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority) const;
};

template<class CT>
class ListScheduleMutation: public ListScheduler, public eoMonOp<CT>
{
	const bool fixed_layout;
	const layout_t &layout;
	const rate_t &rate;

	double current_rate;

	public:

	ListScheduleMutation(const constrains_t &constrains, const rate_t &_rate,
		const Architecture &architecture, const Graph &graph) :

		ListScheduler(architecture, graph),
		fixed_layout(constrains.fixed_layout()),
		layout(constrains.get_layout()), rate(_rate) {}

	inline bool operator()(CT &chromosome)
	{
		current_rate = rate.get();

		Schedule schedule;

		if (fixed_layout) {
			schedule = process(layout, chromosome);
		}
		else {
			/* Should be encoded in the chromosome */
			layout_t layout;
			priority_t priority;

			GeneEncoder::split(chromosome, layout, priority);

			schedule = process(layout, priority);
		}

		chromosome.set_schedule(schedule);
		GeneEncoder::order(chromosome);

		/* NOTE: We always say that nothing has changed, since
		 * the invalidation takes place in set_schedule. The purpose
		 * is to keep the already computed schedule valid,
		 * but the price becomes invalid.
		 */
		return false;
	}

	void push(list_schedule_t &pool, const priority_t &priority, tid_t id) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority) const;
};

#include "ListScheduler.hpp"

#endif
