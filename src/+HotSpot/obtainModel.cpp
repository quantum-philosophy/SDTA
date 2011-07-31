#include <mex.h>
#include <hotspot.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs != 2) mexErrMsgTxt(
		"Exactly two inputs are required: floorplan, and config.");

	if (!mxIsChar(prhs[0]) || !mxIsChar(prhs[1]))
		mexErrMsgTxt("Both inputs should be file names.");

	char *floorplan = mxArrayToString(prhs[0]);
	char *config = mxArrayToString(prhs[1]);

	double *negA, *dinvC;

	int nodes = obtain_coefficients(floorplan, config, &negA, &dinvC,
		mxMalloc, mxFree);

	mxFree(floorplan);
	mxFree(config);

	if (nodes < 0) mexErrMsgIdAndTxt("obtainModel:obtain_coefficients",
		"Cannot obtain the coefficient matrices (%d).", nodes);

    plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
	mxSetPr(plhs[0], negA);
	mxSetM(plhs[0], nodes);
	mxSetN(plhs[0], nodes);

    plhs[1] = mxCreateDoubleMatrix(0, 0, mxREAL);
	mxSetPr(plhs[1], dinvC);
	mxSetM(plhs[1], 1);
	mxSetN(plhs[1], nodes);
}
