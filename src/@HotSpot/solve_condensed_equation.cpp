#include <mex.h>
#include <hotspot.h>
#include "utils.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char *floorplan, *config;
	verify_and_fetch_properties(nrhs, prhs, &floorplan, &config);

	if (nrhs < 2 || mxIsEmpty(prhs[1])) mexErrMsgTxt(
		"The second input should be a matrix of the power profile.");

	/* ATTENTION: Due to the fact that MatLab stores matrices column by
	 * column, not row by row as regular C/C++ arrays, as input we expect
	 * to get a power profile where columns are steps, and rows are cores.
	 */
	int cores = mxGetM(prhs[1]); /* rows */
	int steps = mxGetN(prhs[1]); /* columns */
	double *power = mxGetPr(prhs[1]);

	int tsize = 0;
	str_pair *table = NULL;
    if (nrhs > 2) {
		if (!mxIsStruct(prhs[2])) mexErrMsgTxt(
			"The third input should be a structure (config).");
		tsize = parse_structure_config(prhs[2], &table);
		if (!table || !tsize) mexErrMsgTxt(
			"The format of the configuration structure is wrong.");
	}

	/* ATTENTION: The same note (look above) with the output temperature.
	 */
    mxArray *out_T = mxCreateDoubleMatrix(cores, steps, mxREAL);
	double *T = mxGetPr(out_T);

	define_timer(calc);

	start_timer(calc);

	int ret = solve_condensed_equation(floorplan, config,
		table, tsize, power, cores, steps, T);

	stop_timer(calc);

	mexPrintf("The condensed equation method: %.3f s (v" VERSION ")\n",
		timer_result(calc));

	if (table) mxFree(table);
	mxFree(floorplan);
	mxFree(config);

	if (ret < 0) {
		mxDestroyArray(out_T);
		mexErrMsgIdAndTxt("HotSpot:solve_condensed_equation",
			"Cannot solve using the condensed equation method (%d).", ret);
	}

    plhs[0] = out_T;
}
