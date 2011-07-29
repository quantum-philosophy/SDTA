#include <mex.h>
#include <hotspot.h>
#include <sys/stat.h>
#include <string.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs < 3)
		mexErrMsgTxt(
			"Three inputs are required: floorplan, config, and power.");

	if (!mxIsChar(prhs[0]) || !mxIsChar(prhs[1]))
		mexErrMsgTxt(
			"1st and 2nd inputs (floorplan and config) should be strings.");

	/* ATTENTION: Due to the fact that MatLab stores matrices column by
	 * column, not row by row as regular c/c++ arrays, as input we expect
	 * to get a power profile where columns are steps, and rows are nodes.
	 */
	int nodes = mxGetM(prhs[2]); /* rows */
	int steps = mxGetN(prhs[2]); /* columns */

	if (nodes <= 0 || steps <= 0)
		mexErrMsgTxt(
			"3rd input (power) should be a matrix.");

	double *power = mxGetPr(prhs[2]);

	double tol = 2;
	if (nrhs > 3) {
		if (!mxIsNumeric(prhs[3]))
			mexErrMsgTxt("4th input (tol) should be numeric.");
		tol = mxGetScalar(prhs[3]);
	}

	int maxit = 10;
	if (nrhs > 4) {
		if (!mxIsNumeric(prhs[4]))
			mexErrMsgTxt("5th input (maxit) should be numeric.");
		maxit = (int)mxGetScalar(prhs[4]);
	}

	char *dump = NULL;
	if (nrhs > 5) {
		if (!mxIsChar(prhs[5]))
			mexErrMsgTxt("6th input (dump) should be string.");
		dump = mxArrayToString(prhs[5]);
	}

	char *floorplan = mxArrayToString(prhs[0]);
	char *config = mxArrayToString(prhs[1]);

	/* ATTENTION: The same note (look above) with output temperature.
	 */
    mxArray *out_T = mxCreateDoubleMatrix(nodes, steps, mxREAL);
	double *T = mxGetPr(out_T);

	int it = solve_ssdtc_with_hotspot(floorplan, config, power,
		nodes, steps, tol, maxit, T, dump, mexPrintf);

	mxFree(floorplan);
	mxFree(config);
	mxFree(dump);

	if (it < 0) {
		mxDestroyArray(out_T);
		mexErrMsgIdAndTxt("hotspot:bad",
			"Cannot solve SSDTC with HotSpot (%d).", it);
	}

	mxArray *out_it = mxCreateDoubleMatrix(1, 1, mxREAL);
	*mxGetPr(out_it) = it;

    plhs[0] = out_T;
    plhs[1] = out_it;
}
