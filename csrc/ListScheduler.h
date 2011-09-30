#ifndef __LIST_SCHEDULER_H__
#define __LIST_SCHEDULER_H__

#include "common.h"
#include "Genetics.h"
#include "Architecture.h"
#include "Processor.h"
#include "Graph.h"
#include "Task.h"
#include "Schedule.h"
#include "Evaluation.h"
#include "Pool.h"

class BasicListScheduler
{
	public:

	virtual Schedule process(const layout_t &layout, const priority_t &priority,
		void *data = NULL) const = 0;
};

template<class PT>
class ListScheduler: public BasicListScheduler
{
	protected:

	const processor_vector_t &processors;
	const task_vector_t &tasks;

	public:

	ListScheduler(const Architecture &architecture, const Graph &graph) :
		processors(architecture.processors), tasks(graph.tasks) {}

	Schedule process(const layout_t &layout, const priority_t &priority,
		void *data = NULL) const;

	protected:

	inline bool ready(const Task *task, const bit_string_t &scheduled) const
	{
		size_t parent_count = task->parents.size();

		for (size_t i = 0; i < parent_count; i++)
			if (!scheduled[task->parents[i]->id]) return false;

		return true;
	}
};

typedef ListScheduler<DeterministicPool> DeterministicListScheduler;
typedef ListScheduler<RandomPool> RandomGeneratorListScheduler;
typedef ListScheduler<EarliestProcessorPool> EarliestProcessorListScheduler;

template<class CT>
class ListScheduleCrossover:
	public ListScheduler<CrossoverPool>,
	public eoQuadOp<CT>
{
	size_t points;
	const layout_t *layout;
	const rate_t &rate;

	public:

	ListScheduleCrossover(const Architecture &architecture, const Graph &graph,
		size_t _points, const constrains_t &constrains, const rate_t &_rate) :

		ListScheduler<CrossoverPool>(architecture, graph), points(_points),
		layout(constrains.fixed_layout() ? &constrains.get_layout() : NULL),
		rate(_rate)
	{
		if (points < 1)
			throw std::runtime_error("The number of crossover points should be at least one.");
	}

	bool operator()(CT &one, CT &another);
};

template<class CT>
class ListScheduleMutation:
	public ListScheduler<MutationPool>,
	public eoMonOp<CT>
{
	const layout_t *layout;
	const rate_t &rate;

	public:

	ListScheduleMutation(const Architecture &architecture, const Graph &graph,
		const constrains_t &constrains, const rate_t &_rate) :

		ListScheduler<MutationPool>(architecture, graph),
		layout(constrains.fixed_layout() ? &constrains.get_layout() : NULL),
		rate(_rate) {}

	bool operator()(CT &chromosome);
};

template<class CT>
class ListScheduleTraining:
	public ListScheduler<TrainingPool>,
	public eoMonOp<CT>
{
	size_t max_lessons;
	size_t stall_lessons;
	Evaluation &evaluation;
	const layout_t *layout;
	const rate_t &rate;

	public:

	ListScheduleTraining(const Architecture &architecture, const Graph &graph,
		size_t _max_lessons, size_t _stall_lessons, Evaluation &_evaluation,
		const constrains_t &constrains, const rate_t &_rate) :

		ListScheduler<TrainingPool>(architecture, graph),
		max_lessons(_max_lessons), stall_lessons(_stall_lessons),
		evaluation(_evaluation),
		layout(constrains.fixed_layout() ? &constrains.get_layout() : NULL),
		rate(_rate) {}

	bool operator()(CT &chromosome);
};

#include "ListScheduler.hpp"

#endif
