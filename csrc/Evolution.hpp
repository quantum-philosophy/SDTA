#include "Constrain.h"

/******************************************************************************/
/* Evolution                                                                  */
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
	Continuation<chromosome_t> continuation(tuning.continuation);

	/* Monitor */
	eslabCheckPoint<chromosome_t> checkpoint(continuation);
	stats.watch(population, !tuning.verbose);
	checkpoint.add(stats);

	/* Create */
	populate(population, layout, priority);

	process(population, checkpoint);

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
/* Monitoring                                                                 */
/******************************************************************************/

template<class CT>
eslabEvolutionMonitor<CT>::eslabEvolutionMonitor(population_t &_population,
	const std::string &filename) : population(_population)
{
	stream.open(filename.c_str());
	if (!stream.is_open())
		throw std::runtime_error("Cannot open the output file.");
}
