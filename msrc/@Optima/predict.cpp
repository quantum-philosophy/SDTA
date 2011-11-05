#include <mex.h>
#include <mex_utils.h>
#include <Lifetime.h>

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	matrix_t temperature;
	from_matlab(prhs[0], temperature);
	double sampling_interval = from_matlab<double>(prhs[1]);
	ThermalCyclingLifetime lifetime;
	plhs[0] = to_matlab(lifetime.predict(temperature, sampling_interval));
}
