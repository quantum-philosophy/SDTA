#include <stdexcept>
#include <iostream>
#include <cmath>

#ifndef PROCESSOR_LABEL
#define PROCESSOR_LABEL "core"
#endif

using namespace std;

void usage();

int main(int argc, char *argv[])
{
	size_t i, cols, processor_count;
	double die_area, processor_width, x, y;

	try {
		if (argc < 2)
			throw runtime_error("Not enough input arguments.");

		processor_count = atoi(argv[1]);

		if (processor_count <= 0)
			throw runtime_error("The number of processors should be positive.");

		if (argc < 3) {
			/* Intel i7 620M processing die size 81mm^2 */
			die_area = 81e-6;
		}
		else {
			die_area = atof(argv[2]);

			if (die_area <= 0)
				throw runtime_error("The die area should be positive.");
		}

		cols = sqrt(processor_count);
		processor_width = sqrt(die_area) / cols;

#ifdef DEBUG
		cout
			<< "Processor count:      " << processor_count << endl
			<< "Processors in row:    " << cols << endl
			<< "Total die area:       " << die_area << " m^2" << endl
			<< "Width of a processor: " << processor_width << " m^2" << endl;
#endif

		for (i = 0; i < processor_count; i++) {
			x = (i % cols) * processor_width;
			y = (int)(i / cols) * processor_width;

			cout
				<< PROCESSOR_LABEL << i + 1 << '\t'
				<< processor_width << '\t'
				<< processor_width << '\t'
				<< x << '\t'
				<< y << endl;
		}
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
		<< "Usage: floorplan <processor count> [<die area>]" << endl
		<< endl
		<< "  * processor count - the number of processing elements on the die" << endl
		<< "    die area        - the die area in square miters" << endl
		<< endl
		<< "  (* required parameters)" << endl;
}
