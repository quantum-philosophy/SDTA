#include "ListScheduler.h"

template<class CT>
void ListScheduleMutation<CT>::push(list_schedule_t &pool,
	const priority_t &original_priority, tid_t id) const
{
	list_schedule_t::iterator it;
	rank_t new_priority = original_priority[id];

	for (it = pool.begin(); it != pool.end(); it++)
		if (new_priority < original_priority[*it]) break;

	pool.insert(it, id);
}

template<class CT>
tid_t ListScheduleMutation<CT>::pull(list_schedule_t &pool,
	const priority_t &original_priority) const
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
