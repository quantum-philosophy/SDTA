#ifndef __COMMAND_LINE_H__

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <string.h>

class CommandLine
{
	public:

	std::string system;
	std::string floorplan;
	std::string hotspot;
	std::string params;
	std::stringstream param_stream;

	CommandLine() {}
	CommandLine(int argc, const char **argv) { parse(argc, argv); }

	void parse(int argc, const char **argv)
	{
		reset();

		const char *pname, *pvalue;
		size_t i, pair_count = (argc - 1) / 2;
		std::string name;

		for (i = 0; i < pair_count; i++) {
			pname = argv[1 + 2 * i];
			pvalue = argv[1 + 2 * i + 1];

			if (strlen(pname) == 0 || strlen(pvalue) == 0)
				throw std::runtime_error("A wrong length of the input argument.");

			name = std::string(pname + 1);

			if (name == "s" || name == "system")
				system = std::string(pvalue);

			else if (name == "f" || name == "floorplan")
				floorplan = std::string(pvalue);

			else if (name == "h" || name == "hotspot")
				hotspot = std::string(pvalue);

			else if (name == "p" || name == "parameters")
				params = std::string(pvalue);

			else
				param_stream << name << " " << pvalue << std::endl;
		}

		verify();
	}

	static void usage()
	{
		std::cout
			<< "Usage: optima [-<param name> <param value>]" << std::endl
			<< std::endl
			<< "  Available parameters:" << std::endl
			<< "  * s, system      - a task graph with a number of PEs" << std::endl
			<< "  * f, floorplan   - a floorplan for the PEs" << std::endl
			<< "    h, hotspot     - the Hotspot configuration" << std::endl
			<< "    p, parameters  - the tuning parameters" << std::endl
			<< "    other          - overwrite the tuning parameters" << std::endl
			<< std::endl
			<< "  (* required parameters)" << std::endl;
	}

	private:

	void reset()
	{
		system = std::string();
		floorplan = std::string();
		hotspot = std::string();
		params = std::string();
		param_stream.clear();
	}

	void verify() const
	{
		if (system.empty() || !file_exist(system))
			throw std::runtime_error("The system configuration file does not exist.");

		if (floorplan.empty() || !file_exist(floorplan))
			throw std::runtime_error("The floorplan configuration file does not exist.");

		if (!hotspot.empty() && !file_exist(hotspot))
			throw std::runtime_error("The Hotspot configuration file does not exist.");

		if (!params.empty() && !file_exist(params))
			throw std::runtime_error("The tuning configuration file does not exist.");
	}

	inline bool file_exist(const std::string &filename) const
	{
		return std::ifstream(filename.c_str()).is_open();
	}
};

#endif
