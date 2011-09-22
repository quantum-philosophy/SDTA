#include "Mutation.h"

template<class CT>
bool eslabUniformRangeMutation<CT>::perform(CT &chromosome, double rate)
{
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
bool eslabPeerMutation<CT>::perform(CT &chromosome, double rate)
{
	rank_t rank;
	int direction;
	bool changed = false;
	size_t local_size, pos, peer_id;

	const Schedule &schedule = chromosome.schedule();
	size_t task_count = schedule.tasks();

	/* In order to prevent switching back */
	bit_string_t switched(task_count, false);

	for (tid_t id = 0; id < task_count; id++) {
		/* If we have already modified this gene or we are unlucky, go on */
		if (switched[id] || !Random::flip(rate)) continue;

		pid_t pid = schedule.map(id);
		const LocalSchedule &local_schedule = schedule[pid];
		local_size = local_schedule.size();

		/* If the task is the only one on the core, skip */
		if (local_size == 1) continue;

		/* Find the position of the task in the local schedule */
		for (pos = 0; pos < local_size; pos++)
			if (local_schedule[pos].id == id) break;

#ifndef SHALLOW_CHECK
		if (pos == local_size)
			throw std::runtime_error("Cannot find the task.");
#endif

		/* Choose the direction */
		if (pos == 0) direction = +1;
		else if (pos == local_size - 1) direction = -1;
		else direction = Random::flip(0.5) ? +1 : -1;

		const constrain_t &constrain = constrains[id];

		/* Looking for a peer */
		for (pos += direction; pos >= 0 && pos < local_size; pos += direction) {
			peer_id = local_schedule[pos].id;

			if (switched[peer_id] || !constrain.has_peer(peer_id)) continue;

			/* Switch ranks between two peers */
			rank = chromosome[id];
			chromosome[id] = chromosome[peer_id];
			chromosome[peer_id] = rank;

			/* Remember which we have switched with */
			switched[peer_id] = true;
			switched[id] = true;

			changed = true;
			break;
		}
	}

	return changed;
}
