#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <time.h>

#include "CommandLine.h"
#include "TestCase.h"
#include "Stream.h"

using namespace std;

class SolveCommandLine: public CommandLine
{
	public:

	string system;
	string floorplan;
	string hotspot;
	string params;
	stringstream param_stream;

	string power;
	string temperature;

	SolveCommandLine() : CommandLine() {}

	void usage() const
	{
		cout
			<< "Usage: solve [-<param name> <param value>]" << endl
			<< endl
			<< "  Available parameters:" << endl
			<< "    s, system      - the architecture and application" << endl
			<< "  * f, floorplan   - the floorplan" << endl
			<< "    h, hotspot     - the thermal configuration" << endl
			<< "    p, parameters  - the tuning parameters" << endl
			<< "    i, power       - input power profile" << endl
			<< "  * o, temperature - output temperature profile" << endl
			<< "    other          - overwrite the tuning parameters" << endl
			<< endl
			<< "  (* required parameters)" << endl;
	}

	protected:

	void verify() const
	{
		if (system.empty() || !File::exist(system))
			throw runtime_error("The system configuration file does not exist.");

		if (floorplan.empty() || !File::exist(floorplan))
			throw runtime_error("The floorplan configuration file does not exist.");

		if (!hotspot.empty() && !File::exist(hotspot))
			throw runtime_error("The Hotspot configuration file does not exist.");

		if (!params.empty() && !File::exist(params))
			throw runtime_error("The tuning configuration file does not exist.");

		if (!power.empty() && !File::exist(power))
			throw runtime_error("The power profile file does not exist.");

		if (temperature.empty())
			throw runtime_error("The output file should be specified.");
	}

	void process(const string &name, const string &value)
	{
		if (name == "s" || name == "system") system = value;
		else if (name == "f" || name == "floorplan") floorplan = value;
		else if (name == "h" || name == "hotspot") hotspot = value;
		else if (name == "p" || name == "parameters") params = value;
		else if (name == "i" || name == "power") power = value;
		else if (name == "o" || name == "temperature") temperature = value;
		else param_stream << name << " " << value << endl;
	}
};

void solve(const string &system, const string &floorplan,
	const string &hotspot, const string &_params,
	stringstream &param_stream, const string &_power,
	const string &_temperature)
{
	parameters_t params(_params);
	params.update(param_stream);

	SystemTuning system_tuning;
	system_tuning.setup(params);

	SolutionTuning solution_tuning;
	solution_tuning.setup(params);

	TestCase test(system, floorplan, hotspot, system_tuning, solution_tuning);

	matrix_t power, temperature;
	size_t processor_count = test.architecture->size();

	if (!_power.empty()) {
		size_t step_count = ceil(test.graph->get_deadline() /
			test.hotspot->get_sampling_interval());

		InputStream power_stream(_power);
		power_stream.read(power, step_count);
	}
	else {
		DynamicPower dynamic_power(test.architecture->get_processors(),
			test.graph->get_tasks(), test.graph->get_deadline(),
			test.hotspot->get_sampling_interval());
		dynamic_power.compute(test.schedule, power);
	}

	struct timespec begin, end;

	Time::measure(&begin);
	test.hotspot->solve(power, temperature);
	Time::measure(&end);

	OutputStream temperature_stream(_temperature, processor_count);
	temperature_stream.write(temperature);

	if (system_tuning.verbose)
		cout << "Solved in " << Time::substract(&end, &begin) << " s" << endl;
}

int main(int argc, char **argv)
{
	SolveCommandLine arguments;

	try {
		arguments.parse(argc, (const char **)argv);
		solve(arguments.system, arguments.floorplan, arguments.hotspot,
			arguments.params, arguments.param_stream, arguments.power,
			arguments.temperature);
	}
	catch (exception &e) {
		cerr << e.what() << endl;
		arguments.usage();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
