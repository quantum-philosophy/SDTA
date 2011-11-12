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

	SystemTuning system_tuning;
	system_tuning.setup(params);

	SolutionTuning solution_tuning;
	solution_tuning.setup(params);

	TestCase test(system, floorplan, hotspot, system_tuning, solution_tuning);

	matrix_t power;
	matrix_t temperature;

	struct timespec begin, end;

	Time::measure(&begin);
	test.hotspot->solve(test.schedule, temperature, power);
	Time::measure(&end);

	plhs[0] = to_matlab(temperature);
	plhs[1] = to_matlab(power);
	plhs[2] = to_matlab(Time::substract(&end, &begin));
}
