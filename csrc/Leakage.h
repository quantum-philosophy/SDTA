#ifndef __LEAKAGE_H__
#define __LEAKAGE_H__

#include "common.h"

class Leakage
{
	static const double A = 1.1432e-12;
	static const double B = 1.0126e-14;
	static const double alpha = 466.4029;
	static const double beta = -1224.74083;
	static const double gamma = 6.28153;
	static const double delta = 6.9094;

	/* How to calculate Is?
	 *
	 * Take a look at (all coefficients above are from this paper):
	 * "Temperature and Supply Voltage Aware Performance and Power
	 * Modeling at Microarchitecture Level" (July 2005)
	 *
	 * T = [ 100, 100, 80, 80, 60, 60 ] + 273.15
	 * V = [ 0.95, 1.05, 0.95, 1.05, 0.95, 1.05 ]
	 * Iavg = [ 23.44, 29.56, 19.44, 25.14, 16.00, 21.33 ] * 1e-6
	 * Is = mean(Iavg(i) / favg(T(i), V(i)))
	 *
	 * Where favg is the scaling factor (see calc_scaling).
	 */
	static const double Is = 995.7996;

	const size_t processor_count;
	std::vector<double> voltages;
	std::vector<unsigned long int> ngates;

	public:

	Leakage(const processor_vector_t &processors);

	void inject(size_t step_count, const double *dynamic_power,
		const double *temperature, double *total_power) const;

	void inject(size_t step_count, const double *dynamic_power,
		double temperature, double *total_power) const;
};

#endif
