#include <mex.h>
#include <mex_utils.h>
#include <common.h>
#include <Hotspot.h>

#include <string>

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char *floorplan, *config;
	verify_and_fetch_properties(nrhs, prhs, &floorplan, &config);

	if (nrhs < 2 || mxIsEmpty(prhs[1])) mexErrMsgTxt(
		"The second input should be a matrix of the power profile.");

	int step_count = mxGetM(prhs[1]);
	int processor_count = mxGetN(prhs[1]);
	double *_power = mxGetPr(prhs[1]);

	int tsize = 0;
	str_pair *table = NULL;
	if (nrhs > 2) {
		if (!mxIsStruct(prhs[2])) mexErrMsgTxt(
			"The third input should be a structure (config).");
		tsize = parse_structure_config(prhs[2], &table);
		if (!table || !tsize) mexErrMsgTxt(
			"The format of the configuration structure is wrong.");
	}

    mxArray *out_temperature = mxCreateDoubleMatrix(
		step_count, processor_count, mxREAL);
	double *_temperature = mxGetPr(out_temperature);

	Hotspot hotspot(string(floorplan), string(config), table, tsize);

	if (table) mxFree(table);
	mxFree(floorplan);
	mxFree(config);

	matrix_t power(step_count, processor_count);
	matrix_t temperature;

	mex_matrix_to_c(power.pointer(), _power, step_count, processor_count);

	hotspot.solve(power, temperature);

	c_matrix_to_mex(_temperature, temperature.pointer(),
		step_count, processor_count);

    plhs[0] = out_temperature;
}
