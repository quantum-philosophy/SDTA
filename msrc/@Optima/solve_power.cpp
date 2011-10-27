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

	matrix_t temperature, total_power, extended_power;

	matrix_t *used_power = &power;

	if (tuning.solution == "hotspot") {
		size_t step_count = power.rows();
		size_t processor_count = test.architecture->size();
		size_t node_count = 4 * processor_count + 12;

		extended_power.resize(step_count, node_count);
		extended_power.nullify();

		double *_power = power;
		double *_extended_power = extended_power;

		for (size_t i = 0; i < step_count; i++)
			__MEMCPY(_extended_power + i * node_count,
				_power + i * processor_count, processor_count);

		used_power = &extended_power;
	}

	if (tuning.leakage) {
		Time::measure(&begin);
		test.hotspot->solve(*used_power, temperature, total_power);
		Time::measure(&end);
	}
	else {
		Time::measure(&begin);
		test.hotspot->solve(*used_power, temperature);
		Time::measure(&end);
	}

	plhs[0] = to_matlab(temperature);
	plhs[1] = to_matlab(Time::substract(&end, &begin));
	plhs[2] = to_matlab(total_power);
}
