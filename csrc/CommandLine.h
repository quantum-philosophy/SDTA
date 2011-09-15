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

	std::string system_config;
	std::string genetic_config;
	std::string floorplan_config;
	std::string thermal_config;
	std::stringstream tuning_stream;

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
				system_config = std::string(pvalue);

			else if (name == "g" || name == "genetic")
				genetic_config = std::string(pvalue);

			else if (name == "f" || name == "floorplan")
				floorplan_config = std::string(pvalue);

			else if (name == "t" || name == "thermal")
				thermal_config = std::string(pvalue);

			else
				tuning_stream << name << " " << pvalue << std::endl;
		}

		verify();
	}

	static void usage()
	{
		std::cout
			<< "Usage: optima [-<param name> <param value>]" << std::endl
			<< std::endl
			<< "  Available parameters:" << std::endl
			<< "  * s, system      - a task graph with a set of PEs (architecture)" << std::endl
			<< "    g, genetic     - the configuration of the GA" << std::endl
			<< "  * f, floorplan   - the floorplan of the architecture" << std::endl
			<< "    t, thermal     - the configuration of Hotspot" << std::endl
			<< "    other          - overwrite the genetic configuration" << std::endl
			<< std::endl
			<< "  (* required parameters)" << std::endl;
	}

	private:

	void reset()
	{
		system_config    = std::string();
		genetic_config   = std::string();
		floorplan_config = std::string();
		system_config    = std::string();
		tuning_stream.clear();
	}

	void verify() const
	{
		if (system_config.empty() || !file_exist(system_config))
			throw std::runtime_error("The system configuration file does not exist.");

		if (floorplan_config.empty() || !file_exist(floorplan_config))
			throw std::runtime_error("The floorplan configuration file does not exist.");

		if (!genetic_config.empty() && !file_exist(genetic_config))
			throw std::runtime_error("The genetic configuration file does not exist.");

		if (!thermal_config.empty() && !file_exist(thermal_config))
			throw std::runtime_error("The thermal configuration file does not exist.");
	}

	inline bool file_exist(const std::string &filename) const
	{
		return std::ifstream(filename.c_str()).is_open();
	}
};

#endif
