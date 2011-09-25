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

	populate(population, layout, priority);

	process(population);

	return stats;
}

template<class CT, class PT, class ST>
void GenericEvolution<CT, PT, ST>::populate(population_t &population,
	const layout_t &layout, const priority_t &priority)
{
	size_t i;
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
	RandomGeneratorListScheduler scheduler(architecture, graph);
	create_count = tuning.population_size - create_count;
	for (i = 0; i < create_count; i++) {
		chromosome.set_schedule(scheduler.process(layout, priority_t()));
		evaluate(chromosome);
		GeneEncoder::order(chromosome);
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
