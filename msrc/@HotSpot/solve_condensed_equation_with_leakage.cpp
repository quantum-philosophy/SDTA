#include <mex.h>
#include <hotspot.h>
#include "utils.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char *floorplan, *config;
	verify_and_fetch_properties(nrhs, prhs, &floorplan, &config);

	if (nrhs < 2 || mxIsEmpty(prhs[1])) mexErrMsgTxt(
		"The second input should be the dynamic power profile.");

	/* ATTENTION: Due to the fact that MatLab stores matrices column by
	 * column, not row by row as regular C/C++ arrays, as input we expect
	 * to get a power profile where columns are steps, and rows are cores.
	 */
	int cores = mxGetM(prhs[1]); /* rows */
	int steps = mxGetN(prhs[1]); /* columns */
	double *dynamic_power = mxGetPr(prhs[1]);

	if (nrhs < 3 || mxIsEmpty(prhs[2])) mexErrMsgTxt(
		"The third input should be a vector of the voltage supply.");
	if (mxGetN(prhs[2]) * mxGetM(prhs[2]) != cores) mexErrMsgTxt(
		"Dimensions of the power profile and the voltage supply should agree.");
	double *vdd = mxGetPr(prhs[2]);

	if (nrhs < 4 || mxIsEmpty(prhs[3])) mexErrMsgTxt(
		"The fourth input should be a vector of the number of gates.");
	if (mxGetN(prhs[3]) * mxGetM(prhs[3]) != cores) mexErrMsgTxt(
		"Dimensions of the power profile and the number of gates should agree.");
	double *ngate = mxGetPr(prhs[3]);

	double tol = 0.01;
	if (nrhs > 4) {
		if (!mxIsNumeric(prhs[4])) mexErrMsgTxt(
			"The fifth input should be numeric (tol).");
		tol = mxGetScalar(prhs[4]);
	}

	int maxit = 10;
	if (nrhs > 5) {
		if (!mxIsNumeric(prhs[5])) mexErrMsgTxt(
			"The sixth input should be numeric (maxit).");
		maxit = (int)mxGetScalar(prhs[5]);
	}

	int tsize = 0;
	str_pair *table = NULL;
	if (nrhs > 6) {
		if (!mxIsStruct(prhs[6])) mexErrMsgTxt(
			"The seventh input should be a structure (config).");
		tsize = parse_structure_config(prhs[6], &table);
		if (!table || !tsize) mexErrMsgTxt(
			"The format of the configuration structure is wrong.");
	}

	/* ATTENTION: The same note as above */
    mxArray *out_T = mxCreateDoubleMatrix(cores, steps, mxREAL);
	double *T = mxGetPr(out_T);

	int it = solve_condensed_equation_with_leakage(floorplan, config,
		table, tsize, cores, steps, dynamic_power, vdd, ngate, T, tol, maxit);

	if (table) mxFree(table);
	mxFree(floorplan);
	mxFree(config);

	if (it < 0) {
		mxDestroyArray(out_T);
		mexErrMsgIdAndTxt("HotSpot:solve_condensed_equation_with_leakage",
			"Cannot solve using the condensed equation method (%d).", it);
	}

	mxArray *out_it = mxCreateDoubleMatrix(1, 1, mxREAL);
	*mxGetPr(out_it) = it;

    plhs[0] = out_T;
    plhs[1] = out_it;
}
