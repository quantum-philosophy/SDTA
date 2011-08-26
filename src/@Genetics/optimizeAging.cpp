#include <stdexcept>
#include <mex.h>

#include "TaskGraph.h"
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
	char **floorplan, char **config, vector<unsigned long int> &nc,
	vector<double> &ceff, vector<vector<bool> > &link,
	vector<double> &frequency, vector<double> &voltage,
	vector<unsigned long int> &ngate)
{
	*floorplan = NULL;
	*config = NULL;

	try {
		if (nrhs < 8)
			throw runtime_error("Not enough input arguments.");

		int i, j, k, no, size;
		double *data;

		no = 0;

		if (!mxIsChar(ARG) || !(*floorplan = mxArrayToString(ARG)))
			throw runtime_error("The floorplan is bad.");

		no++;

		if (!mxIsChar(ARG) || !(*config = mxArrayToString(ARG)))
			throw runtime_error("The configuration file is bad.");

		no++;

		nc.clear();
		data = mxGetPr(ARG);
		size = mxGetM(ARG) * mxGetN(ARG);
		for (i = 0; i < size; i++) nc.push_back((unsigned long int)data[i]);

		no++;

		ceff.clear();
		data = mxGetPr(ARG);
		size = mxGetM(ARG) * mxGetN(ARG);
		for (i = 0; i < size; i++) ceff.push_back(data[i]);

		no++;

		link.clear();
		data = mxGetPr(ARG);
		size = mxGetM(ARG);
		if (mxGetN(ARG) != size)
			throw runtime_error("The link matrix is bad.");
		k = 0;
		link.resize(size);
		/* NOTE: We are moving column my column here */
		for (i = 0; i < size; i++)
			for (j = 0; j < size; j++, k++)
				link[j].push_back((bool)data[k]);

		if (nc.size() != ceff.size() || nc.size() != link.size())
			throw runtime_error("The task properties do not agree.");

		no++;

		frequency.clear();
		data = mxGetPr(ARG);
		size = mxGetM(ARG) * mxGetN(ARG);
		for (i = 0; i < size; i++) frequency.push_back(data[i]);

		no++;

		voltage.clear();
		data = mxGetPr(ARG);
		size = mxGetM(ARG) * mxGetN(ARG);
		for (i = 0; i < size; i++) voltage.push_back(data[i]);

		no++;

		ngate.clear();
		data = mxGetPr(ARG);
		size = mxGetM(ARG) * mxGetN(ARG);
		for (i = 0; i < size; i++) ngate.push_back((unsigned long int)data[i]);

		if (frequency.size() != voltage.size() || frequency.size() != ngate.size())
			throw runtime_error("The processor properties do not agree.");
	}
	catch (...) {
		if (*floorplan) mxFree(*floorplan);
		if (*config) mxFree(*config);
		throw;
	}
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char *floorplan = NULL;
	char *config = NULL;
	Graph *graph = NULL;
	Hotspot *hotspot = NULL;
	GeneticListScheduler *scheduler = NULL;

	vector<unsigned long int> nc;
	vector<double> ceff;
	vector<vector<bool> > link;
	vector<double> frequency;
	vector<double> voltage;
	vector<unsigned long int> ngate;

	try {
		fetch_configuration(nrhs, prhs, &floorplan, &config,
			nc, ceff, link, frequency, voltage, ngate);

		graph = Graph::build(nc, ceff, link, frequency, voltage, ngate);
		hotspot = new Hotspot(floorplan, config);
		scheduler = new GeneticListScheduler(graph, hotspot);
	}
	catch (exception &e) {
		__DELETE(graph);
		__DELETE(hotspot);
		__DELETE(scheduler);
		__MX_FREE(floorplan);
		__MX_FREE(config);
		mexErrMsgTxt(e.what());
	}

	__DELETE(graph);
	__DELETE(hotspot);
	__DELETE(scheduler);
	__MX_FREE(floorplan);
	__MX_FREE(config);
}
