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

	double lifetime = chromosome.fitness();

	size_t lessons = 0, stall = 0;

	bool found;

	while (stall < stall_lessons && lessons < max_lessons) {
		data_t data;

		data.point = 0;
		data.switch_point = Random::number(task_count);
		data.trial = 0;
		data.done = false;

		found = false;

		do {
			Schedule schedule = process(layout, priority, (void *)&data);

			if (data.done) break;

			price_t price = evaluation.process(schedule, true);

			if (lifetime < price.lifetime) {
				/* We have found a better solution */
				lifetime = price.lifetime;

				chromosome.set_schedule(schedule);
				chromosome.set_price(price);

				found = true;
			}
		}
		while (true);

		if (data.trial == 0) {
			/* It means that there were no options */
			continue;
		}

		lessons++;

		if (found) {
			/* Everything is assigned, we need to reorder */
			GeneEncoder::order(chromosome);

			stall = 0;
		}
		else stall++;
	}
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

	if (data->point == data->switch_point) {
		size_t size = pool.size();

		if (data->trial < size) {
			for (size_t i = 0; i < data->trial; i++) it++;
			data->trial++;
		}
		else data->done = true;
	}

	data->point++;

	id = *it;
	pool.erase(it);

	return id;
}
