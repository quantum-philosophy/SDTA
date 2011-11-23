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

#endif
