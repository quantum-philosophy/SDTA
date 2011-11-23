#ifndef __POOL_H__
#define __POOL_H__

#include "common.h"
#include "GraphAnalysis.h"
#include "Hotspot.h"

class Pool: public std::list<tid_t>
{
	template<class PT>
	friend class ListScheduler;

	public:

	Pool(const processor_vector_t &_processors, const task_vector_t &_tasks,
		const layout_t &_layout, const priority_t &_priority, void *_data = NULL) :

		processor_count(_processors.size()), task_count(_tasks.size()),
		processor_time(processor_count, 0), task_time(task_count, 0),
		processed(task_count, false), scheduled(task_count, false),
		layout(_layout), priority(_priority)
	{
	}

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

	DeterministicPool(const processor_vector_t &_processors, const task_vector_t &_tasks,
		const layout_t &_layout, const priority_t &_priority, void *_data = NULL) :

		Pool(_processors, _tasks, _layout, _priority, _data) {}

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

	RandomPool(const processor_vector_t &_processors, const task_vector_t &_tasks,
		const layout_t &_layout, const priority_t &_priority, void *_data = NULL) :

		Pool(_processors, _tasks, _layout, _priority, _data) {}

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

class EarliestProcessorPool: public DeterministicPool
{
	public:

	EarliestProcessorPool(const processor_vector_t &_processors, const task_vector_t &_tasks,
		const layout_t &_layout, const priority_t &_priority, void *_data = NULL) :

		DeterministicPool(_processors, _tasks, _layout, _priority, _data) {}

	virtual void pull(pid_t &pid, tid_t &id)
	{
		pid_t earliest = 0;

		for (pid_t i = 1; i < processor_count; i++)
			if (processor_time[earliest] > processor_time[i])
				earliest = i;

		id = front();
		pid = earliest;

		pop_front();
	}
};

class CriticalityPool: public Pool
{
	protected:

	vector_t sc;

	const processor_vector_t &processors;
	const task_vector_t &tasks;

	matrix_t power;
	matrix_t time;
	vector_t energy;

	public:

	CriticalityPool(const processor_vector_t &_processors, const task_vector_t &_tasks,
		const layout_t &_layout, const priority_t &_priority, void *_data = NULL) :

		Pool(_processors, _tasks, _layout, _priority, _data),
		processors(_processors), tasks(_tasks)
	{
		sc = GraphAnalysis::statical_criticality(processors, tasks);

		power.resize(task_count, processor_count);
		time.resize(task_count, processor_count);

		for (size_t i = 0; i < task_count; i++) {
			unsigned int type = tasks[i]->get_type();
			for (size_t j = 0; j < processor_count; j++) {
				power[i][j] = processors[j]->calc_power(type);
				time[i][j] = processors[j]->calc_duration(type);
			}
		}
	}

	virtual inline void push(tid_t id)
	{
		push_back(id);
	}

	virtual void pull(pid_t &best_pid, tid_t &best_id)
	{
		iterator best_it;
		double dc, max_dc = -DBL_MAX;
		best_pid = best_id = 0;

		/* For all tasks in the pool */
		for (iterator it = begin(); it != end(); it++) {
			tid_t id = *it;
			for (pid_t pid = 0; pid < processor_count; pid++) {
				/* 1. Statical criticality */
				dc = sc[id];
				/* 2. Worst case execution time */
				dc -= time[id][pid];
				/* 3. Earliest start time */
				dc -= std::max(processor_time[pid], task_time[id]);
				/* 4. Some additional cost */
				dc -= estimate_cost(pid, id);

				if (dc > max_dc) {
					max_dc = dc;

					best_pid = pid;
					best_id = id;
					best_it = it;
				}
			}
		}

		confirm_cost(best_pid, best_id);

		erase(best_it);
	}

	protected:

	virtual inline double estimate_cost(pid_t pid, tid_t id)
	{
		return 0;
	}

	virtual inline void confirm_cost(pid_t pid, tid_t id)
	{
	}
};

class PowerCriticalityPool: public CriticalityPool
{
	public:

	struct data_t
	{
		double coefficient;
	};

	private:

	vector_t energy;

	data_t *data;

	public:

	PowerCriticalityPool(const processor_vector_t &_processors, const task_vector_t &_tasks,
		const layout_t &_layout, const priority_t &_priority, void *_data = NULL) :

		CriticalityPool(_processors, _tasks, _layout, _priority, _data)
	{
		if (!_data)
			throw std::runtime_error("The data is null.");

		energy.resize(processor_count);
		energy.nullify();

		data = (data_t *)_data;
	}

