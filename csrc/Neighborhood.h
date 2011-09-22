#ifndef __NEIGHBORHOOD_H__
#define __NEIGHBORHOOD_H__

class Neighborhood
{
	public:

	/* Find a peer... */
	static tid_t peer(
		/* ... of */
		tid_t id,
		/* ... in a schedule */
		const Schedule &schedule,
		/* ... with constrains */
		const constrains_t &constrains,
		/* ... excluding bad guys */
		const bit_string_t &exclude = bit_string_t())
	{
		size_t pos;
		int direction;
		tid_t peer_id;

		pid_t pid = schedule.map(id);

		const LocalSchedule &local_schedule = schedule[pid];
		size_t local_size = local_schedule.size();

		bool check = !exclude.empty();

		/* If the task is the only one on the core, skip */
		if (local_size == 1) return id;

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

		for (pos += direction; pos >= 0 && pos < local_size; pos += direction) {
			peer_id = local_schedule[pos].id;

			if ((check && !exclude[peer_id]) ||
				!constrain.has_peer(peer_id)) continue;

			return peer_id;
		}

		return id;
	}
};

#endif
