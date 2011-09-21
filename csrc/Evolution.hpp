#include "Constrain.h"

/******************************************************************************/
/* eslabPop                                                                   */
/******************************************************************************/

template<class CT>
size_t eslabPop<CT>::unique() const
{
	const eslabPop<CT> &self = *this;
	size_t population_size = self.size();

	if (!population_size) return 0;

	size_t i, j, k, count;
	bool found;
	size_t chromosome_length = self[0].size();
	std::vector<bool> done(population_size, false);

	count = 0;
	for (i = 0; i < population_size; i++) {
		if (done[i]) continue;
		count++;

		for (j = i + 1; j < population_size; j++) {
			found = true;

			for (k = 0; k < chromosome_length; k++)
				if (self[i][k] != self[j][k]) {
					found = false;
					break;
				}

			if (found) done[j] = true;
		}
	}

	return count;
}

template<class CT>
double eslabPop<CT>::diversity() const
{
	const eslabPop<CT> &self = *this;
	size_t population_size = self.size();

	if (!population_size) return 0;

	double value = 0;
	size_t i, j, k, count, total = 0;
	size_t chromosome_length = self[0].size();

	for (i = 0; i < population_size - 1; i++)
		for (j = i + 1; j < population_size; j++) {
			for (count = 0, k = 0; k < chromosome_length; k++)
				if (self[i][k] != self[j][k]) count++;

			value += count;
			total++;
		}

	return value / (double)total / (double)chromosome_length;
}

/******************************************************************************/
/* GenericEvolutionStats                                                      */
/******************************************************************************/

template<class CT, class PT>
void GenericEvolutionStats<CT, PT>::watch(population_t &_population, bool _silent)
{
	population = &_population;
	silent = _silent;

	if (!silent)
		std::cout << "   0: ";

	generations = 0;
	evaluations = 0;
	deadline_misses = 0;

	crossover_rate = 0;
	mutation_rate = 0;

	reset();
}

template<class CT, class PT>
eoMonitor &GenericEvolutionStats<CT, PT>::operator()()
{
	if (!population)
		throw std::runtime_error("The population is not defined.");

	generations++;

	process();

	return *this;
}

template<class CT, class PT>
void GenericEvolutionStats<CT, PT>::display(std::ostream &o) const
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Stats:" << std::endl

		<< std::setprecision(0)
		<< "  Generations:     " << generations << std::endl
		<< "  Evaluations:     " << evaluations << std::endl
		<< "  Deadline misses: " << deadline_misses << std::endl;
}

/******************************************************************************/
/* GenericEvolution                                                           */
/******************************************************************************/

template<class CT, class PT, class ST>
ST &GenericEvolution<CT, PT, ST>::solve(const layout_t &layout,
	const priority_t &priority)
{
	/* Save the default layout in case the mapping part
	 * is not included, and we are varying only schedules.
	 */
	this->layout = layout;

	population_t population;

	/* Continue */
	eslabGenContinue<chromosome_t> gen_continue(tuning.max_generations);

	/* Monitor */
	eslabCheckPoint<chromosome_t> checkpoint(gen_continue);
	stats.watch(population, !tuning.verbose);
	checkpoint.add(stats);

	/* Create */
	populate(population, layout, priority);

	/* TODO: Get rid of... */ stats();

	/* Transform = Crossover + Mutate */
	eslabNPtsBitCrossover<chromosome_t, population_t> crossover(
		tuning.crossover_points, tuning.crossover_min_rate,
		tuning.crossover_scale, tuning.crossover_exponent, stats);
	eslabPeerMutation<chromosome_t, population_t> mutate(
		constrains, tuning.mutation_min_rate, tuning.mutation_scale,
		tuning.mutation_exponent, stats);
	eslabTransform<chromosome_t> transform(crossover, mutate);

	process(population, checkpoint, transform);

	return stats;
}

template<class CT, class PT, class ST>
void GenericEvolution<CT, PT, ST>::populate(population_t &population,
	const layout_t &layout, const priority_t &priority)
{
	size_t i, j;
	size_t create_count;

	population.clear();

	chromosome_t chromosome;

	GeneEncoder::encode(chromosome, priority);

	if (tuning.include_mapping)
		GeneEncoder::extend(chromosome, layout);

	evaluate(chromosome);

	/* Fill the first part with uniform chromosomes */
	create_count = tuning.uniform_ratio * tuning.population_size;
	for (i = 0; i < create_count; i++)
		population.push_back(chromosome);

	/* Fill the second part with randomly generated chromosomes */
	create_count = tuning.population_size - create_count;
	for (i = 0; i < create_count; i++) {
		for (j = 0; j < chromosome_length; j++)
			chromosome[j] = constrains[j].random();

		chromosome.invalidate();
		evaluate(chromosome);
		population.push_back(chromosome);
	}
}

