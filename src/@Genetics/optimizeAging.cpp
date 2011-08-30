#include <mex.h>

#include <vector>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include "Graph.h"
#include "Architecture.h"
#include "Hotspot.h"
#include "ListScheduler.h"
#include "GeneticListScheduler.h"
#include "DynamicPower.h"
#include "Lifetime.h"

#include <string.h>

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

void parse_hotspot_config(const mxArray *structure,
	char **floorplan, char **config, str_pair **table, size_t *tsize)
{
	size_t i, j, field_count;

	if (mxGetNumberOfElements(structure) != 1 ||
		(field_count = mxGetNumberOfFields(structure)) < 2)
		throw runtime_error("A wrong configuration structure of Hotspot.");

	mxArray *value;
	const char *name;
	char *temp;

	*floorplan = NULL;
	*config = NULL;

	*table = NULL;
	*tsize = 0;

	try {
		/* Excluding the floorplan and config */
		*table = (str_pair *)mxCalloc(field_count - 2, sizeof(str_pair));

		/* ATTENTION: Unsafe strcpy and sprintf, with hope for the best... */
		for (i = 0; i < field_count; i++) {
			value = mxGetFieldByNumber(structure, 0, i);
			name = mxGetFieldNameByNumber(structure, i);

			if (strcmp(name, "floorplan") == 0) {
				if (!mxIsChar(value) ||
					*floorplan || !(*floorplan = mxArrayToString(value)))
					throw runtime_error("The floorplan field is bad.");

				continue;
			}

			if (strcmp(name, "config") == 0) {
				if (!mxIsChar(value) ||
					*config || !(*config = mxArrayToString(value)))
					throw runtime_error("The config field is bad.");

				continue;
			}

			strcpy((*table)[*tsize].name, name);

			if (mxIsChar(value)) {
				temp = mxArrayToString(value);
				strcpy((*table)[*tsize].value, temp);
				__MX_FREE(temp);
			}
			else if (mxIsNumeric(value)) {
				sprintf((*table)[*tsize].value, "%f", mxGetScalar(value));
			}
			else throw runtime_error("An unknown type of the field.");

			(*tsize)++;
		}

		if (!*floorplan)
			throw runtime_error("The floorplan is not found.");

		if (!*config)
			throw runtime_error("The config is not found.");
	}
	catch (...) {
		__MX_FREE(*floorplan);
		__MX_FREE(*config);
		__MX_FREE(*table);
		throw;
	}
}

