#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

extern "C" {
#include <hotspot/util.h>
#include <hotspot/flp.h>
#include <hotspot/temperature.h>
#include <hotspot/temperature_block.h>
}

#define __A			1.1432e-12
#define __B			1.0126e-14
#define __alpha		466.4029
#define __beta		-1224.74083
#define __gamma		6.28153
#define __delta		6.9094
#define __Is		995.7996

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

class Hotspot
{
	thermal_config_t cfg;
	flp_t *flp;

	public:

	Hotspot(char *floorplan, char *config,
		str_pair *extra_table = NULL, int rows = 0);
	~Hotspot();

	unsigned int solve(double *dynamic_power, int steps, const double *vdd,
		const double *ngate, double *T, double tol, int maxit);

	private:

	static void inject_leakage(const double *dynamic_power, const double *vdd,
		const double *ngate, int cores, int steps, const double *T,
		double *total_power);

	/* Initial leakage with the same ambient temperature */
	static void inject_leakage(const double *dynamic_power, const double *vdd,
		const double *ngate, int cores, int steps, double T, double *total_power);
};

#endif
