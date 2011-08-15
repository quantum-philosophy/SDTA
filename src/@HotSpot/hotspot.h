#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

extern "C" {
#include <hotspot/util.h>
}

int obtain_coefficients(char *floorplan, char *config,
	double **negA, double **invC, void *(*alloc)(size_t), void (*dealloc)(void *));

int solve_original(
	char *floorplan, char *config, str_pair *table, int tsize,
	double *power, int nodes, int steps,
	double tol, int minbad, int maxit, double *T);

int solve_condensed_equation(
	char *floorplan, char *config, str_pair *table, int tsize,
	double *power, int cores, int steps,
	double *T);

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

int solve_condensed_equation_with_leakage(
	/* Configuration of HotSpot */
	char *floorplan, char *config, str_pair *table, int tsize,
	/* Dynamic power */
	int cores, int steps,
	const double *dynamic_power,
	/* Static power */
	const double *vdd, const double *ngate,
	/* Final temperature with error control */
	double *T, double tol, int maxit);

#endif
