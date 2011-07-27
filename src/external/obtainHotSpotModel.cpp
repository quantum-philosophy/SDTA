#include <mex.h>
#include <hotspot.h>
#include <sys/stat.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs != 2)
		mexErrMsgTxt(
			"Two inputs are required: floorplan and config filenames.");

	if (nlhs != 2)
		mexErrMsgTxt(
			"Two outputs are required: negative matrix A and inverse matrix C.");

	if (!mxIsChar(prhs[0]) || !mxIsChar(prhs[1]))
		mexErrMsgTxt("Both inputs (negA, invC) should be strings.");

	int nodes;
	char *floorplan = mxArrayToString(prhs[0]);
	char *config = mxArrayToString(prhs[1]);

	HotSpotMatrix negA, invC;

	int ret = obtain_hotspot_model(floorplan, config, &nodes, &negA, &invC);

	mxFree(floorplan);
	mxFree(config);

	if (ret < 0) mexErrMsgIdAndTxt("hotspot:bad",
		"Cannot obtain parameters from HotSpot (%d).", ret);

    plhs[0] = mxCreateDoubleMatrix(nodes, nodes, mxREAL);
    plhs[1] = mxCreateDoubleMatrix(nodes, nodes, mxREAL);

	double *out_negA = mxGetPr(plhs[0]);
	double *out_invC = mxGetPr(plhs[1]);

	int i, j;

	for (i = 0; i < nodes; i++)
		for (j = 0; j < nodes; j++) {
			out_negA[i * nodes + j] = negA[i][j];
			out_invC[i * nodes + j] = invC[i][j];
		}

	free_hotspot_matrix(negA);
	free_hotspot_matrix(invC);
}