void parse_system_config(const mxArray *structure,
	vector<unsigned int> &type, vector<vector<bool> > &link,
	vector<double> &frequency, vector<double> &voltage,
	vector<unsigned long int> &ngate, vector<vector<unsigned long int> > &nc,
	vector<vector<double> > &ceff)
{
	size_t field_count;

	if (mxGetNumberOfElements(structure) != 1 ||
		(field_count = mxGetNumberOfFields(structure)) < 7)
		throw runtime_error("A wrong configuration structure of the system.");

	type.clear();
	link.clear();
	frequency.clear();
	voltage.clear();
	ngate.clear();
	nc.clear();
	ceff.clear();

	mxArray *value;
	const char *name;
	double *data;
	size_t field, i, j, k, task_count, processor_count, type_count;

	for (field = 0; field < field_count; field++) {
		value = mxGetFieldByNumber(structure, 0, field);
		name = mxGetFieldNameByNumber(structure, field);

		if (strcmp(name, "type") == 0) {
			task_count = mxGetM(value) * mxGetN(value);

			if (!(data = mxGetPr(value)))
				throw runtime_error("The type vector is bad.");

			for (i = 0; i < task_count; i++)
				type.push_back((unsigned int)data[i]);
		}
		else if (strcmp(name, "link") == 0) {
			task_count = mxGetM(value);

			if (!(data = mxGetPr(value)) ||
				task_count != mxGetN(value))
				throw runtime_error("The link matrix is bad.");

			k = 0;
			link.resize(task_count);

			/* NOTE: We are moving column my column here */
			for (i = 0; i < task_count; i++)
				for (j = 0; j < task_count; j++, k++)
					link[j].push_back((bool)data[k]);
		}
		else if (strcmp(name, "frequency") == 0) {
			processor_count = mxGetM(value) * mxGetN(value);

			if (!(data = mxGetPr(value)))
				throw runtime_error("The frequency vector is bad.");

			for (i = 0; i < processor_count; i++)
				frequency.push_back(data[i]);
		}
		else if (strcmp(name, "voltage") == 0) {
			processor_count = mxGetM(value) * mxGetN(value);

			if (!(data = mxGetPr(value)))
				throw runtime_error("The voltage vector is bad.");

			for (i = 0; i < processor_count; i++)
				voltage.push_back(data[i]);
		}
		else if (strcmp(name, "ngate") == 0) {
			processor_count = mxGetM(value) * mxGetN(value);

			if (!(data = mxGetPr(value)))
				throw runtime_error("The NGATE vector is bad.");

			for (i = 0; i < processor_count; i++)
				ngate.push_back((unsigned long int)data[i]);
		}
		else if (strcmp(name, "nc") == 0) {
			type_count = mxGetM(value);
			processor_count = mxGetN(value);

			if (!(data = mxGetPr(value)))
				throw runtime_error("The NC vector is bad.");

			k = 0;
			nc.resize(processor_count);
			/* NOTE: We are moving column my column here */
			for (i = 0; i < processor_count; i++)
				for (j = 0; j < type_count; j++, k++)
					nc[i].push_back((unsigned int)data[k]);
		}
		else if (strcmp(name, "ceff") == 0) {
			type_count = mxGetM(value);
			processor_count = mxGetN(value);

			if (!(data = mxGetPr(value)))
				throw runtime_error("The Ceff vector is bad.");

			k = 0;
			ceff.resize(processor_count);
			/* NOTE: We are moving column my column here */
			for (i = 0; i < processor_count; i++)
				for (j = 0; j < type_count; j++, k++)
					ceff[i].push_back(data[k]);
		}
		else throw runtime_error("An unknown field.");
	}

	task_count = type.size();

	if (!task_count)
		throw runtime_error("There are no tasks.");

	if (task_count != link.size() || task_count != link[0].size())
		throw runtime_error("The size of the link matrix is wrong.");

	processor_count = frequency.size();

	if (!processor_count)
		throw runtime_error("There are no frequencies.");

	if (processor_count != voltage.size())
		throw runtime_error("The size of the voltage vector is wrong.");

	if (processor_count != ngate.size())
		throw runtime_error("The size of the ngate vector is wrong.");

	if (processor_count != nc.size() || (type_count = nc[0].size()) == 0)
		throw runtime_error("The size of the nc matrix is wrong.");

	if (processor_count != ceff.size() || type_count != ceff[0].size())
		throw runtime_error("The size of the ceff matrix is wrong.");
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	const double deadline_ratio = 1.1;

	char *floorplan = NULL;
	char *config = NULL;
	str_pair *table = NULL;
	size_t tsize = 0;

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

	try {
		if (nrhs != 2) throw runtime_error("A wrong number of input arguments.");

		parse_hotspot_config(prhs[0], &floorplan, &config, &table, &tsize);
		parse_system_config(prhs[1], type, link, frequency, voltage, ngate, nc, ceff);

		graph = new GraphBuilder(type, link);
		architecture = new ArchitectureBuilder(frequency, voltage, ngate, nc, ceff);

		size_t task_count = graph->size();
		size_t processor_count = architecture->size();

		mapping_t mapping(task_count);

		for (size_t i = 0; i < task_count; i++)
			mapping[i] = i % processor_count;

		graph->assign_mapping(architecture, mapping);

		/* Just to assign a deadline */
		schedule_t schedule = ListScheduler::process(graph);
		graph->assign_schedule(schedule);
		graph->assign_deadline(deadline_ratio * graph->calc_duration());

		hotspot = new Hotspot(floorplan, config, table, tsize);
		scheduler = new GeneticListScheduler(graph, hotspot);

		double sampling_interval = hotspot->sampling_interval();
		matrix_t dynamic_power, temperature, total_power;
		DynamicPower::compute(graph, sampling_interval, dynamic_power);
		unsigned int iterations = hotspot->solve(architecture,
			dynamic_power, temperature, total_power);
		double fitness = Lifetime::predict(temperature, sampling_interval);

		std::cout << "Initial lifetime: "
			<< std::setiosflags(std::ios::fixed)
			<< std::setprecision(2)
			<< fitness << std::endl;

		schedule = scheduler->solve();
	}
	catch (exception &e) {
		__MX_FREE(floorplan);
		__MX_FREE(config);
		__MX_FREE(table);

		__DELETE(graph);
		__DELETE(architecture);
		__DELETE(hotspot);
		__DELETE(scheduler);

		mexErrMsgTxt(e.what());
	}

	__MX_FREE(floorplan);
	__MX_FREE(config);
	__MX_FREE(table);

	__DELETE(graph);
	__DELETE(architecture);
	__DELETE(hotspot);
	__DELETE(scheduler);
}
