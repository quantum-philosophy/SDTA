#include <mex.h>
#include <hotspot.h>

#include "utils.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs < 3) mexErrMsgTxt(
		"At least three inputs are required: floorplan, config, and power.");

	if (!mxIsChar(prhs[0]) || !mxIsChar(prhs[1])) mexErrMsgTxt(
		"The first two inputs should be file names.");

	/* ATTENTION: Due to the fact that MatLab stores matrices column by
	 * column, not row by row as regular C/C++ arrays, as input we expect
	 * to get a power profile where columns are steps, and rows are cores.
	 */
	int cores = mxGetM(prhs[2]); /* rows */
	int steps = mxGetN(prhs[2]); /* columns */

	if (cores <= 0 || steps <= 0) mexErrMsgTxt(
		"The third input should be a matrix.");

	int tsize = 0;
	str_pair *table = NULL;

    if (nrhs > 3) {
		if (!mxIsStruct(prhs[3])) mexErrMsgTxt(
			"The forth input should be a structure.");
		tsize = parse_structure_config(prhs[3], &table);
		if (!table || !tsize) mexErrMsgTxt(
			"The format of the configuration structure is not supported.");
	}

	char *floorplan = mxArrayToString(prhs[0]);
	char *config = mxArrayToString(prhs[1]);
	double *power = mxGetPr(prhs[2]);

	/* ATTENTION: The same note (look above) with the output temperature.
	 */
    mxArray *out_T = mxCreateDoubleMatrix(cores, steps, mxREAL);
	double *T = mxGetPr(out_T);

	define_timer(calc);

	start_timer(calc);

	int ret = solve_ssdtc_condensed_equation(floorplan, config,
		table, tsize, power, cores, steps, T);

	stop_timer(calc);

	mexPrintf("The condensed equation method: %.3f s\n", timer_result(calc));

	if (table) mxFree(table);
	mxFree(floorplan);
	mxFree(config);

	if (ret < 0) {
		mxDestroyArray(out_T);
		mexErrMsgIdAndTxt("solveSSDTCEC:solve_ssdtc_condensed_equation",
			"Cannot solve SSDTC using the condensed equation method (%d).", ret);
	}

    plhs[0] = out_T;
}
