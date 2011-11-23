#include "Constrain.h"

/******************************************************************************/
/* Evolution                                                                  */
/******************************************************************************/

template<class CT, class PT, class ST>
ST &Evolution<CT, PT, ST>::solve(const layout_t &layout,
	const priority_t &priority)
{
	population_t population;

	populate(population, layout, priority);

	process(population);

	return stats;
}

template<class CT, class PT, class ST>
void Evolution<CT, PT, ST>::populate(population_t &population,
	const layout_t &layout, const priority_t &priority)
{
	size_t i;
	size_t create_count;
	bool fixed_layout = constrains.fixed_layout();

	population.clear();

	chromosome_t chromosome;

	GeneEncoder::encode(chromosome, priority);

	if (!fixed_layout)
		GeneEncoder::extend(chromosome, layout);

	Schedule schedule;

	const CreationTuning &creation_tuning = tuning.creation;

	/* Fill the first part with uniform chromosomes */
	create_count = creation_tuning.uniform_ratio * creation_tuning.population_size;

	/* NOTE: Everything is given, so work as
	 * the deterministic scheduler.
	 */
	schedule = this->scheduler.process(layout, priority);
	GeneEncoder::reorder(chromosome, schedule);

	if (!fixed_layout)
		GeneEncoder::reallocate(chromosome, schedule);

	evaluate(chromosome);

	for (i = 0; i < create_count; i++)
		population.push_back(chromosome);

	RandomGeneratorListScheduler scheduler(architecture, graph);

	/* Fill the second part with randomly generated chromosomes */
	create_count = creation_tuning.population_size - create_count;
	for (i = 0; i < create_count; i++) {
		chromosome.invalidate();
		if (fixed_layout) {
			/* Partially deterministic */
			schedule = scheduler.process(layout, priority_t());
		}
		else {
			/* Totally stochastic */
			schedule = scheduler.process(layout_t(), priority_t());
			GeneEncoder::reallocate(chromosome, schedule);
		}
		GeneEncoder::reorder(chromosome, schedule);
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
}
