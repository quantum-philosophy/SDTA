#include "SingleGLS.h"

void SingleGLS::evolve(population_t &population,
	eoContinue &continuator, eoTransform &transform)
{
	/* Evaluate */
	eslabEvaluate evaluate(this);

	/* Select */
	eoDetTournamentSelect<chromosome_t> select_one(tunning.tournament_size);
	eoSelectPerc<chromosome_t> select(select_one);

	/* Replace = Merge + Reduce */
	eslabElitismMerge merge(tunning.elitism_rate);
	eoLinearTruncate<chromosome_t> reduce;
	eoMergeReduce<chromosome_t> replace(merge, reduce);

	eslabEvolution ga(continuator, evaluate, select, transform, replace);
}

double SingleGLS::evaluate_schedule(const schedule_t &schedule)
{
	double fitness;

	graph->assign_schedule(schedule);

	if (graph->duration > graph->deadline) {
		stats.deadline_misses++;
		fitness = std::numeric_limits<double>::min();

		if (tunning.verbose) {
			std::cout << "!";
			std::cout.flush();
		}
	}
	else {
		matrix_t dynamic_power, temperature, total_power;

		/* The graph is rescheduled now, obtain the dynamic power profile */
		DynamicPower::compute(graph, sampling_interval, dynamic_power);

		/* Now, we can get the temperature profile, and the total power profile
		 * including the leakage part.
		 */
		unsigned int iterations = hotspot->solve(graph->architecture,
			dynamic_power, temperature, total_power);

		fitness = Lifetime::predict(temperature, sampling_interval);

		if (tunning.verbose) {
			std::cout << ".";
			std::cout.flush();
		}
	}

	return fitness;
}

/******************************************************************************/

void eslabEvolution::operator()(population_t &population)
{
	size_t population_size = population.size();;
	population_t offspring;

	/* Initial evaluation */
	evaluate(population);

	do {
		/* Select */
		select(population, offspring);

		/* Transform = Crossover + Mutate */
		transform(offspring);

		/* Evaluate newcomers */
		evaluate(offspring);

		/* Evolve */
		replace(population, offspring);

		if (population.size() != population_size)
			throw std::runtime_error("The size of the population changes.");
	}
	while (continuator(population));
}

void eslabEvolution::evaluate(population_t &population) const
{
	apply<chromosome_t>(evaluate_one, population);
}

/******************************************************************************/

eslabElitismMerge::eslabElitismMerge(double _rate) : rate(_rate)
{
	if (rate < 0 || rate > 1)
		std::runtime_error("The elitism rate is invalid.");
}

void eslabElitismMerge::operator()(const population_t &population,
	population_t &offspring)
{
	size_t count = rate * population.size();

	std::vector<const chromosome_t *> elite;
	population.nth_element(count, elite);

	for (size_t i = 0; i < count; i++)
		offspring.push_back(*elite[i]);
}
