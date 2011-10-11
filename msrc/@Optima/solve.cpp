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

	SystemTuning tuning;
	tuning.setup(params);

	TestCase test(system, floorplan, hotspot, tuning);

	matrix_t power;
	matrix_t temperature;

	clock_t begin, end;
	double elapsed;

	if (!tuning.steady_state) {
		DynamicPower dynamic_power(test.architecture->get_processors(),
			test.graph->get_tasks(), test.graph->get_deadline(),
			test.hotspot->get_sampling_interval());

		dynamic_power.compute(test.schedule, power);

		begin = clock();
		test.hotspot->solve(power, temperature);
		end = clock();
	}
	else {
		begin = clock();
		test.hotspot->solve(test.schedule, temperature, power);
		end = clock();
	}

	elapsed = (double)(end - begin) / CLOCKS_PER_SEC;

	plhs[0] = to_matlab(temperature);
	plhs[1] = to_matlab(power);
	plhs[2] = to_matlab(elapsed);
}
