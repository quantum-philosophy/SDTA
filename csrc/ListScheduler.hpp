#include "ListScheduler.h"

template<class CT>
void ListScheduleMutation<CT>::push(list_schedule_t &pool,
	const priority_t &original_priority, tid_t id, void *data) const
{
	list_schedule_t::iterator it;
	rank_t new_priority = original_priority[id];

	for (it = pool.begin(); it != pool.end(); it++)
		if (new_priority < original_priority[*it]) break;

	pool.insert(it, id);
}

template<class CT>
tid_t ListScheduleMutation<CT>::pull(list_schedule_t &pool,
	const priority_t &original_priority, void *data) const
{
	tid_t id;

	list_schedule_t::iterator it = pool.begin();

	if (Random::flip(current_rate)) {
		size_t choice = Random::number(pool.size());
		for (size_t i = 0; i < choice; i++) it++;
	}

	id = *it;
	pool.erase(it);

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
		GeneEncoder::split(chromosome, layout, priority);
	}

	Schedule schedule, best_schedule;
	price_t start_price, best_price, price;

	bool improved = false;
	size_t lessons = 0, stall = 0;

	data_t data(task_count);

	/* Collect all possible branches */
	schedule = process(layout, priority, (void *)&data);

	start_price = best_price = evaluation.process(schedule, true);

	while (stall < stall_lessons && lessons < max_lessons) {
		/* Choose one to inspect */
		if (!data.choose()) break;

		improved = false;

		/* Iterate through all branches */
		while (data.next()) {
			schedule = process(layout, priority, (void *)&data);
			price = evaluation.process(schedule, true);

			if (best_price.lifetime < price.lifetime) {
				/* We have found a better solution */
				best_price = price;
				best_schedule = schedule;

				improved = true;
			}
		}

		lessons++;

		if (improved) {
			stall = 0;

			const order_t &order = best_schedule.get_order();
			for (size_t i = 0; i < task_count; i++)
				priority[order[i]] = (rank_t)i;

			/* Reset and recollect */
			data.reset();
			(void)process(layout, priority, (void *)&data);
		}
		else stall++;
	}

	if (start_price.lifetime >= best_price.lifetime) return false;

	chromosome.set_schedule(best_schedule);
	chromosome.set_price(best_price);

	return true;
}

template<class CT>
void ListScheduleTraining<CT>::push(list_schedule_t &pool,
	const priority_t &original_priority, tid_t id, void *data) const
{
	list_schedule_t::iterator it;
	rank_t new_priority = original_priority[id];

	for (it = pool.begin(); it != pool.end(); it++)
		if (new_priority < original_priority[*it]) break;

	pool.insert(it, id);
}

template<class CT>
tid_t ListScheduleTraining<CT>::pull(list_schedule_t &pool,
	const priority_t &original_priority, void *_data) const
{
	data_t *data = (data_t *)_data;

	tid_t id;

	list_schedule_t::iterator it = pool.begin();

	size_t size = pool.size();

	if (data->checkpoint(size)) {
		size_t direction = data->direction();
		for (size_t i = 0; i < direction; i++) it++;
	}

	id = *it;
	pool.erase(it);

	return id;
}
