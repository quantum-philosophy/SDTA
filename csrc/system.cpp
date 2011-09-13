#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cmath>
#include <vector>
#include <map>
#include <stdlib.h>

#ifndef GRAPH_LABEL
#define GRAPH_LABEL "TASK_GRAPH"
#endif

#ifndef PROCESSOR_LABEL
#define PROCESSOR_LABEL "PE"
#endif

#define LOOK_FOR_ID     0
#define PARSE_GRAPH     1
#define PARSE_PROCESSOR 2
#define PARSE_TYPES     3

using namespace std;

void usage();

void parse(istream &in, ostream &out);

int main(int argc, char *argv[])
{
	try {
		if (argc < 2)
			throw runtime_error("Not enough input arguments.");

		ifstream stream(argv[1]);

		parse(stream, cout);

		stream.close();
	}
	catch (exception &e) {
		cerr << e.what() << endl;
		usage();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void usage()
{
	cout
		<< "Usage: system <TGFF output>" << endl
		<< endl
		<< "  * TGFF output - the result produced by TGFF" << endl
		<< endl
		<< "  (* required parameters)" << endl
		<< endl
		<< "  NOTE: '" GRAPH_LABEL "' marks a graph (only one is supported); '" PROCESSOR_LABEL "' marks a processor;" << endl
		<< "  each processor has 'frequency', 'voltage', and 'ngate' in this particular order;" << endl
		<< "  each type has 'version' (unused), 'effective_switched_capacitance', and" << endl
		<< "  'number_of_clock_cycles'." << endl;
}

void chomp(string &line)
{
	size_t found;

	found = line.find_first_of("#");
	if (found != string::npos) line.erase(found);

	string white_spaces(" \t\f\v\n\r");

	found = line.find_first_not_of(white_spaces);
	if (found != string::npos) line = line.substr(found);

	found = line.find_last_not_of(white_spaces);
	if (found != string::npos) line.erase(found + 1);
	else line.clear();
}

template <class T>
T string2(const string &line)
{
	T t;

	stringstream stream(line);

	stream >> t;

	return t;
}

template <class T>
void print_vector(const char *name, const vector<T> &vector, ostream &out)
{
	size_t size = vector.size();

	out << "@" << name << " (1 x " << size << ")" << endl;

	for (size_t i = 0; i < size; i++) {
		out << vector[i];
		if (i < (size - 1)) out << '\t';
	}

	out << endl;
}

template <class T>
void print_matrix(const char *name, const vector<vector<T> > &matrix, ostream &out)
{
	size_t rows = matrix.size();

	if (rows == 0)
		throw runtime_error("The matrix is empty.");

	size_t cols = matrix[0].size();

	out << "@" << name << " (" << rows << " x " << cols << ")" << endl;

	for (size_t i = 0; i < rows; i++) {
		if (matrix[i].size() != cols)
			throw runtime_error("The matrix is not consistent.");

		for (size_t j = 0; j < cols; j++) {
			out << matrix[i][j];
			if (j < (cols - 1)) out << '\t';
		}

		out << endl;
	}
}

void parse(istream &in, ostream &out)
{
	in.exceptions(ios::failbit | ios::badbit);

	char char_token;
	int state, int_token;
	double real_token;
	string line, name, token;

	/* Graphs */
	size_t graph_count;

	/* Tasks */
	size_t task_count;
	map<string, int> task_map;
	vector<int> type;
	vector<vector<bool> > link;

	/* Processors */
	size_t processor_count;
	vector<double> frequency;
	vector<double> voltage;
	vector<unsigned long int> ngate;

	/* Types */
	size_t type_count;
	vector<vector<double> > ceff;
	vector<vector<unsigned long int> > nc;

	graph_count = 0;

	task_count = 0;
	task_map.clear();
	type.clear();
	link.clear();

	processor_count = 0;
	frequency.clear();
	voltage.clear();
	ngate.clear();
	ceff.clear();
	nc.clear();

	state = LOOK_FOR_ID;

	while (true) {
		try {
			getline(in, line);
		}
		catch (...) {
			break;
		}

		chomp(line);

		if (line.empty()) continue;

		stringstream stream(line);

		switch (state) {
		case LOOK_FOR_ID:
			stream >> char_token;

			if (char_token != '@') continue;
			stream >> name;

			if (name == GRAPH_LABEL) {
				graph_count++;

				if (graph_count > 1)
					throw runtime_error("Only one task graph is supported.");

				state = PARSE_GRAPH;
			}
			else if (name == PROCESSOR_LABEL) {
				processor_count++;
				state = PARSE_PROCESSOR;
			}

			break;

		case PARSE_GRAPH:
			stream >> token;

			if (token == "TASK") {
				/* First, read the name */
				stream >> name;

				stream >> token;
				if (token != "TYPE")
					throw runtime_error("Cannot find 'TYPE'.");

				stream >> int_token;

				task_map[name] = task_count;
				type.push_back(int_token);

				/* Quite stupid, but why not? =) */
				for (size_t i = 0; i < task_count; i++)
					link[i].push_back(false);
				task_count++;
				link.push_back(vector<bool>(task_count, false));
			}
			else if (token == "ARC") {
				/* Skip the name of the arc */
				stream >> name;

				stream >> token;
				if (token != "FROM")
					throw runtime_error("Cannot find 'FROM'.");

				/* The first name */
				stream >> name;

				stream >> token;
				if (token != "TO")
					throw runtime_error("Cannot find 'TO'.");

				/* The second name */
				stream >> token;

				if (task_map.count(name) == 0 ||
					task_map.count(token) == 0)
					throw runtime_error("Do not know such task.");

				link[task_map[name]][task_map[token]] = true;
			}
			else if (token == "}") {
				state = LOOK_FOR_ID;
			}

			break;

		case PARSE_PROCESSOR:
			stream >> real_token;
			frequency.push_back(real_token);

			stream >> real_token;
			voltage.push_back(real_token);

			stream >> real_token;
			ngate.push_back(real_token);

			type_count = 0;

			state = PARSE_TYPES;

			break;

		case PARSE_TYPES:
			stream >> token;

			if (token == "}") {
				state = LOOK_FOR_ID;
				continue;
			}

			int_token = string2<int>(token);

			if (int_token != type_count)
				throw runtime_error("We have missed a type.");

			type_count++;

			if (processor_count == 1) {
				ceff.push_back(vector<double>());
				nc.push_back(vector<unsigned long int>());
			}
			else if (ceff.size() < type_count || nc.size() < type_count)
				throw runtime_error("The processors have different number of types.");

			/* Skip the version */
			stream >> int_token;

			stream >> real_token;
			ceff[type_count - 1].push_back(real_token);

			stream >> real_token;
			nc[type_count - 1].push_back(real_token);

			break;

		default:
			throw runtime_error("An unknown state.");
		}
	}

	print_vector<int>("type", type, out);
	out << endl;
	print_matrix<bool>("link", link, out);
	out << endl;

	print_vector<double>("frequency", frequency, out);
	out << endl;
	print_vector<double>("voltage", voltage, out);
	out << endl;
	print_vector<unsigned long int>("ngate", ngate, out);
	out << endl;

	print_matrix<double>("ceff", ceff, out);
	out << endl;
	print_matrix<unsigned long int>("nc", nc, out);
}
