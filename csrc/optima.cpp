#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <time.h>

#include "CommandLine.h"
#include "TestCase.h"
#include "SOEvolution.h"
#include "MOEvolution.h"
#include "Evaluation.h"

using namespace std;

class OptimaCommandLine: public CommandLine
{
	public:

	string system;
	string floorplan;
	string hotspot;
	string params;
	stringstream param_stream;

	OptimaCommandLine() : CommandLine() {}

	void usage() const
	{
		cout
			<< "Usage: optima [-<param name> <param value>]" << endl
			<< endl
			<< "  Available parameters:" << endl
			<< "  * s, system      - a task graph with a number of PEs" << endl
			<< "  * f, floorplan   - a floorplan for the PEs" << endl
			<< "    h, hotspot     - the Hotspot configuration" << endl
			<< "    p, parameters  - the tuning parameters" << endl
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
	}

	void process(const string &name, const string &value)
	{
		if (name == "s" || name == "system") system = value;
		else if (name == "f" || name == "floorplan") floorplan = value;
		else if (name == "h" || name == "hotspot") hotspot = value;
		else if (name == "p" || name == "parameters") params = value;
		else param_stream << name << " " << value << endl;
	}
};

void backup(const string &iname, int index)
{
	stringstream oname;
	oname << iname << "_" << index;
	ifstream is(iname.c_str(), ios::binary);
	ofstream os(oname.str().c_str(), ios::binary);
	os << is.rdbuf();
}

void optimize(const string &system, const string &floorplan,
	const string &hotspot, const string &_params,
	stringstream &param_stream)
{
	parameters_t params(_params);
	params.update(param_stream);

	EvolutionTuning evolution_tuning;
	evolution_tuning.setup(params);

	SystemTuning &system_tuning = evolution_tuning.system;
	OptimizationTuning &optimization_tuning = evolution_tuning.optimization;

	if (system_tuning.verbose)
		cout << evolution_tuning << endl;

	Random::set_seed(optimization_tuning.seed, system_tuning.verbose);

	size_t repeat = optimization_tuning.repeat < 0 ? 1 : optimization_tuning.repeat;

	BasicEvolution *evolution = NULL;
	Evaluation *evaluation = NULL;

	try {
		TestCase test(system, floorplan, hotspot, system_tuning);

		/* Obtain the initial measurements to compare with.
		 *
		 */
		if (optimization_tuning.cache.empty()) {
			evaluation = new Evaluation(*test.architecture, *test.graph,
				*test.hotspot, !optimization_tuning.mapping);
		}
		else {
#ifndef WITHOUT_MEMCACHED
			evaluation = new MemcachedEvaluation(optimization_tuning.cache,
				optimization_tuning.multiobjective, *test.architecture,
				*test.graph, *test.hotspot, !optimization_tuning.mapping);
#else
			throw runtime_error("The code is compiled without caching support.");
#endif
		}

		price_t price = evaluation->process(test.schedule);

		constrains_t constrains;

		if (optimization_tuning.mapping)
			constrains = Constrain::structural(
				*test.architecture, *test.graph);
		else
			constrains = Constrain::structural(
				*test.architecture, *test.graph, test.mapping);

		if (system_tuning.verbose) {
			cout
				<< test.graph << endl
				<< test.architecture << endl
				<< "Start mapping: " << print_t<pid_t>(test.mapping) << endl
				<< "Start priority: " << print_t<rank_t>(test.priority) << endl
				<< "Start schedule:" << endl << test.schedule << endl
				<< "Constrains: " << print_t<constrain_t>(constrains) << endl;

			size_t out = 0;
			size_t task_count = test.graph->size();
			for (size_t i = 0; i < task_count; i++)
				if (test.priority[i] < constrains[i].min ||
					test.priority[i] > constrains[i].max) out++;

			if (out > 0)
				cout << "Out of range priorities: " << out << endl;
		}

		cout
			<< "Initial lifetime: "
			<< setiosflags(ios::fixed) << setprecision(2)
			<< price.lifetime << endl
			<< "Initial energy: "
			<< price.energy << endl;

		for (size_t i = 0; i < repeat; i++) {
			Random::reseed();
			evaluation->reset();

			if (optimization_tuning.multiobjective)
				evolution = new MOEvolution(*test.architecture,
					*test.graph, *test.scheduler, *evaluation,
					evolution_tuning, constrains);
			else
				evolution = new SOEvolution(*test.architecture,
					*test.graph, *test.scheduler, *evaluation,
					evolution_tuning, constrains);

			clock_t begin = clock();

			BasicEvolutionStats &stats = evolution->solve(
				test.mapping, test.priority);

			clock_t end = clock();
			double elapsed = (double)(end - begin) / CLOCKS_PER_SEC;

			cout << endl << endl << stats << endl << *evaluation << endl;

			cout << "Improvement: " << setiosflags(ios::fixed) << setprecision(2);

			if (!optimization_tuning.multiobjective) {
				SOEvolutionStats *sstats = (SOEvolutionStats *)&stats;

				cout
					<< (sstats->best_lifetime / price.lifetime - 1.0) * 100
					<< "% lifetime with "
					<< (sstats->final_energy / price.energy - 1.0) * 100
					<< "% energy"
					<< endl;
			}
			else {
				MOEvolutionStats *sstats = (MOEvolutionStats *)&stats;

				cout
					<< (sstats->best_lifetime.lifetime / price.lifetime - 1.0) * 100
					<< "% lifetime with "
					<< (sstats->best_lifetime.energy / price.energy - 1.0) * 100
					<< "% energy"
					<< endl;
			}

			if (system_tuning.verbose)
				cout << "Time elapsed: " << setprecision(2)
					<< (elapsed / 60.0) << " minutes" << endl << endl;

			/* Make a back copy of the dump file */
			if (!optimization_tuning.dump.empty() && repeat > 1)
				backup(optimization_tuning.dump, i);

			__DELETE(evolution);
		}
	}
	catch (exception &e) {
		__DELETE(evaluation);
		__DELETE(evolution);
		throw;
	}

	__DELETE(evaluation);
	__DELETE(evolution);
}

int main(int argc, char **argv)
{
	OptimaCommandLine arguments;

	try {
		arguments.parse(argc, (const char **)argv);
		optimize(arguments.system, arguments.floorplan, arguments.hotspot,
			arguments.params, arguments.param_stream);
	}
	catch (exception &e) {
		cerr << e.what() << endl;
		arguments.usage();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
