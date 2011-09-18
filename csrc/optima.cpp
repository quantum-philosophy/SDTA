#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <time.h>

#include "CommandLine.h"
#include "Graph.h"
#include "Architecture.h"
#include "Schedule.h"
#include "Layout.h"
#include "Priority.h"
#include "ListScheduler.h"
#include "SOEvolution.h"
#include "MOEvolution.h"
#include "DynamicPower.h"
#include "Lifetime.h"
#include "Hotspot.h"

using namespace std;

#define __DELETE(some) 			\
	do { 						\
		if (some) delete some; 	\
		some = NULL;			\
	} while(0)

void optimize(const string &system_config, const string &genetic_config,
	const string &floorplan_config, const string &thermal_config,
	stringstream &tuning_stream);

int main(int argc, char **argv)
{
	try {
		CommandLine arguments(argc, (const char **)argv);
		optimize(arguments.system_config, arguments.genetic_config,
			arguments.floorplan_config, arguments.thermal_config,
			arguments.tuning_stream);
	}
	catch (exception &e) {
		cerr << e.what() << endl;
		CommandLine::usage();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void backup(const string &iname, int index)
{
	stringstream oname;
	oname << iname << "_" << index;
	ifstream is(iname.c_str(), ios::binary);
	ofstream os(oname.str().c_str(), ios::binary);
	os << is.rdbuf();
}

void optimize(const string &system_config, const string &genetic_config,
	const string &floorplan_config, const string &thermal_config,
	stringstream &tuning_stream)
{
	system_t system(system_config);

	EvolutionTuning tuning(genetic_config);
	tuning.update(tuning_stream);

	if (tuning.seed < 0) {
		tuning.seed = time(NULL);
		if (tuning.verbose)
			std::cout << "Chosen seed: " << tuning.seed << std::endl;
	}
	Random::seed(tuning.seed);

	if (tuning.verbose)
		cout << tuning << endl;

	size_t repeat = tuning.repeat < 0 ? 1 : tuning.repeat;

	Graph *graph = NULL;
	Architecture *architecture = NULL;
	Hotspot *hotspot = NULL;
	Evolution *scheduler = NULL;

	try {
		graph = new GraphBuilder(system.type, system.link);
		architecture = new ArchitectureBuilder(system.frequency,
			system.voltage, system.ngate, system.nc, system.ceff);

		/* In order to assign a reasonable deadline and perform
		 * initial measurements to compare with, we:
		 * - assign a dummy mapping,
		 * - compute a mobility-based priority,
		 * - and obtain a schedule with help of List Scheduler.
		 */

		mapping_t mapping = system.mapping;
		priority_t priority = system.priority;
		double deadline = system.deadline;

		/* 1. Create and assign an even mapping.
		 *
		 */
		if (mapping.empty())
			mapping = Layout::calculate(*architecture, *graph);
		else if (tuning.verbose)
			cout << "Using external mapping." << endl;

		/* 2. Calculate a priority vector based on the task mobility.
		 *
		 */
		if (priority.empty())
			priority = Priority::calculate(*architecture, *graph, mapping);
		else if (tuning.verbose)
			cout << "Using external priority." << endl;

		/* 3. Compute a schedule.
		 *
		 */
		Schedule schedule = ListScheduler::process(
			*architecture, *graph, mapping, priority);

		/* 4. Assign a deadline.
		 *
		 */
		if (deadline == 0)
			deadline = tuning.deadline_ratio * schedule.get_duration();
		else if (tuning.verbose)
			cout << "Using external deadline." << endl;

		/* 5. Reorder the tasks if requested.
		 *
		 */
		if (tuning.reorder_tasks)
			throw std::runtime_error("Not implemented yet.");

		if (tuning.verbose) {
			cout << graph << endl << architecture << endl
				<< "Start mapping: " << print_t<pid_t>(mapping) << endl
				<< "Start priority: " << print_t<rank_t>(priority) << endl
				<< "Start schedule:" << endl << schedule << endl;

			const constrains_t constrains =
				Constrain::calculate(*architecture, *graph);

			size_t out = 0, task_count = graph->size();
			for (size_t i = 0; i < task_count; i++)
				if (priority[i] < constrains[i].min ||
					priority[i] > constrains[i].max) out++;

			if (out > 0)
				cout << "Out of range priorities: " << out << endl;

			cout << endl;
		}

		hotspot = new Hotspot(floorplan_config, thermal_config);

		/* 6. Obtain the initial measurements to compare with.
		 *
		 */
		price_t price = schedule.evaluate(*hotspot);

		if (tuning.verbose)
			cout << "Initial lifetime: "
				<< setiosflags(ios::fixed) << setprecision(2)
				<< price.lifetime << endl
				<< "Initial energy: "
				<< price.energy << endl;

		for (size_t i = 0; i < repeat; i++) {
			if (tuning.multiobjective) {
				scheduler = new MOEvolution(*architecture,
					*graph, *hotspot, tuning);
			}
			else {
				scheduler = new SOEvolution(*architecture,
					*graph, *hotspot, tuning);
			}

			clock_t begin = clock();

			EvolutionStats &stats = scheduler->solve(mapping, priority);

			clock_t end = clock();
			double elapsed = (double)(end - begin) / CLOCKS_PER_SEC;

			cout << endl << stats << endl;

			cout << "Improvement: " << setiosflags(ios::fixed) << setprecision(2);

			if (!tuning.multiobjective) {
				SOEvolutionStats *sstats = (SOEvolutionStats *)&stats;

				cout
					<< (sstats->best_lifetime / price.lifetime - 1.0) * 100
					<< "% lifetime" << endl;
			}
			else {
				MOEvolutionStats *sstats = (MOEvolutionStats *)&stats;

				cout
					<< (sstats->best_lifetime.lifetime / price.lifetime - 1.0) * 100
					<< "% lifetime with "
					<< (sstats->best_lifetime.energy / price.energy - 1.0) * 100
					<< "% energy"
					<< endl;
			}

			if (tuning.verbose)
				cout << "Time elapsed: " << elapsed << endl << endl;

			/* Make a back copy of the dump file */
			if (!tuning.dump_evolution.empty() && repeat > 1)
				backup(tuning.dump_evolution, i);

			__DELETE(scheduler);
		}
	}
	catch (exception &e) {
		__DELETE(graph);
		__DELETE(architecture);
		__DELETE(hotspot);
		__DELETE(scheduler);
		throw;
	}

	__DELETE(graph);
	__DELETE(architecture);
	__DELETE(hotspot);
	__DELETE(scheduler);
}
