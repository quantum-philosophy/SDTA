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
	 * to get a power profile where columns are steps, and rows are nodes.
	 */
	int nodes = mxGetM(prhs[1]); /* rows */
	int steps = mxGetN(prhs[1]); /* columns */
	double *power = mxGetPr(prhs[1]);

	double tol = 2;
	if (nrhs > 2) {
		if (!mxIsNumeric(prhs[2])) mexErrMsgTxt(
			"The forth input should be numeric (tol).");
		tol = mxGetScalar(prhs[2]);
	}

	int minbad = 0;
	if (nrhs > 3) {
		if (!mxIsNumeric(prhs[3])) mexErrMsgTxt(
			"The fifth input should be numeric (minbad).");
		minbad = (int)mxGetScalar(prhs[3]);
	}

	int maxit = 10;
	if (nrhs > 4) {
		if (!mxIsNumeric(prhs[4])) mexErrMsgTxt(
			"The fifth input should be numeric (maxit).");
		maxit = (int)mxGetScalar(prhs[4]);
	}

	int tsize = 0;
	str_pair *table = NULL;
	if (nrhs > 5) {
		if (!mxIsStruct(prhs[5])) mexErrMsgTxt(
			"The sixth input should be a structure (config).");
		tsize = parse_structure_config(prhs[5], &table);
		if (!table || !tsize) mexErrMsgTxt(
			"The format of the configuration structure is wrong.");
	}

	/* ATTENTION: The same note (look above) with output temperature.
	 */
    mxArray *out_T = mxCreateDoubleMatrix(nodes, steps, mxREAL);
	double *T = mxGetPr(out_T);

	int it = solve_original(floorplan, config, table, tsize,
		power, nodes, steps, tol, minbad, maxit, T);

	if (table) mxFree(table);
	mxFree(floorplan);
	mxFree(config);

	if (it < 0) {
		mxDestroyArray(out_T);
		mexErrMsgIdAndTxt("HotSpot:solve_original",
			"Cannot solve using the original method (%d).", it);
	}

	mxArray *out_it = mxCreateDoubleMatrix(1, 1, mxREAL);
	*mxGetPr(out_it) = it;

    plhs[0] = out_T;
    plhs[1] = out_it;
}
