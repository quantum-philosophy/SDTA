#include "Training.h"

template<class CT>
bool Training<CT>::peer(CT &chromosome, double rate)
{
	if (!Random::flip(rate)) return false;

	evaluate(chromosome);

	const Schedule &schedule = chromosome.schedule();
	size_t task_count = schedule.tasks();

	CT current_one = chromosome, best_one = chromosome;
	typename CT::fitness_t current_fitness, best_fitness = chromosome.fitness();

	/* In order to prevent switching back */
	bit_string_t switched(task_count, false);

	size_t lessons = 0, stall = 0;

	while (stall < tuning.stall_lessons && lessons < tuning.max_lessons) {
		tid_t id = Random::number(task_count);
		pid_t pid = schedule.map(id);

		const LocalSchedule &local_schedule = schedule[pid];
		size_t local_size = local_schedule.size();

		/* We want to teach something... */
		if (local_size == 1) continue;

		size_t pos = 0;

		for (; pos < local_size; pos++)
			if (local_schedule[pos].id == id) break;

#ifndef SHALLOW_CHECK
		if (pos == local_size)
			throw std::runtime_error("Cannot find the task.");
#endif

		/* Choose the direction */
		int direction;

		if (pos == 0) direction = +1;
		else if (pos == local_size - 1) direction = -1;
		else direction = Random::flip(0.5) ? +1 : -1;

		const constrain_t &constrain = constrains[id];

		/* Looking for a peer */
		bool found = false, improved = false;

		rank_t rank = current_one[id];

		for (pos += direction; pos >= 0 && pos < local_size; pos += direction) {
			size_t peer_id = local_schedule[pos].id;

			if (!constrain.has_peer(peer_id)) continue;
			found = true;

			/* Switch ranks between two peers */
			current_one[id] = current_one[peer_id];
			current_one[peer_id] = rank;

			current_one.invalidate();
			evaluate(current_one);
			current_fitness = current_one.fitness();

			if (best_fitness < current_fitness) {
				best_fitness = current_fitness;
				best_one = current_one;
				improved = true;
			}

			/* Return back */
			current_one[peer_id] = current_one[id];
			current_one[id] = rank;
		}

		if (found) {
			lessons++;

			if (improved) stall = 0;
			else stall++;
		}
	}

	if (best_fitness > chromosome.fitness()) {
		/* The lesson has learnt */
		chromosome = best_one;
		return true;
	}

	return false;
}
