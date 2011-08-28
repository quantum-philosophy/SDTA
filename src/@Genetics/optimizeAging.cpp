#include <mex.h>

#include <vector>
#include <stdexcept>
#include <iostream>

#include "Graph.h"
#include "Architecture.h"
#include "Hotspot.h"
#include "GeneticListScheduler.h"

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

#define ARG (prhs[no])

void fetch_configuration(int nrhs, const mxArray *prhs[],
	char **floorplan, char **config, vector<unsigned int> &type,
	vector<vector<bool> > &link, vector<double> &frequency,
	vector<double> &voltage, vector<unsigned long int> &ngate,
	vector<vector<unsigned long int> > &nc, vector<vector<double> > &ceff)
{
	*floorplan = NULL;
	*config = NULL;

	try {
		if (nrhs < 9)
			throw runtime_error("Not enough input arguments.");

		int no;
		size_t i, j, k, task_count, processor_count, type_count;
		double *data;

		no = 0;

		if (!mxIsChar(ARG) || !(*floorplan = mxArrayToString(ARG)))
			throw runtime_error("The floorplan is bad.");

		no++;

		if (!mxIsChar(ARG) || !(*config = mxArrayToString(ARG)))
			throw runtime_error("The configuration file is bad.");

		no++;

		type.clear();
		task_count = mxGetM(ARG) * mxGetN(ARG);
		if (!(data = mxGetPr(ARG)))
			throw runtime_error("The type vector is bad.");
		for (i = 0; i < task_count; i++)
			type.push_back((unsigned int)data[i]);

		no++;

		link.clear();
		if (!(data = mxGetPr(ARG)) ||
			task_count != mxGetM(ARG) ||
			task_count != mxGetN(ARG))
			throw runtime_error("The link matrix is bad.");
		k = 0;
		link.resize(task_count);
		/* NOTE: We are moving column my column here */
		for (i = 0; i < task_count; i++)
			for (j = 0; j < task_count; j++, k++)
				link[j].push_back((bool)data[k]);

		no++;

		frequency.clear();
		processor_count = mxGetM(ARG) * mxGetN(ARG);
		if (!(data = mxGetPr(ARG)))
			throw runtime_error("The frequency vector is bad.");
		for (i = 0; i < processor_count; i++)
			frequency.push_back(data[i]);

		no++;

		voltage.clear();
		if (!(data = mxGetPr(ARG)) ||
			processor_count != mxGetM(ARG) * mxGetN(ARG))
			throw runtime_error("The voltage vector is bad.");
		for (i = 0; i < processor_count; i++)
			voltage.push_back(data[i]);

		no++;

		ngate.clear();
		if (!(data = mxGetPr(ARG)) ||
			processor_count != mxGetM(ARG) * mxGetN(ARG))
			throw runtime_error("The NGATE vector is bad.");
		for (i = 0; i < processor_count; i++)
			ngate.push_back((unsigned long int)data[i]);

		no++;

		nc.clear();
		type_count = mxGetM(ARG);
		if (!(data = mxGetPr(ARG)) ||
			processor_count != mxGetN(ARG))
			throw runtime_error("The NC vector is bad.");
		k = 0;
		nc.resize(processor_count);
		/* NOTE: We are moving column my column here */
		for (i = 0; i < processor_count; i++)
			for (j = 0; j < type_count; j++, k++)
				nc[i].push_back((unsigned int)data[k]);

		no++;

		ceff.clear();
		if (!(data = mxGetPr(ARG)) ||
			type_count != mxGetM(ARG) ||
			processor_count != mxGetN(ARG))
			throw runtime_error("The Ceff vector is bad.");
		k = 0;
		ceff.resize(processor_count);
		/* NOTE: We are moving column my column here */
		for (i = 0; i < processor_count; i++)
			for (j = 0; j < type_count; j++, k++)
				ceff[i].push_back((unsigned int)data[k]);
	}
	catch (...) {
		__MX_FREE(*floorplan);
		__MX_FREE(*config);
		throw;
	}
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char *floorplan = NULL;
	char *config = NULL;
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
		fetch_configuration(nrhs, prhs, &floorplan, &config,
			type, link, frequency, voltage, ngate, nc, ceff);

		graph = new GraphBuilder(type, link);
		architecture = new ArchitectureBuilder(frequency, voltage, ngate, nc, ceff);

		size_t task_count = graph->size();
		size_t processor_count = architecture->size();

		mapping_t mapping(task_count);

		for (size_t i = 0; i < task_count; i++)
			mapping[i] = i % processor_count;

		graph->assign_mapping(architecture, mapping);

		std::cout << graph;
		std::cout << architecture;

		hotspot = new Hotspot(floorplan, config);
		scheduler = new GeneticListScheduler();

		scheduler->solve(graph, hotspot);
	}
	catch (exception &e) {
		__DELETE(graph);
		__DELETE(architecture);
		__DELETE(hotspot);
		__DELETE(scheduler);
		__MX_FREE(floorplan);
		__MX_FREE(config);
		mexErrMsgTxt(e.what());
	}

	__DELETE(graph);
	__DELETE(architecture);
	__DELETE(hotspot);
	__DELETE(scheduler);
	__MX_FREE(floorplan);
	__MX_FREE(config);
}
