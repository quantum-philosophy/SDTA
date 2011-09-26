#ifndef __LIST_SCHEDULER_H__
#define __LIST_SCHEDULER_H__

#include "common.h"
#include "Genetics.h"
#include "Architecture.h"
#include "Graph.h"
#include "Evaluation.h"

typedef std::list<tid_t> list_schedule_t;

class ListScheduler
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

	virtual void push(list_schedule_t &pool, const priority_t &priority,
		tid_t id, void *data) const = 0;
	virtual tid_t pull(list_schedule_t &pool, const priority_t &priority,
		void *data) const = 0;

	bool ready(const Task *task, const bit_string_t &scheduled) const;
};

class DeterministicListScheduler: public ListScheduler
{
	public:

	DeterministicListScheduler(const Architecture &architecture,
		const Graph &graph) : ListScheduler(architecture, graph) {}

	protected:

	void push(list_schedule_t &pool, const priority_t &priority,
		tid_t id, void *data) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority,
		void *data) const;
};

class StochasticListScheduler: public ListScheduler
{
	public:

	StochasticListScheduler(const Architecture &architecture,
		const Graph &graph) : ListScheduler(architecture, graph) {}

	void push(list_schedule_t &pool, const priority_t &priority,
		tid_t id, void *data) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority,
		void *data) const;
};

class RandomGeneratorListScheduler: public ListScheduler
{
	public:

	RandomGeneratorListScheduler(const Architecture &architecture,
		const Graph &graph) : ListScheduler(architecture, graph) {}

	void push(list_schedule_t &pool, const priority_t &priority,
		tid_t id, void *data) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority,
		void *data) const;
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

	void push(list_schedule_t &pool, const priority_t &priority,
		tid_t id, void *data) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority,
		void *data) const;
};

template<class CT>
class ListScheduleTraining: public ListScheduler, public eoMonOp<CT>
{
	size_t max_lessons;
	size_t stall_lessons;
	const Evaluation &evaluation;
	const bool fixed_layout;
	const layout_t &layout;
	const rate_t &rate;

	struct data_t
	{
		data_t(size_t _size) : size(_size)
		{
			reset();
		}

		inline void reset()
		{
			count = 0;
			branching = false;
			branches = std::vector<size_t>(size, 0);
			point = 0;
		}

		inline bool choose()
		{
			if (count == 0) {
#ifndef SHALLOW_CHECK
				throw std::runtime_error("Cannot choose.");
#endif
				return false;
			}

			size_t i, j, pos = Random::number(count);

			for (i = 0, j = 0; i < size; i++)
				if (branches[i] > 1) {
					if (j == pos) break;
					else j++;
				}

			branching = true;
			trial = 0;
			branch_point = i;

			return true;
		}

		inline bool checkpoint(size_t number)
		{
#ifndef SHALLOW_CHECK
			if (point >= size)
				throw std::runtime_error("The data is broken.");
#endif

			if (branching) {
				return branch_point == point++;
			}
			else {
				if (number > 1) count++;
				branches[point++] = number;
				return false;
			}
		}

		inline size_t direction() const
		{
			return trial;
		}

		inline bool next()
		{
			point = 0;
			trial++;

			if (trial < branches[branch_point]) return true;

			return false;
		}

		private:

		const size_t size;
		size_t count;

		bool branching;
		std::vector<size_t> branches;
		int point;
		size_t branch_point;
		size_t trial;
	};

	public:

	ListScheduleTraining(size_t _max_lessons, size_t _stall_lessons,
		const Evaluation &_evaluation, const constrains_t &constrains,
		const rate_t &_rate, const Architecture &architecture, const Graph &graph) :

		ListScheduler(architecture, graph),
		max_lessons(_max_lessons), stall_lessons(_stall_lessons),
		evaluation(_evaluation), fixed_layout(constrains.fixed_layout()),
		layout(constrains.get_layout()), rate(_rate) {}

	bool operator()(CT &chromosome);

	void push(list_schedule_t &pool, const priority_t &priority,
		tid_t id, void *data) const;
	tid_t pull(list_schedule_t &pool, const priority_t &priority,
		void *data) const;
};

#include "ListScheduler.hpp"

#endif
