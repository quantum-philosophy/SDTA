#include <mex.h>
#include <mex_utils.h>
#include <common.h>
#include <Hotspot.h>
#include <string.h>

#include <string>

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char *floorplan, *config;
	verify_and_fetch_properties(nrhs, prhs, &floorplan, &config);

	Hotspot hotspot(string(floorplan), string(config), NULL, 0);

	mxFree(floorplan);
	mxFree(config);

	matrix_t neg_a;
	vector_t inv_c;

	hotspot.calc_coefficients(neg_a, inv_c);

	int node_count = inv_c.size();

    plhs[0] = mxCreateDoubleMatrix(node_count, node_count, mxREAL);
	c_matrix_to_mex(mxGetPr(plhs[0]), neg_a.pointer(), node_count, node_count);

    plhs[1] = mxCreateDoubleMatrix(1, node_count, mxREAL);
	memcpy(mxGetPr(plhs[1]), &inv_c[0], sizeof(double) * node_count);
}
