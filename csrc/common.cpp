#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>
#include <string.h>

#include "common.h"

bool Random::verbose = false;
int Random::seed = -1;

system_t::system_t(const std::string &filename)
{
	char c;
	int tmp;
	std::string line, name;
	size_t i, j, rows, cols;

	std::ifstream file(filename.c_str());
	file.exceptions(std::fstream::failbit | std::fstream::badbit);

	if (!file.is_open())
		throw std::runtime_error("Cannot open the system file.");

	type.clear();
	link.clear();

	frequency.clear();
	voltage.clear();
	ngate.clear();
	nc.clear();
	ceff.clear();

	mapping.clear();
	priority.clear();
	deadline = 0;

	while (true) {
		try {
			std::getline(file, line);
		}
		catch (...) {
			break;
		}

		/* Skip empty lines and comments */
		if (line.empty() || line[0] == '#') continue;

		std::stringstream stream(line);
		stream.exceptions(std::ios::failbit | std::ios::badbit);

		stream >> c;
		if (c != '@')
			throw std::runtime_error("Cannot read the start sign for the object.");

		stream >> name;

		stream >> c;
		if (c != '(')
			throw std::runtime_error("Cannot read the dimensions.");

		stream >> rows;
		if (rows <= 0)
			throw std::runtime_error("Cannot read the dimensions.");

		stream >> c;
		if (c != 'x')
			throw std::runtime_error("Cannot read the dimensions.");

		stream >> cols;
		if (cols <= 0)
			throw std::runtime_error("Cannot read the dimensions.");

		stream >> c;
		if (c != ')')
			throw std::runtime_error("Cannot read the dimensions.");

		if (name == "type") {
			if (rows != 1)
				throw std::runtime_error("The type should be a vector.");

			type.resize(cols);
			for (i = 0; i < cols; i++)
				file >> type[i];
		}
		else if (name == "link") {
			if (rows != cols)
				throw std::runtime_error("The link should be a square matrix.");

			link.resize(rows);
			for (i = 0; i < rows; i++) {
				link[i].resize(cols);
				for (j = 0; j < cols; j++) {
					file >> tmp;
					link[i][j] = (bool)tmp;
				}
			}
		}
		else if (name == "frequency") {
			if (rows != 1)
				throw std::runtime_error("The frequency should be a vector.");

			frequency.resize(cols);
			for (i = 0; i < cols; i++)
				file >> frequency[i];
		}
		else if (name == "voltage") {
			if (rows != 1)
				throw std::runtime_error("The voltage should be a vector.");

			voltage.resize(cols);
			for (i = 0; i < cols; i++)
				file >> voltage[i];
		}
		else if (name == "ngate") {
			if (rows != 1)
				throw std::runtime_error("The ngate should be a vector.");

			ngate.resize(cols);
			for (i = 0; i < cols; i++)
				file >> ngate[i];
		}
		else if (name == "nc") {
			nc.resize(cols);
			for (i = 0; i < cols; i++)
				nc[i].resize(rows);
			for (i = 0; i < rows; i++)
				for (j = 0; j < cols; j++)
					file >> nc[j][i];
		}
		else if (name == "ceff") {
			ceff.resize(cols);
			for (i = 0; i < cols; i++)
				ceff[i].resize(rows);
			for (i = 0; i < rows; i++)
				for (j = 0; j < cols; j++)
					file >> ceff[j][i];
		}
		else if (name == "mapping") {
			if (rows != 1)
				throw std::runtime_error("The mapping should be a vector.");

			mapping.resize(cols);
			for (i = 0; i < cols; i++)
				file >> mapping[i];
		}
		else if (name == "priority") {
			if (rows != 1)
				throw std::runtime_error("The priority should be a vector.");

			priority.resize(cols);
			for (i = 0; i < cols; i++)
				file >> priority[i];
		}
		else if (name == "deadline") {
			if (rows != 1 && cols != 1)
				throw std::runtime_error("The deadline should be a scalar.");

			file >> deadline;
		}
		else
			throw std::runtime_error("An unknown variable.");
	}

	size_t task_count = type.size();

	if (!task_count)
		throw std::runtime_error("There are no tasks.");

	if (task_count != link.size() || task_count != link[0].size())
		throw std::runtime_error("The size of the link matrix is wrong.");

	size_t processor_count = frequency.size();

	if (!processor_count)
		throw std::runtime_error("There are no frequencies.");

	if (processor_count != voltage.size())
		throw std::runtime_error("The size of the voltage vector is wrong.");

	if (processor_count != ngate.size())
		throw std::runtime_error("The size of the ngate vector is wrong.");

	size_t type_count;

	if (processor_count != nc.size() || (type_count = nc[0].size()) == 0)
		throw std::runtime_error("The size of the nc matrix is wrong.");

	if (processor_count != ceff.size() || type_count != ceff[0].size())
		throw std::runtime_error("The size of the ceff matrix is wrong.");

	if (!mapping.empty() && mapping.size() != task_count)
		throw std::runtime_error("The size of the mapping vector is wrong.");

	if (!priority.empty() && priority.size() != task_count)
		throw std::runtime_error("The size of the priority vector is wrong.");

	if (deadline < 0)
		throw std::runtime_error("The deadline should not be negative.");
}

std::ostream &operator<< (std::ostream &o, const price_t &price)
{
	o << std::setprecision(2)
		<< "("
			<< price.lifetime << ", "
			<< price.energy
		<< ")";

	return o;
}

std::ostream &operator<< (std::ostream &o, const constrain_t &constrain)
{
	o << std::setprecision(2)
		<< "{"
			<< constrain.min << ", "
			<< constrain.max
		<< "}";

	return o;
}
