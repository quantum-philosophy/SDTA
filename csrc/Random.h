#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <iostream>
#include <ctime>

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <tinymt64.h>
}

class Random
{
	static tinymt64_t tinymt;
	static bool verbose;
	static int seed;

	public:

	static void set_seed(int seed, bool verbose = false)
	{
		Random::seed = seed;
		Random::verbose = verbose;
	}

	static int get_seed()
	{
		int seed = Random::seed;

		if (seed < 0) seed = time(NULL);

		if (verbose)
			std::cout << "Chosen seed: " << seed << std::endl;

		return seed;
	}

	static void reseed()
	{
		tinymt64_init(&tinymt, get_seed());
	}

	static inline double uniform(double range = 1.0)
	{
		return range * tinymt64_generate_double(&tinymt);
	}

	static inline int number(int range)
	{
		return (double)range * uniform();
	}

	static bool flip(double p)
	{
		return uniform() < p;
	}
};

#endif
