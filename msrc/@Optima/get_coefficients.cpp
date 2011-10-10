#include <mex.h>
#include <mex_utils.h>
#include <Hotspot.h>

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	matrix_t conductance;
	vector_t capacitance;
	vector_t inversed_capacitance;

	string floorplan = from_matlab<string>(prhs[0]);
	string config = from_matlab<string>(prhs[1]);
	string config_line = from_matlab<string>(prhs[2]);

	Hotspot hotspot(floorplan, config, config_line);

	hotspot.get_conductance(conductance);
	hotspot.get_capacitance(capacitance);
	hotspot.get_inversed_capacitance(inversed_capacitance);

	plhs[0] = to_matlab(conductance);
	plhs[1] = to_matlab(capacitance);
	plhs[2] = to_matlab(inversed_capacitance);
}
