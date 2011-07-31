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

	char *floorplan = mxArrayToString(prhs[0]);
	char *power = mxArrayToString(prhs[1]);
	char *config = mxArrayToString(prhs[2]);

	HotSpotVector T;

	int cores;
	int ret = solve_sst_with_hotspot(floorplan, power, config, &cores, &T);

	mxFree(floorplan);
	mxFree(power);
	mxFree(config);

	if (ret < 0) mexErrMsgIdAndTxt("hotspot:bad",
		"Cannot solve SST with HotSpot (%d).", ret);

    plhs[0] = mxCreateDoubleMatrix(cores, 1, mxREAL);

	double *out_T = mxGetPr(plhs[0]);
	memcpy(out_T, T, sizeof(double) * cores);

	free_hotspot_vector(T);
}