	protected:

	virtual inline double estimate_cost(pid_t pid, tid_t id)
	{
		double total_time = std::max(processor_time[pid], task_time[id]) + time[id][pid];
		double total_energy = energy[pid] + time[id][pid] * power[id][pid];
		return data->coefficient * total_energy / total_time;
	}

	virtual inline void confirm_cost(pid_t pid, tid_t id)
	{
		energy[pid] += time[id][pid] * power[id][pid];
	}
};

class TemperatureCriticalityPool: public CriticalityPool
{
	public:

	struct data_t
	{
		Hotspot *hotspot;
		double coefficient;
	};

	private:

	vector_t energy;

	matrix_t average_power;
	matrix_t temperature;

	data_t *data;

	public:

	TemperatureCriticalityPool(const processor_vector_t &_processors, const task_vector_t &_tasks,
		const layout_t &_layout, const priority_t &_priority, void *_data = NULL) :

		CriticalityPool(_processors, _tasks, _layout, _priority, _data)
	{
		if (!_data)
			throw std::runtime_error("The data is null.");

		energy.resize(processor_count);
		energy.nullify();

		average_power.resize(1, processor_count);

		data = (data_t *)_data;
	}

	protected:

	virtual inline double estimate_cost(pid_t pid, tid_t id)
	{
		double total_time = std::max(processor_time[pid], task_time[id]) + time[id][pid];

		for (size_t i = 0; i < processor_count; i++)
			if (i == pid)
				average_power[0][i] = (energy[i] + time[id][i] * power[id][i]) / total_time;
			else
				average_power[0][i] = energy[i] / total_time;

		data->hotspot->solve(average_power, temperature);

		double Tmax = -DBL_MAX;

		for (size_t i = 0; i < processor_count; i++)
			if (temperature[0][i] > Tmax) Tmax = temperature[0][i];

		return data->coefficient * Tmax;
	}

	virtual inline void confirm_cost(pid_t pid, tid_t id)
	{
		energy[pid] += time[id][pid] * power[id][pid];
	}
};

class CrossoverPool: public Pool
{
	public:

	struct data_t
	{
		private:

		const priority_t &initial;
		const priority_t &alternative;
		const bit_string_t &turn;
		bool changed;
		size_t position;

		public:

		data_t(const priority_t &_initial, const priority_t &_alternative,
			const bit_string_t &_turn) :

			initial(_initial), alternative(_alternative),
			turn(_turn), changed(false), position(0) {}

		inline rank_t operator[](size_t i) const
		{
			if (changed) return alternative[i];
			else return initial[i];
		}

		inline void tick()
		{
#ifndef SHALLOW_CHECK
			if (position > turn.size())
				throw std::runtime_error("The turn vector is broken.");
#endif
			if (turn[position]) changed = !changed;
			position++;
		}
	};

	CrossoverPool(const processor_vector_t &_processors, const task_vector_t &_tasks,
		const layout_t &_layout, const priority_t &_priority, void *_data) :

		Pool(_processors, _tasks, _layout, _priority)
	{
#ifndef SHALLOW_CHECK
		if (!_data)
			throw std::runtime_error("The data is missing.");
#endif
		data = (data_t *)_data;
	}

	virtual inline void push(tid_t id)
	{
		push_back(id);
	}

	virtual inline void pull(pid_t &pid, tid_t &id)
	{
		data_t &data = *(this->data);

		data.tick();

		iterator best, it;

		best = it = begin();

		for (; it != end(); it++)
			if (data[*it] < data[*best])
				best = it;

		id = *best;
		pid = layout[id];

		erase(best);
	}

	private:

	data_t *data;
};

class MutationPool: public DeterministicPool
{
	double rate;

	public:

	MutationPool(const processor_vector_t &_processors, const task_vector_t &_tasks,
		const layout_t &_layout, const priority_t &_priority, void *_data) :

		DeterministicPool(_processors, _tasks, _layout, _priority)
	{
#ifndef SHALLOW_CHECK
		if (!_data) throw std::runtime_error("There is not data.");
#endif
		rate = *((double *)_data);
	}

	virtual void pull(pid_t &pid, tid_t &id)
	{
		iterator it = begin();
		size_t count = size();

		if (count > 1 && Random::flip(rate)) {
			size_t choice = Random::number(count - 1) + 1;
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

	TrainingPool(const processor_vector_t &_processors, const task_vector_t &_tasks,
		const layout_t &_layout, const priority_t &_priority, void *_data) :

		DeterministicPool(_processors, _tasks, _layout, _priority),
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
