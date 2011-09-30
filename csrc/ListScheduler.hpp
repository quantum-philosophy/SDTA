#include "ListScheduler.h"

template<class PT>
Schedule ListScheduler<PT>::process(const layout_t &layout,
	const priority_t &priority, void *data) const
{
	tid_t id, cid;
	pid_t pid;
	const Task *task, *child;
	const Processor *processor;
	size_t i, count;
	double start, duration, finish;

	size_t processor_count = processors.size();
	size_t task_count = tasks.size();

	PT pool(processor_count, task_count, layout, priority, data);

	bit_string_t &processed = pool.processed;
	bit_string_t &scheduled = pool.scheduled;

	vector_t &processor_time = pool.processor_time;
	vector_t &task_time = pool.task_time;

	Schedule schedule(processor_count, task_count);

	for (id = 0; id < task_count; id++) {
		task = tasks[id];
		if (task->is_root()) {
			pool.push(id);
			processed[id] = true;
		}
	}

	while (!pool.empty()) {
		/* Get the next task */
		pool.pull(pid, id);

#ifndef SHALLOW_CHECK
		if (pid >= processor_count || id >= task_count)
			throw std::runtime_error("We are pulling something strange.");
#endif

		task = tasks[id];
		processor = processors[pid];

		/* Calculate its start time and duration */
		start = std::max(processor_time[pid], task_time[id]);
		duration = processor->calc_duration(task->get_type());
		finish = start + duration;

		processor_time[pid] = finish;

		/* Append to the schedule */
		schedule.append(pid, id, start, duration);
		scheduled[id] = true;

		/* Append children, but only those which are ready,
		 * and ensure absence of repetitions.
		 */
		count = task->children.size();
		for (i = 0; i < count; i++) {
			child = task->children[i];
			cid = child->id;

			/* Shift the child in time with respect to the parent */
			task_time[cid] = std::max(task_time[cid], finish);

			/* Prevent from considering the child once again */
			if (processed[cid]) continue;

			/* All parents should be scheduled */
			if (!ready(child, scheduled)) continue;

			pool.push(cid);
			processed[cid] = true;
		}
	}

	return schedule;
}

template<class CT>
bool ListScheduleCrossover<CT>::operator()(CT &one, CT &another)
{
	double rate = this->rate.get();

	if (!Random::flip(rate)) return false;

	size_t i;
	size_t size = one.size();
	size_t select_points = points;

#ifndef SHALLOW_CHECK
	if (size != another.size())
		throw std::runtime_error("The chromosomes have different size.");
#endif

	bit_string_t turn(size, false);

	do {
		i = 1 + Random::number(size - 1);

		if (turn[i]) continue;
		else {
			turn[i] = true;
			select_points--;
		}
	}
	while (select_points);

	{
		CrossoverPool::data_t data(one, another, turn);
		Schedule schedule = ListScheduler<CrossoverPool>::process(
			*layout, one, &data);
		one.set_schedule(schedule);
		GeneEncoder::reorder(one);
	}

	{
		CrossoverPool::data_t data(another, one, turn);
		Schedule schedule = ListScheduler<CrossoverPool>::process(
			*layout, another, &data);
		another.set_schedule(schedule);
		GeneEncoder::reorder(another);
	}

	return true;
}

template<class CT>
bool ListScheduleMutation<CT>::operator()(CT &chromosome)
{
	double current_rate = rate.get();
	void *data = (void *)&current_rate;

	Schedule schedule;

	if (layout) {
		schedule = ListScheduler<MutationPool>::process(
			*layout, chromosome, data);
	}
	else {
		/* Should be encoded in the chromosome */
		layout_t layout;
		priority_t priority;

		GeneEncoder::split(chromosome, layout, priority);

		schedule = ListScheduler<MutationPool>::process(
			layout, priority, data);
	}

	chromosome.set_schedule(schedule);
	GeneEncoder::reorder(chromosome);

	/* NOTE: We always say that nothing has changed, since
	 * the invalidation takes place in set_schedule. The purpose
	 * is to keep the already computed schedule valid,
	 * but the price becomes invalid.
	 */
	return false;
}

template<class CT>
bool ListScheduleTraining<CT>::operator()(CT &chromosome)
{
	double rate = this->rate.get();

	if (!Random::flip(rate)) return false;

	size_t task_count = tasks.size();

	layout_t layout;
	priority_t priority;

	if (this->layout) {
		layout = *(this->layout);
		priority = chromosome;
	}
	else {
		GeneEncoder::split(chromosome, priority, layout);
	}

	Schedule schedule, best_schedule;
	price_t start_price, best_price, price;

	bool improved = false;
	size_t lessons = 0, stall = 0;

	TrainingPool::data_t data(task_count);

	/* Collect all possible branches */
	schedule = process(layout, priority, (void *)&data);

	start_price = best_price = evaluation.process(schedule);

	while (stall < stall_lessons && lessons < max_lessons) {
		/* Choose one to inspect */
		if (!data.choose()) break;

		improved = false;

		/* Iterate through all branches */
		while (data.next()) {
			schedule = process(layout, priority, (void *)&data);
			price = evaluation.process(schedule);

			if (best_price.lifetime < price.lifetime) {
				/* We have found a better solution */
				best_schedule = schedule;
				best_price = price;

				improved = true;
			}
		}

		lessons++;

		if (improved) {
			stall = 0;

			priority = best_schedule.get_priority();

			/* Reset and recollect */
			data.reset();
			(void)process(layout, priority, (void *)&data);
		}
		else stall++;
	}

	if (start_price.lifetime >= best_price.lifetime) return false;

	chromosome.set_schedule(best_schedule);
	chromosome.set_price(best_price);
	GeneEncoder::reorder(chromosome);

	return true;
}
