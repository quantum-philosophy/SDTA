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

	matrix_t temperature;
	matrix_t power;

	test.hotspot->solve(test.schedule, temperature, power);

	size_t step_count = temperature.rows();
	size_t processor_count = temperature.cols();

	/* Temperature */
    mxArray *out_temperature = mxCreateDoubleMatrix(
		step_count, processor_count, mxREAL);
	double *_temperature = mxGetPr(out_temperature);

	c_matrix_to_mex(_temperature, temperature.pointer(),
		step_count, processor_count);

	/* Power */
    mxArray *out_power = mxCreateDoubleMatrix(
		step_count, processor_count, mxREAL);
	double *_power = mxGetPr(out_power);

	c_matrix_to_mex(_power, power.pointer(),
		step_count, processor_count);

    plhs[0] = out_temperature;
    plhs[1] = out_power;
}