/******************************************************************************/
/* eslabTransform                                                             */
/******************************************************************************/

template<class CT>
eslabTransform<CT>::eslabTransform(eoQuadOp<CT> &_crossover,
	eoMonOp<CT> &_mutate) : crossover(_crossover), mutate(_mutate) {}

template<class CT>
void eslabTransform<CT>::operator()(population_t &population)
{
	size_t i;
	size_t population_size = population.size();
	size_t crossover_count = population_size / 2;

	bool changed;
	std::vector<bool> changes(population_size, false);
	for (i = 0; i < crossover_count; i++) {
		changed = crossover(population[2 * i], population[2 * i + 1]);
		changes[2 * i] = changes[2 * i] || changed;
		changes[2 * i + 1] = changes[2 * i + 1] || changed;
	}

	for (i = 0; i < population_size; i++) {
		changed = mutate(population[i]);
		changes[i] = changes[i] || changed;
	}

	for (i = 0; i < population_size; i++)
		if (changes[i]) population[i].invalidate();
}

/******************************************************************************/
/* eslabCrossover                                                             */
/******************************************************************************/

template<class CT, class PT>
bool eslabNPtsBitCrossover<CT, PT>::perform(CT &one, CT &another, double rate)
{
	if (!Random::flip(rate)) return false;

	size_t i;
	size_t size = one.size();
	size_t select_points = points;

#ifndef SHALLOW_CHECK
	if (size != another.size())
		throw std::runtime_error("The chromosomes have different size.");
#endif

	std::vector<bool> turn_points(size, false);

	do {
		i = 1 + Random::number(size - 1);

		if (turn_points[i]) continue;
		else {
			turn_points[i] = true;
			select_points--;
		}
	}
	while (select_points);

	bool change = false;

	for (i = 1; i < size; i++) {
		if (turn_points[i]) change = !change;
		if (change) {
			rank_t tmp = one[i];
			one[i] = another[i];
			another[i] = tmp;
		}
	}

	return true;
}

template<class CT, class PT>
bool eslabPeerCrossover<CT, PT>::perform(CT &one, CT &another, double rate)
{
	if (!Random::flip(rate)) return false;

	size_t first, peer, peer_count, size = one.size();

#ifndef SHALLOW_CHECK
	if (size != another.size())
		throw std::runtime_error("The chromosomes have different size.");
#endif

	first = Random::number(size);
	peer_count = constrains[first].peers.size();

	rank_t rank;

	rank = one[first];
	one[first] = another[first];
	another[first] = rank;

	for (size_t i = 0; i < peer_count; i++) {
		peer = constrains[first].peers[i];
		rank = one[peer];
		one[peer] = another[peer];
		another[peer] = rank;
	}

	return true;
}

/******************************************************************************/
/* eslabMutation                                                              */
/******************************************************************************/

template<class CT, class PT>
bool eslabUniformRangeMutation<CT, PT>::perform(CT &chromosome, double rate)
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

template<class CT, class PT>
bool eslabPeerMutation<CT, PT>::perform(CT &chromosome, double rate)
{
	rank_t rank;
	int direction;
	bool changed = false;
	size_t local_size, pos, peer_id;

	const Schedule &schedule = chromosome.schedule();
	size_t task_count = schedule.tasks();

	/* In order to prevent switching back */
	bit_string_t switched(task_count, false);

	/* Since this mutation affects two genes */
	rate = rate / 2.0;

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
			if (constrain.has_peer(peer_id) && !switched[peer_id]) {
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
	}

	return changed;
}

/******************************************************************************/
/* eslabLearning                                                              */
/******************************************************************************/

template<class CT, class PT>
bool eslabLearning<CT, PT>::operator()(chromosome_t &chromosome)
{
	return false;
}

/******************************************************************************/
/* eslabEvolutionMonitor                                                      */
/******************************************************************************/

template<class CT>
eslabEvolutionMonitor<CT>::eslabEvolutionMonitor(population_t &_population,
	const std::string &filename) : population(_population)
{
	stream.open(filename.c_str());
	if (!stream.is_open())
		throw std::runtime_error("Cannot open the output file.");
}
