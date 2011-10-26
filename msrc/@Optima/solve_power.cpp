#include <mex.h>
#include <mex_utils.h>
#include <TestCase.h>

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	string system = from_matlab<string>(prhs[0]);
	string floorplan = from_matlab<string>(prhs[1]);
	string hotspot = from_matlab<string>(prhs[2]);

	parameters_t params;

	string param_filename = from_matlab<string>(prhs[3], string());
	if (!param_filename.empty()) params.update(param_filename);

	string param_line = from_matlab<string>(prhs[4], string());
	if (!param_line.empty()) {
		stringstream param_stream(param_line);
		params.update(param_stream);
	}

	matrix_t power;
	from_matlab(prhs[5], power);

	SystemTuning tuning;
	tuning.setup(params);

	TestCase test(system, floorplan, hotspot, tuning);

	struct timespec begin, end;

	matrix_t temperature, total_power;

	if (tuning.leakage) {
		Time::measure(&begin);
		test.hotspot->solve(power, temperature, total_power);
		Time::measure(&end);
	}
	else {
		Time::measure(&begin);
		test.hotspot->solve(power, temperature);
		Time::measure(&end);
	}

	plhs[0] = to_matlab(temperature);
	plhs[1] = to_matlab(Time::substract(&end, &begin));
	plhs[2] = to_matlab(total_power);
}
