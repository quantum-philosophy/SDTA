#include "SingleGLS.h"
#include "Graph.h"
#include "Lifetime.h"
#include "DynamicPower.h"

void SingleGLS::process(eoPop<chromosome_t> &population,
	eoContinue<chromosome_t> &continuator,
	eoTransform<chromosome_t> &transform)
{
	/* Evaluate */
	evaluate_t evaluate(this);

	/* Select */
	eoDetTournamentSelect<chromosome_t> select_one(tunning.tournament_size);
	eoSelectPerc<chromosome_t> select(select_one);

	/* Replace = Merge + Reduce */
	eslabElitismMerge<chromosome_t> merge(tunning.elitism_rate);
	eoLinearTruncate<chromosome_t> reduce;
	eoMergeReduce<chromosome_t> replace(merge, reduce);

	eslabEvolution<chromosome_t> ga(continuator, evaluate, select, transform, replace);
}

SingleGLS::fitness_t SingleGLS::evaluate_schedule(const schedule_t &schedule)
{
	fitness_t fitness;

	graph->assign_schedule(schedule);

	if (graph->duration > graph->deadline) {
		stats.deadline_misses++;
		fitness = std::numeric_limits<fitness_t>::min();

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
		size_t iterations = hotspot->solve(graph->architecture,
			dynamic_power, temperature, total_power);

		fitness = Lifetime::predict(temperature, sampling_interval);

		if (tunning.verbose) {
			std::cout << ".";
			std::cout.flush();
		}
	}

	return fitness;
}

template<class chromosome_t>
void eslabEvolution<chromosome_t>::operator()(eoPop<chromosome_t> &population)
{
	size_t population_size = population.size();;
	eoPop<chromosome_t> offspring;

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

template<class chromosome_t>
void eslabEvolution<chromosome_t>::evaluate(
	eoPop<chromosome_t> &population) const
{
	apply<chromosome_t>(evaluate_one, population);
}

template<class chromosome_t>
eslabElitismMerge<chromosome_t>::eslabElitismMerge(double _rate) : rate(_rate)
{
	if (rate < 0 || rate > 1)
		std::runtime_error("The elitism rate is invalid.");
}

template<class chromosome_t>
void eslabElitismMerge<chromosome_t>::operator()(
	const eoPop<chromosome_t> &population,
	eoPop<chromosome_t> &offspring)
{
	size_t count = rate * population.size();

	std::vector<const chromosome_t *> elite;
	population.nth_element(count, elite);

	for (size_t i = 0; i < count; i++)
		offspring.push_back(*elite[i]);
}

std::ostream &operator<< (std::ostream &o, const SingleGLS::tunning_t &tunning)
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Tunning:" << std::endl

		<< std::setprecision(0)
		<< "  Seed:                " << tunning.seed << std::endl

		/* Create */
		<< std::setprecision(2)
		<< "  Uniform ratio:       " << tunning.uniform_ratio << std::endl
		<< std::setprecision(0)
		<< "  Population size:     " << tunning.population_size << std::endl

		/* Continue */
		<< "  Minimum generations: " << tunning.min_generations << std::endl
		<< "  Maximum generations: " << tunning.max_generations << std::endl
		<< "  Stall generations:   " << tunning.stall_generations << std::endl

		/* Select */
		<< std::setprecision(2)
		<< "  Elitism rate:        " << tunning.elitism_rate << std::endl
		<< std::setprecision(0)
		<< "  Tournament size:     " << tunning.tournament_size << std::endl

		/* Crossover */
		<< std::setprecision(2)
		<< "  Crossover rate:      " << tunning.crossover_rate << std::endl
		<< std::setprecision(0)
		<< "  Crossover points:    " << tunning.crossover_points << std::endl

		/* Mutate */
		<< std::setprecision(2)
		<< "  Mutation rate:       " << tunning.mutation_rate << std::endl
		<< std::setprecision(0)
		<< "  Mutation points:     " << tunning.mutation_points << std::endl;
}

std::ostream &operator<< (std::ostream &o, const SingleGLS::stats_t &stats)
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Stats:" << std::endl

		<< std::setprecision(0)
		<< "  Generations:         " << stats.generations << std::endl
		<< "  Evaluations:         " << stats.evaluations << std::endl
		<< "  Cache hits:          " << stats.cache_hits << std::endl
		<< "  Deadline misses:     " << stats.deadline_misses << std::endl

		<< std::setiosflags(std::ios::scientific)
		<< std::setprecision(2)
		<< "  Best priority:       " << print_t<rank_t>(stats.priority) << std::endl
		<< std::setiosflags(std::ios::fixed)
		<< std::setprecision(0)
		<< "  Best schedule:       " << print_t<tid_t>(stats.schedule) << std::endl
		<< std::setprecision(2)
		<< "  Best fitness:        " << stats.fitness << std::endl;
}
