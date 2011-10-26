#ifndef __COMMAND_LINE_H__

#include <stdexcept>
#include <iostream>
#include <string>
#include <string.h>

class CommandLine
{
	public:

	CommandLine()
	{
	}

	void parse(int argc, const char **argv)
	{
		const char *name, *value;
		size_t i, pair_count = (argc - 1) / 2;

		for (i = 0; i < pair_count; i++) {
			name = argv[1 + 2 * i];
			value = argv[1 + 2 * i + 1];

			if (strlen(name) == 0 || strlen(value) == 0)
				throw std::runtime_error("A wrong length of the input argument.");

			process(std::string(name + 1), std::string(value));
		}

		verify();
	}

	virtual void usage() const = 0;

	protected:

	virtual void verify() const = 0;
	virtual void process(const std::string &name, const std::string &value) = 0;
};

#endif
