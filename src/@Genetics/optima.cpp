#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>
#include <vector>
#include <time.h>

#include "Graph.h"
#include "Architecture.h"
#include "Hotspot.h"
#include "ListScheduler.h"
#include "GeneticListScheduler.h"
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
	vector<vector<unsigned long int> > &nc, vector<vector<double> > &ceff);

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
	cout << "Usage: optima <system config> <genetic config> <floorplan> <thermal config>" << endl
		<< "  * <system config>    - a task graph with a set of PEs (architecture)" << endl
		<< "  * <genetic config>   - the configuration of the GLSA" << endl
		<< "  * <floorplan config> - the floorplan of the architecture" << endl
		<< "  * <thermal config>   - the configuration of Hotspot" << endl;
}

bool file_exist(const char *filename)
{
	return ifstream(filename).is_open();
}

void optimize(const char *system, const char *genetic, char *floorplan, char *thermal)
{
	const double deadline_ratio = 1.1;

	Graph *graph = NULL;
	Architecture *architecture = NULL;
	Hotspot *hotspot = NULL;
	GeneticListScheduler *scheduler = NULL;

	vector<unsigned int> type;
	vector<vector<bool> > link;
	vector<double> frequency;
	vector<double> voltage;
	vector<unsigned long int> ngate;
	vector<vector<unsigned long int> > nc;
	vector<vector<double> > ceff;

	double initial_lifetime, final_lifetime;

	parse_system(system, type, link, frequency, voltage, ngate, nc, ceff);

	try {
		GeneticListScheduler::tunning_t tunning(genetic);

		if (tunning.verbose)
			cout << tunning << endl;

		graph = new GraphBuilder(type, link);
		architecture = new ArchitectureBuilder(frequency, voltage, ngate, nc, ceff);

		if (tunning.verbose)
			cout << architecture << endl;

		size_t task_count = graph->size();
		size_t processor_count = architecture->size();

		mapping_t mapping(task_count);

		for (size_t i = 0; i < task_count; i++)
			mapping[i] = i % processor_count;

		graph->assign_mapping(architecture, mapping);

		hotspot = new Hotspot(floorplan, thermal);

		schedule_t schedule = ListScheduler::process(graph);
		graph->assign_schedule(schedule);

		graph->assign_deadline(deadline_ratio * graph->get_duration());

		if (tunning.verbose)
			cout << graph << endl;

		initial_lifetime = Lifetime::predict(graph, hotspot);

		cout << "Initial lifetime: "
			<< setiosflags(ios::fixed) << setprecision(2)
			<< initial_lifetime << endl;

		scheduler = new GeneticListScheduler(graph, hotspot, tunning);

		clock_t begin, end;
		double elapsed;

		begin = clock();
		schedule = scheduler->solve();
		end = clock();
		elapsed = (double)(end - begin) / CLOCKS_PER_SEC;

		GeneticListScheduler::stats_t stats = scheduler->get_stats();

		if (tunning.verbose)
			cout << endl << stats << endl;

		cout << "Improvement: "
			<< setiosflags(ios::fixed) << setprecision(2)
			<< (stats.best_fitness / initial_lifetime - 1.0) * 100 << " %" << endl;

		if (tunning.verbose)
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

void parse_system(const char *filename, vector<unsigned int> &type,
	vector<vector<bool> > &link, vector<double> &frequency,
	vector<double> &voltage, vector<unsigned long int> &ngate,
	vector<vector<unsigned long int> > &nc, vector<vector<double> > &ceff)
{
	char c;
	int i, j, tmp;
	string line, name;
	size_t rows, cols;

	ifstream file(filename);
	file.exceptions(fstream::failbit | fstream::badbit);

	if (!file.is_open())
		throw runtime_error("Cannot open the system file.");

	type.clear();
	link.clear();
	frequency.clear();
	voltage.clear();
	ngate.clear();
	nc.clear();
	ceff.clear();

	while (true) {
		try {

			getline(file, line);
		}
		catch (...) {
			break;
		}

		/* Skip empty lines and comments */
		if (line.empty() || line[0] == '#') continue;

		stringstream stream(line);
		stream.exceptions(ios::failbit | ios::badbit);

		stream >> c;
		if (c != '@')
			throw runtime_error("Cannot read the start sign for the object.");

		stream >> name;

		stream >> c;
		if (c != '(')
			throw runtime_error("Cannot read the dimensions.");

		stream >> rows;
		if (rows <= 0)
			throw runtime_error("Cannot read the dimensions.");

		stream >> c;
		if (c != 'x')
			throw runtime_error("Cannot read the dimensions.");

		stream >> cols;
		if (cols <= 0)
			throw runtime_error("Cannot read the dimensions.");

		stream >> c;
		if (c != ')')
			throw runtime_error("Cannot read the dimensions.");

		if (name.compare("type") == 0) {
			if (rows != 1)
				throw runtime_error("The type should be a vector.");

			type.resize(cols);
			for (i = 0; i < cols; i++)
				file >> type[i];
		}
		else if (name.compare("link") == 0) {
			if (rows != cols)
				throw runtime_error("The link should be a square matrix.");

			link.resize(rows);
			for (i = 0; i < rows; i++) {
				link[i].resize(cols);
				for (j = 0; j < cols; j++) {
					file >> tmp;
					link[i][j] = (bool)tmp;
				}
			}
		}
		else if (name.compare("frequency") == 0) {
			if (rows != 1)
				throw runtime_error("The frequency should be a vector.");

			frequency.resize(cols);
			for (i = 0; i < cols; i++)
				file >> frequency[i];
		}
		else if (name.compare("voltage") == 0) {
			if (rows != 1)
				throw runtime_error("The voltage should be a vector.");

			voltage.resize(cols);
			for (i = 0; i < cols; i++)
				file >> voltage[i];
		}
		else if (name.compare("ngate") == 0) {
			if (rows != 1)
				throw runtime_error("The ngate should be a vector.");

			ngate.resize(cols);
			for (i = 0; i < cols; i++)
				file >> ngate[i];
		}
		else if (name.compare("nc") == 0) {
			nc.resize(cols);
			for (i = 0; i < cols; i++)
				nc[i].resize(rows);
			for (i = 0; i < rows; i++)
				for (j = 0; j < cols; j++)
					file >> nc[j][i];
		}
		else if (name.compare("ceff") == 0) {
			ceff.resize(cols);
			for (i = 0; i < cols; i++)
				ceff[i].resize(rows);
			for (i = 0; i < rows; i++)
				for (j = 0; j < cols; j++)
					file >> ceff[j][i];
		}
		else
			throw runtime_error("An unknown variable.");
	}

	size_t task_count = type.size();

	if (!task_count)
		throw runtime_error("There are no tasks.");

	if (task_count != link.size() || task_count != link[0].size())
		throw runtime_error("The size of the link matrix is wrong.");

	size_t processor_count = frequency.size();

	if (!processor_count)
		throw runtime_error("There are no frequencies.");

	if (processor_count != voltage.size())
		throw runtime_error("The size of the voltage vector is wrong.");

	if (processor_count != ngate.size())
		throw runtime_error("The size of the ngate vector is wrong.");

	size_t type_count;

	if (processor_count != nc.size() || (type_count = nc[0].size()) == 0)
		throw runtime_error("The size of the nc matrix is wrong.");

	if (processor_count != ceff.size() || type_count != ceff[0].size())
		throw runtime_error("The size of the ceff matrix is wrong.");
}
