#include <stdexcept>
#include <iostream>
#include <cmath>
#include <stdlib.h>

#ifndef PROCESSOR_LABEL
#define PROCESSOR_LABEL "core"
#endif

using namespace std;

void usage();

int main(int argc, char *argv[])
{
	size_t i, cols, processor_count;
	double processor_area, processor_width, x, y;

	try {
		if (argc < 2)
			throw runtime_error("Not enough input arguments.");

		processor_count = atoi(argv[1]);

		if (processor_count <= 0)
			throw runtime_error("The number of processors should be positive.");

		if (argc < 3) {
			processor_area = 4e-6; /* m^2 */
		}
		else {
			processor_area = atof(argv[2]);

			if (processor_area <= 0)
				throw runtime_error("The core area should be positive.");
		}

		processor_width = sqrt(processor_area);
		cols = ceil(sqrt(processor_count));

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
		<< "Usage: floorplan <processor count> [<processor area>]" << endl
		<< endl
		<< "  * processor count - the number of cores on the die" << endl
		<< "    processor area  - the area of a single core in square miters (4 mm^2 by default)" << endl
		<< endl
		<< "  (* required parameters)" << endl;
}
