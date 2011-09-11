#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <time.h>

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

#define __MX_FREE(some) \
	do { 						\
		if (some) mxFree(some); \
		some = NULL;			\
	} while(0)

void usage();
bool file_exist(const char *filename);
void optimize(const char *system, const char *genetic, char *floorplan, char *thermal);
void parse_system(const char *filename, vector<unsigned int> &type,
	vector<vector<bool> > &link, vector<double> &frequency,
	vector<double> &voltage, vector<unsigned long int> &ngate,
	vector<vector<unsigned long int> > &nc, vector<vector<double> > &ceff,
	mapping_t &mapping, schedule_t &schedule, priority_t &priority);

int main(int argc, char **argv)
{
	char *system, *genetic, *floorplan, *thermal;

	try {
		if (argc != 5) {
			usage();
			throw runtime_error("Wrong number of arguments.");
		}

		if (!file_exist(system = argv[1]))
			throw runtime_error("The system configuration file does not exist.");

		if (!file_exist(genetic = argv[2]))
			throw runtime_error("The genetic configuration file does not exist.");

		if (!file_exist(floorplan = argv[3]))
			throw runtime_error("The floorplan configuration file does not exist.");

		if (!file_exist(thermal = argv[4]))
			throw runtime_error("The thermal configuration file does not exist.");

		optimize(system, genetic, floorplan, thermal);
	}
	catch (exception &e) {
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void usage()
{
	cout << "Usage: optima <system config> <genetic config> <floorplan config> <thermal config>" << endl
		<< "  * <system config>    - a task graph with a set of PEs (architecture)" << endl
		<< "  * <genetic config>   - the configuration of the GLSA" << endl
		<< "  * <floorplan config> - the floorplan of the architecture" << endl
		<< "  * <thermal config>   - the configuration of Hotspot" << endl;
}

bool file_exist(const char *filename)
{
	return ifstream(filename).is_open();
}

void optimize(const char *system_config, const char *genetic_config,
	char *floorplan_config, char *thermal_config)
{
	const double deadline_ratio = 1.1;

	Graph *graph = NULL;
	Architecture *architecture = NULL;
	Hotspot *hotspot = NULL;
	GeneticListScheduler *scheduler = NULL;

	try {
		system_t system(system_config);

		GLSTuning tuning(genetic_config);

		graph = new GraphBuilder(system.type, system.link);
		architecture = new ArchitectureBuilder(system.frequency,
			system.voltage, system.ngate, system.nc, system.ceff);

		size_t task_count = graph->size();
		size_t processor_count = architecture->size();

		mapping_t mapping = system.mapping;

		if (mapping.empty()) {
			mapping = mapping_t(task_count);

			for (size_t i = 0; i < task_count; i++)
				mapping[i] = i % processor_count;
		}
		else if (tuning.verbose)
			cout << "Using external mapping." << endl;

		graph->assign_mapping(architecture, mapping);

		schedule_t schedule = system.schedule;

		if (schedule.empty())
			schedule = ListScheduler::process(graph);
		else if (tuning.verbose)
			cout << "Using external schedule." << endl;

		if (tuning.reorder_tasks) {
			if (tuning.verbose)
				cout << "Reordering the tasks according to the first schedule." << endl;

			graph->reorder_tasks(schedule);
			for (size_t i = 0; i < task_count; i++)
				schedule[i] = i;
		}

		graph->assign_schedule(schedule);

		double deadline = system.deadline;

		if (deadline == 0) {
			deadline = deadline_ratio * graph->get_duration();
		}
		else if (tuning.verbose)
			cout << "Using external deadline." << endl;

		graph->assign_deadline(deadline);

		if (tuning.verbose)
			cout << tuning << endl << graph << endl << architecture << endl;

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

		if (tuning.verbose && !system.priority.empty())
			cout << "Using external priority." << endl;

		clock_t begin, end;
		double elapsed;

		begin = clock();

		GeneticListSchedulerStats &stats = scheduler->solve(system.priority);

		end = clock();
		elapsed = (double)(end - begin) / CLOCKS_PER_SEC;

		if (tuning.verbose)
			cout << endl << stats << endl;

		if (!tuning.multiobjective) {
			SOGLSStats *sstats = (SOGLSStats *)&stats;

			cout << "Improvement: "
				<< setiosflags(ios::fixed) << setprecision(2)
				<< (sstats->best_lifetime / price.lifetime - 1.0) * 100 << "%" << endl;
		}
		else {
			MOGLSStats *sstats = (MOGLSStats *)&stats;
		}

		if (tuning.verbose)
			cout << "Time elapsed: " << elapsed << endl;
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
