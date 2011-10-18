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

	SystemTuning tuning;
	tuning.setup(params);

	TestCase test(system, floorplan, _hotspot, tuning);

	matrix_t power;
	matrix_t reference_temperature;

	struct timespec begin, end;
	double reference_elapsed, elapsed;

	clock_gettime(CLOCK_MONOTONIC, &begin);
	test.hotspot->solve(test.schedule, reference_temperature, power);
	clock_gettime(CLOCK_MONOTONIC, &end);
	reference_elapsed = substract(&end, &begin);

	matrix_t temperature;

	size_t max_iterations = from_matlab<size_t>(prhs[5], 1000);
	double tolerance = from_matlab<double>(prhs[6], 1);

	IterativeHotspot hotspot(floorplan, _hotspot, tuning.hotspot,
		max_iterations, tolerance);

	clock_gettime(CLOCK_MONOTONIC, &begin);
	size_t iterations = hotspot.verify(power, reference_temperature, temperature);
	clock_gettime(CLOCK_MONOTONIC, &end);
	elapsed = substract(&end, &begin);

	plhs[0] = to_matlab(reference_temperature);
	plhs[1] = to_matlab(reference_elapsed);
	plhs[2] = to_matlab(power);
	plhs[3] = to_matlab(iterations);
	plhs[4] = to_matlab(temperature);
	plhs[5] = to_matlab(elapsed);
}
