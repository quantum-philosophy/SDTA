#include <mex.h>
#include <mex_utils.h>
#include <TestCase.h>

using namespace std;

mxArray *output(const matrix_t &matrix)
{
	size_t rows = matrix.rows();
	size_t cols = matrix.cols();

    mxArray *out = mxCreateDoubleMatrix(rows, cols, mxREAL);
	double *_out = mxGetPr(out);

	c_matrix_to_mex(_out, matrix.pointer(), rows, cols);

	return out;
}

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

	size_t max_iterations = 1000;
	if (nrhs > 5) {
		if (!mxIsNumeric(prhs[5])) mexErrMsgTxt(
			"'max_iterations' should be numeric.");
		max_iterations = mxGetScalar(prhs[5]);
	}

	int min_mismatches = 0;
	if (nrhs > 6) {
		if (!mxIsNumeric(prhs[6])) mexErrMsgTxt(
			"'min_mismatches' should be numeric.");
		min_mismatches = mxGetScalar(prhs[6]);
	}

	double tolerance = 1;
	if (nrhs > 7) {
		if (!mxIsNumeric(prhs[7])) mexErrMsgTxt(
			"'tolerance' should be numeric.");
		tolerance = mxGetScalar(prhs[7]);
	}

	TestCase test(system, floorplan, hotspot, tuning);

	matrix_t reference_temperature;
	matrix_t power;

	test.hotspot->solve(test.schedule, reference_temperature, power);

	matrix_t temperature;

	IterativeHotspot iterative_hotspot(*test.architecture,
		*test.graph, floorplan, hotspot, tuning.hotspot,
		max_iterations, min_mismatches, tolerance);

	size_t iterations = iterative_hotspot.solve(power, temperature,
		reference_temperature);

    plhs[0] = output(temperature);
    plhs[1] = output(power);
    plhs[2] = output(reference_temperature);
	plhs[3] = output(iterations);
}
