#ifndef __POOL_H__
#define __POOL_H__

#include "common.h"

class Pool: public std::list<tid_t>
{
	template<class PT>
	friend class ListScheduler;

	public:

	Pool(size_t _processor_count, size_t _task_count, const layout_t &_layout,
		const priority_t &_priority, void *_data = NULL);

	virtual void push(tid_t id) = 0;
	virtual void pull(pid_t &pid, tid_t &id) = 0;

	protected:

	const size_t processor_count;
	const size_t task_count;

	vector_t processor_time;
	vector_t task_time;

	bit_string_t processed;
	bit_string_t scheduled;

	const layout_t &layout;
	const priority_t &priority;
};

class DeterministicPool: public Pool
{
	public:

	DeterministicPool(size_t _processor_count, size_t _task_count,
		const layout_t &_layout, const priority_t &_priority, void *_data = NULL) :

		Pool(_processor_count, _task_count, _layout, _priority, _data) {}

	virtual inline void push(tid_t id)
	{
		iterator it;
		rank_t new_priority = priority[id];

		for (it = begin(); it != end(); it++)
			if (new_priority < priority[*it]) break;

		insert(it, id);
	}

	virtual inline void pull(pid_t &pid, tid_t &id)
	{
		id = front();
		pid = layout[id];

		pop_front();
	}
};

class RandomPool: public Pool
{
	public:

	RandomPool(size_t _processor_count, size_t _task_count,
		const layout_t &_layout, const priority_t &_priority, void *_data = NULL) :

		Pool(_processor_count, _task_count, _layout, _priority, _data) {}

	virtual void push(tid_t id)
	{
		iterator it = begin();

		if (!priority.empty()) {
			/* Deterministic scheduling */
			rank_t new_priority = priority[id];
			for (; it != end(); it++)
				if (new_priority < priority[*it]) break;
		}
		/* else (Random scheduling) */

		insert(it, id);
	}

	virtual void pull(pid_t &pid, tid_t &id)
	{
		iterator it = begin();

		if (priority.empty()) {
			/* Random scheduling */
			size_t choice = Random::number(size());
			for (size_t i = 0; i < choice; i++) it++;
		}
		/* else (Deterministic scheduling) */

		id = *it;
		if (layout.empty()) pid = Random::number(processor_count);
		else pid = layout[id];

		erase(it);
	}
};

class MutationPool: public DeterministicPool
{
	double rate;

	public:

	MutationPool(size_t _processor_count, size_t _task_count,
		const layout_t &_layout, const priority_t &_priority, void *_data) :

		DeterministicPool(_processor_count, _task_count, _layout, _priority)
	{
#ifndef SHALLOW_CHECK
		if (!_data) throw std::runtime_error("There is not data.");
#endif
		rate = *((double *)_data);
	}

	virtual void pull(pid_t &pid, tid_t &id)
	{
		iterator it = begin();

		if (Random::flip(rate)) {
			size_t choice = Random::number(size());
			for (size_t i = 0; i < choice; i++) it++;
		}

		id = *it;
		pid = layout[id];

		erase(it);
	}
};

class TrainingPool: public DeterministicPool
{
	public:

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

	TrainingPool(size_t _processor_count, size_t _task_count,
		const layout_t &_layout, const priority_t &_priority, void *_data) :

		DeterministicPool(_processor_count, _task_count, _layout, _priority),
		data((data_t *)_data)
	{
#ifndef SHALLOW_CHECK
		if (!data) throw std::runtime_error("There is not data.");
#endif
	}

	virtual inline void pull(pid_t &pid, tid_t &id)
	{
		iterator it = begin();

		if (data->checkpoint(size())) {
			size_t direction = data->direction();
			for (size_t i = 0; i < direction; i++) it++;
		}

		id = *it;
		pid = layout[id];

		erase(it);
	}

	private:

	data_t *data;
};

#endif
