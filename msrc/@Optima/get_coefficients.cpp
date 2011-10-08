#include <mex.h>
#include <mex_utils.h>
#include <TestCase.h>

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs < 3)
		mexErrMsgTxt("The number of arguments should be at least three.");

	if (!mxIsChar(prhs[0]) || !mxIsChar(prhs[1]) || !mxIsChar(prhs[2]))
		mexErrMsgTxt("The first three arguments should be strings.");

	string system = array_to_string(prhs[0]);
	string floorplan = array_to_string(prhs[1]);
	string hotspot = array_to_string(prhs[2]);

	SystemTuning tuning;

	if (nrhs > 3) {
		parameters_t params;

		string file = array_to_string(prhs[3]);

		if (!file.empty())
			params.update(file);

		if (nrhs > 4) {
			stringstream stream(array_to_string(prhs[4]));
			params.update(stream);
		}

		tuning.setup(params);
	}

	TestCase test(system, floorplan, hotspot, tuning);

	matrix_t conductance;
	vector_t capacitance;

	test.hotspot->get_conductance(conductance);
	test.hotspot->get_capacitance(capacitance);

	size_t node_count = conductance.rows();

	/* Conductance */
    mxArray *out_conductance = mxCreateDoubleMatrix(node_count, node_count, mxREAL);
	double *_conductance = mxGetPr(out_conductance);

	c_matrix_to_mex(_conductance, conductance.pointer(),
		node_count, node_count);

	/* Capacitance */
    mxArray *out_capacitance = mxCreateDoubleMatrix(1, node_count, mxREAL);
	double *_capacitance = mxGetPr(out_capacitance);

	for (size_t i = 0; i < node_count; i++)
		_capacitance[i] = capacitance[i];

    plhs[0] = out_conductance;
    plhs[1] = out_capacitance;
}
