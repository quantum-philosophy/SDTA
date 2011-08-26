#include <mex.h>
#include <gaListScheduler.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	gaListScheduler ga;
	ga.solve();
}
