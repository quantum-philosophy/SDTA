#include <mex.h>
#include <hotspot.h>
#include <sys/stat.h>
#include <string.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs < 3)
		mexErrMsgTxt(
			"Three inputs are required: floorplan, powerprofile, and config.");

	if (!mxIsChar(prhs[0]) || !mxIsChar(prhs[1]) || !mxIsChar(prhs[2]))
		mexErrMsgTxt(
			"First three inputs (floorplan, power, config) should be strings.");

	if (nrhs > 3 && !mxIsNumeric(prhs[3]))
		mexErrMsgTxt(
			"The fourth input (tol) should be numeric.");

	if (nrhs > 4 && !mxIsNumeric(prhs[4]))
		mexErrMsgTxt(
			"The fifth input (maxit) should be numeric.");

	if (nrhs > 5 && !mxIsChar(prhs[5]))
		mexErrMsgTxt(
			"The sixth input (dump) should be string.");

	char *floorplan = mxArrayToString(prhs[0]);
	char *power = mxArrayToString(prhs[1]);
	char *config = mxArrayToString(prhs[2]);

	double tol = 1e-2;
	if (nrhs > 3) tol = mxGetScalar(prhs[3]);

	int maxit = 100;
	if (nrhs > 4) maxit = (int)mxGetScalar(prhs[4]);

	char *dump = NULL;
	if (nrhs > 5) dump = mxArrayToString(prhs[5]);

	HotSpotVector T;

	int steps, cores;
	int it = solve_ssdtc_with_hotspot(floorplan, power, config, tol, maxit,
		&steps, &cores, &T, dump);

	mxFree(floorplan);
	mxFree(power);
	mxFree(config);

	if (it < 0) mexErrMsgIdAndTxt("hotspot:bad",
		"Cannot solve SSDTC with HotSpot (%d).", it);

    plhs[0] = mxCreateDoubleMatrix(steps * cores, 1, mxREAL);
    plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);

	double *out_T = mxGetPr(plhs[0]);
	memcpy(out_T, T, sizeof(double) * cores * steps);

	double *out_it = mxGetPr(plhs[1]);
	*out_it = it;

	free_hotspot_vector(T);
}
