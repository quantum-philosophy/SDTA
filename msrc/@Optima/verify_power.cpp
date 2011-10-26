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

	matrix_t power;
	from_matlab(prhs[5], power);

	size_t max_iterations = from_matlab<size_t>(prhs[6], 1000);
	double tolerance = from_matlab<double>(prhs[7], 1);

	SystemTuning tuning;
	tuning.setup(params);

	TestCase test(system, floorplan, _hotspot, tuning);

	struct timespec begin, end;

	matrix_t reference_temperature;

	Time::measure(&begin);
	test.hotspot->solve(power, reference_temperature);
	Time::measure(&end);

	plhs[0] = to_matlab(reference_temperature);
	plhs[1] = to_matlab(Time::substract(&end, &begin));

	matrix_t temperature;

	IterativeHotspot hotspot(floorplan, _hotspot, tuning.hotspot,
		max_iterations, tolerance);

	Time::measure(&begin);
	size_t iterations = hotspot.verify(power, reference_temperature, temperature);
	Time::measure(&end);

	plhs[2] = to_matlab(iterations);
	plhs[3] = to_matlab(temperature);
	plhs[4] = to_matlab(Time::substract(&end, &begin));
}
