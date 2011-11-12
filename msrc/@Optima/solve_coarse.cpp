#include <mex.h>
#include <mex_utils.h>
#include <TestCase.h>

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	string system = from_matlab<string>(prhs[0]);
	string floorplan = from_matlab<string>(prhs[1]);
	string _hotspot = from_matlab<string>(prhs[2]);

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

	TestCase test(system, floorplan, _hotspot, system_tuning, solution_tuning);

	if (solution_tuning.method != "coarse_condensed_equation")
		throw std::runtime_error("Should be coarse.");

	CoarseCondensedEquationHotspot *hotspot =
		dynamic_cast<CoarseCondensedEquationHotspot *>(test.hotspot);

	vector_t intervals;
	matrix_t temperature;
	matrix_t power;

	struct timespec begin, end;

	Time::measure(&begin);
	hotspot->solve(test.schedule, intervals, temperature, power);
	Time::measure(&end);

	plhs[0] = to_matlab(intervals);
	plhs[1] = to_matlab(temperature);
	plhs[2] = to_matlab(power);
	plhs[3] = to_matlab(Time::substract(&end, &begin));
}
