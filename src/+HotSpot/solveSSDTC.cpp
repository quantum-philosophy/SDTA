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
	 * to get a power profile where columns are steps, and rows are nodes.
	 */
	int nodes = mxGetM(prhs[2]); /* rows */
	int steps = mxGetN(prhs[2]); /* columns */

	if (nodes <= 0 || steps <= 0) mexErrMsgTxt(
		"The third input should be a matrix.");

	double *power = mxGetPr(prhs[2]);

	double tol = 2;
	if (nrhs > 3) {
		if (!mxIsNumeric(prhs[3])) mexErrMsgTxt(
			"The forth input (tol) should be numeric.");
		tol = mxGetScalar(prhs[3]);
	}

	int minbad = 0;
	if (nrhs > 4) {
		if (!mxIsNumeric(prhs[4])) mexErrMsgTxt(
			"The fifth input (minbad) should be numeric.");
		minbad = (int)mxGetScalar(prhs[4]);
	}

	int maxit = 10;
	if (nrhs > 5) {
		if (!mxIsNumeric(prhs[5])) mexErrMsgTxt(
			"The fifth input (maxit) should be numeric.");
		maxit = (int)mxGetScalar(prhs[5]);
	}

	char *floorplan = mxArrayToString(prhs[0]);
	char *config = mxArrayToString(prhs[1]);

	/* ATTENTION: The same note (look above) with output temperature.
	 */
    mxArray *out_T = mxCreateDoubleMatrix(nodes, steps, mxREAL);
	double *T = mxGetPr(out_T);

	define_timer(calc);

	start_timer(calc);

	int it = solve_ssdtc_original(floorplan, config, power,
		nodes, steps, tol, minbad, maxit, T);

	stop_timer(calc);

	mexPrintf("The original solution: %.3f s (v" VERSION ")\n", timer_result(calc));

	mxFree(floorplan);
	mxFree(config);

	if (it < 0) {
		mxDestroyArray(out_T);
		mexErrMsgIdAndTxt("solveSSDTC:solve_ssdtc_original",
			"Cannot solve SSDTC using the original method (%d).", it);
	}

	mxArray *out_it = mxCreateDoubleMatrix(1, 1, mxREAL);
	*mxGetPr(out_it) = it;

    plhs[0] = out_T;
    plhs[1] = out_it;
}
