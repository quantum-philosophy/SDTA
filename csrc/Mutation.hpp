#include "Mutation.h"
#include "Neighborhood.h"

template<class CT>
bool UniformMutation<CT>::operator()(CT &chromosome)
{
	double rate = this->rate.get();

	size_t size = chromosome.size();
	bool changed = false;
	rank_t prev, next;

	for (size_t i = 0; i < size; i++) {
		const constrain_t &constrain = constrains[i];
		if (constrain.tight() || !Random::flip(rate)) continue;

		prev = chromosome[i];
		do next = constrain.random(); while (prev == next);

		chromosome[i] = next;
		changed = true;
	}

	return changed;
}

template<class CT>
bool PeerMutation<CT>::operator()(CT &chromosome)
{
	double rate = this->rate.get();

	bool changed = false;

	const Schedule &schedule = chromosome.get_schedule();
	size_t task_count = schedule.tasks();

	/* In order to prevent switching back */
	bit_string_t switched(task_count, false);

	for (tid_t id = 0; id < task_count; id++) {
		/* If we have already modified this gene or we are unlucky, go on */
		if (switched[id] || !Random::flip(rate)) continue;

		tid_t peer_id = Neighborhood::peer(id, schedule, constrains, switched);

		/* If cannot find, continue */
		if (peer_id == id) continue;

		/* Switch ranks between two peers */
		rank_t rank = chromosome[id];
		chromosome[id] = chromosome[peer_id];
		chromosome[peer_id] = rank;

		/* Remember which we have switched with */
		switched[peer_id] = true;
		switched[id] = true;

		changed = true;
	}

	return changed;
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

		GeneEncoder::split(chromosome, priority, layout);

		schedule = ListScheduler<MutationPool>::process(
			layout, priority, data);
	}

	chromosome.set_schedule(schedule);

	/* NOTE: We always say that nothing has changed, since
	 * the invalidation takes place in set_schedule. The purpose
	 * is to keep the already computed schedule valid,
	 * but the price becomes invalid.
	 */
	return false;
}
