#include <mex.h>
#include <TestCase.h>

using namespace std;

void mex_matrix_to_c(double *dest, const double *src, size_t rows, size_t cols)
{
	for (size_t i = 0; i < rows; i++)
		for (size_t j = 0; j < cols; j++)
			dest[i * cols + j] = src[i + j * rows];
}

void c_matrix_to_mex(double *dest, const double *src, size_t rows, size_t cols)
{
	for (size_t i = 0; i < rows; i++)
		for (size_t j = 0; j < cols; j++)
			dest[i + j * rows] = src[i * cols + j];
}

string array_to_string(const mxArray *array)
{
	char *pointer = mxArrayToString(array);

	if (!pointer)
		mexErrMsgTxt("Cannot read the string.");

	string line(pointer);

	mxFree(pointer);

	return line;
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
