#include "ListScheduler.h"

template<class CT>
void ListScheduleMutation<CT>::push(pool_t &pool, tid_t id) const
{
	const layout_t &layout = pool.layout;
	const priority_t &priority = pool.priority;
	list_schedule_t &local_pool = pool[layout[id]];

	list_schedule_t::iterator it;
	rank_t new_priority = priority[id];

	for (it = local_pool.begin(); it != local_pool.end(); it++)
		if (new_priority < priority[*it]) break;

	local_pool.insert(it, id);
}

template<class CT>
tid_t ListScheduleMutation<CT>::pull(pool_t &pool, pid_t pid) const
{
	list_schedule_t &local_pool = pool[pid];

	tid_t id;

	list_schedule_t::iterator it = local_pool.begin();

	if (Random::flip(current_rate)) {
		size_t choice = Random::number(local_pool.size());
		for (size_t i = 0; i < choice; i++) it++;
	}

	id = *it;
	local_pool.erase(it);

	return id;
}

template<class CT>
bool ListScheduleTraining<CT>::operator()(CT &chromosome)
{
	double rate = this->rate.get();

	if (!Random::flip(rate)) return false;

	size_t task_count = tasks.size();

	layout_t layout;
	priority_t priority;

	if (fixed_layout) {
		layout = this->layout;
		priority = chromosome;
	}
	else {
		GeneEncoder::split(chromosome, priority, layout);
	}

	Schedule schedule, best_schedule;
	price_t start_price, best_price, price;

	bool improved = false;
	size_t lessons = 0, stall = 0;

	data_t data(task_count);

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

template<class CT>
void ListScheduleTraining<CT>::push(pool_t &pool, tid_t id) const
{
	const layout_t &layout = pool.layout;
	const priority_t &priority = pool.priority;
	list_schedule_t &local_pool = pool[layout[id]];

	list_schedule_t::iterator it;
	rank_t new_priority = priority[id];

	for (it = local_pool.begin(); it != local_pool.end(); it++)
		if (new_priority < priority[*it]) break;

	local_pool.insert(it, id);
}

template<class CT>
tid_t ListScheduleTraining<CT>::pull(pool_t &pool, pid_t pid) const
{
	list_schedule_t &local_pool = pool[pid];

	data_t *data = (data_t *)pool.data;

	tid_t id;

	list_schedule_t::iterator it = local_pool.begin();

	size_t size = local_pool.size();

	if (data->checkpoint(size)) {
		size_t direction = data->direction();
		for (size_t i = 0; i < direction; i++) it++;
	}

	id = *it;
	local_pool.erase(it);

	return id;
}
