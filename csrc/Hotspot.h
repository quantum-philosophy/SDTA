#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

extern "C" {
#include <hotspot/util.h>
#include <hotspot/flp.h>
#include <hotspot/temperature.h>
#include <hotspot/temperature_block.h>
}

#include "common.h"

class Hotspot
{
	thermal_config_t cfg;
	flp_t *flp;

	static const double tol = 0.01;
	static const size_t maxit = 10;

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

	public:

	Hotspot(char *floorplan, char *config,
		str_pair *extra_table = NULL, size_t tsize = 0);
	~Hotspot();

	unsigned int solve(const Architecture *architecture,
		const matrix_t &m_dynamic_power, matrix_t &m_temperature,
		matrix_t &m_total_power);

	double sampling_interval() const { return cfg.sampling_intvl; }

	private:

	static void inject_leakage(const Architecture *architecture,
		size_t processor_count, size_t step_count, const double *dynamic_power,
		const double *temperature, double *total_power);

	/* Initial leakage with the ambient temperature */
	static void inject_leakage(const Architecture *architecture,
		size_t processor_count, size_t step_count, const double *dynamic_power,
		double temperature, double *total_power);
};

#endif
