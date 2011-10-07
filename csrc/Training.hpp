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

	return true;
}
