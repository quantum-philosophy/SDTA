#include "Training.h"

template<class CT>
bool PeerTraining<CT>::operator()(CT &chromosome)
{
	double rate = this->rate.get();

	if (!Random::flip(rate)) return false;

	evaluate(chromosome);

	const Schedule &schedule = chromosome.get_schedule();
	size_t task_count = schedule.tasks();

	CT current = chromosome;

	typename CT::fitness_t fitness, best_fitness = chromosome.fitness();

	size_t lessons = 0, stall = 0;

	while (stall < stall_lessons && lessons < max_lessons) {
		tid_t id = Random::number(task_count);
		tid_t peer_id = Neighborhood::peer(id, schedule, constrains);

		/* If cannot find, continue */
		if (peer_id == id) continue;

		lessons++;

		/* Switch ranks between two peers */
		rank_t rank = current[id];
		current[id] = current[peer_id];
		current[peer_id] = rank;
		current.set_invalid();

		evaluate(current);
		fitness = current.fitness();

		if (best_fitness < fitness) {
			best_fitness = fitness;

			stall = 0;
		}
		else {
			/* Return back */
			current[peer_id] = current[id];
			current[id] = rank;
			current.set_invalid();

			stall++;
		}
	}

	/* Has the lesson been learnt? */
	if (best_fitness > chromosome.fitness()) {
		chromosome = current;
		return true;
	}

	return false;
}
