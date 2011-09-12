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
#include "Hotspot.h"
#include "ListScheduler.h"
#include "SingleObjectiveGLS.h"
#include "MultiObjectiveGLS.h"
#include "DynamicPower.h"
#include "Lifetime.h"

using namespace std;

#define __DELETE(some) \
	do { 						\
		if (some) delete some; 	\
		some = NULL;			\
	} while(0)

void perform(const string &system_config, const string &genetic_config,
	const string &floorplan_config, const string &thermal_config,
	stringstream &tuning_stream);

int main(int argc, char **argv)
{
	try {
		CommandLine arguments(argc, (const char **)argv);
		perform(arguments.system_config, arguments.genetic_config,
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

void optimize(const system_t &system, const GLSTuning &tuning,
	const string &floorplan_config, const string &thermal_config)
{
	clock_t begin, end;
	double elapsed;

	Graph *graph = NULL;
	Architecture *architecture = NULL;
	Hotspot *hotspot = NULL;
	GeneticListScheduler *scheduler = NULL;

	try {
		graph = new GraphBuilder(system.type, system.link);
		architecture = new ArchitectureBuilder(system.frequency,
			system.voltage, system.ngate, system.nc, system.ceff);

		size_t task_count = graph->size();
		size_t processor_count = architecture->size();

		mapping_t mapping = system.mapping;
		schedule_t schedule = system.schedule;
		priority_t priority = system.priority;
		double deadline = system.deadline;

		/* 1. Assign a mapping
		 *
		 * Generate an even mapping.
		 */
		if (mapping.empty()) {
			mapping = mapping_t(task_count);

			for (size_t i = 0; i < task_count; i++)
				mapping[i] = i % processor_count;
		}
		else if (tuning.verbose)
			cout << "Using external mapping." << endl;

		graph->assign_mapping(architecture, mapping);

		/* 2. Assign a schedule
		 *
		 * Generate a schedule based on the task mobility or
		 * the given priority.
		 */
		if (schedule.empty()) {
			if (priority.empty())
				schedule = ListScheduler::process(graph);
			else
				schedule = ListScheduler::process(graph, priority);
		}
		else if (tuning.verbose)
			cout << "Using external schedule." << endl;

		if (tuning.verbose && !priority.empty())
			cout << "Using external priority." << endl;

		graph->assign_schedule(schedule);

		/* 3. Assign a deadline
		 *
		 * Calculate if needed.
		 */
		if (deadline == 0) {
			deadline = tuning.deadline_ratio * graph->get_duration();
		}
		else if (tuning.verbose)
			cout << "Using external deadline." << endl;

		graph->assign_deadline(deadline);

		/* 4. Reorder the tasks if requested.
		 */
		if (tuning.reorder_tasks) {
			if (tuning.verbose)
				cout << "Reordering the tasks according to the first schedule." << endl;

			graph->reorder_tasks(schedule);
		}

		if (tuning.verbose)
			cout << graph << endl << architecture << endl;

		hotspot = new Hotspot(floorplan_config, thermal_config);

		price_t price = graph->evaluate(hotspot);

		cout << "Initial lifetime: "
			<< setiosflags(ios::fixed) << setprecision(2)
			<< price.lifetime << endl
			<< "Initial energy: "
			<< price.energy << endl;

		if (tuning.multiobjective)
			scheduler = new MultiObjectiveGLS(graph, hotspot, tuning);
		else
			scheduler = new SingleObjectiveGLS(graph, hotspot, tuning);

		begin = clock();

		GeneticListSchedulerStats &stats = scheduler->solve(priority);

		end = clock();
		elapsed = (double)(end - begin) / CLOCKS_PER_SEC;

		if (tuning.verbose)
			cout << endl << stats << endl;

		cout << "Improvement: " << setiosflags(ios::fixed) << setprecision(2);

		if (tuning.verbose)
			cout << "Time elapsed: " << elapsed << endl;

		if (!tuning.multiobjective) {
			SOGLSStats *sstats = (SOGLSStats *)&stats;

			cout
				<< (sstats->best_lifetime / price.lifetime - 1.0) * 100
				<< "% lifetime" << endl;
		}
		else {
			MOGLSStats *sstats = (MOGLSStats *)&stats;

			cout
				<< (sstats->best_lifetime.lifetime / price.lifetime - 1.0) * 100
				<< "% lifetime with "
				<< (sstats->best_lifetime.energy / price.energy - 1.0) * 100
				<< "% energy"
				<< endl;
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

void perform(const string &system_config, const string &genetic_config,
	const string &floorplan_config, const string &thermal_config,
	stringstream &tuning_stream)
{
	system_t system(system_config);

	GLSTuning tuning(genetic_config);
	tuning.update(tuning_stream);

	if (tuning.verbose)
		cout << tuning << endl;

	size_t repeat = tuning.repeat < 0 ? 1 : tuning.repeat;

	for (size_t i = 0; i < repeat; i++)
		optimize(system, tuning, floorplan_config, thermal_config);
}
