#ifndef __NEIGHBORHOOD_H__
#define __NEIGHBORHOOD_H__

class Neighborhood
{
	public:

	/* Find all peers... */
	static void peers(tid_t id,
		const Schedule &schedule, const constrains_t &constrains,
		std::list<tid_t> &left, std::list<tid_t> &right,
		const bit_string_t &exclude = bit_string_t())
	{
		pid_t pid = schedule.map(id);

		const LocalSchedule &local_schedule = schedule[pid];
		size_t local_size = local_schedule.size();

		bool check = !exclude.empty();

		const constrain_t &constrain = constrains[id];

		bool before = true;

		left.clear();
		right.clear();

		for (size_t i = 0; i < local_size; i++) {
			tid_t peer_id = local_schedule[i].id;

			if (peer_id == id) {
				before = false;
				continue;
			}

			if ((check && !exclude[peer_id]) ||
				!constrain.has_peer(peer_id)) continue;

			if (before) left.push_front(peer_id);
			else right.push_back(peer_id);
		}
	}

	/* Find one peer... */
	static tid_t peer(tid_t id,
		const Schedule &schedule, const constrains_t &constrains,
		const bit_string_t &exclude = bit_string_t())
	{
		std::list<tid_t> left;
		std::list<tid_t> right;

		peers(id, schedule, constrains, left, right, exclude);

		size_t left_count = left.size();
		size_t right_count = right.size();

		bool go_left = true;

		if (left_count && right_count) {
			/* Choose a random direction */
			if (Random::flip(0.5)) go_left = false;
		}
		else if (right_count) {
			/* Do not have another choice */
			go_left = false;
		}
		else if (!left_count) {
			/* Not found */
			return id;
		}

		size_t steps;
		std::list<tid_t>::iterator it;

		if (go_left) {
			it = left.begin();
			steps = Random::number(left_count);
		}
		else {
			it = right.begin();
			steps = Random::number(right_count);
		}

		for (size_t i = 0; i < steps; i++) it++;

		return *it;
	}
};

#endif
