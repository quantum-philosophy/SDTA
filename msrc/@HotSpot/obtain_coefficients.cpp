#include <mex.h>
#include <hotspot.h>
#include "utils.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char *floorplan, *config;
	verify_and_fetch_properties(nrhs, prhs, &floorplan, &config);

	double *negA, *dinvC;

	int nodes = obtain_coefficients(floorplan, config, &negA, &dinvC,
		mxMalloc, mxFree);

	mxFree(floorplan);
	mxFree(config);

	if (nodes < 0) mexErrMsgIdAndTxt("HotSpot:obtain_coefficients",
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
