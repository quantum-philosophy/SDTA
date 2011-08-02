#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

extern "C" {
#include <hotspot/util.h>
}

int obtain_coefficients(char *floorplan, char *config,
	double **negA, double **invC, void *(*alloc)(size_t), void (*dealloc)(void *));

int solve_ssdtc_original(char *floorplan, char *config, double *power,
	int nodes, int steps, double tol, int minbad, int maxit, double *T);

int solve_ssdtc_condensed_equation(char *floorplan, char *config,
	str_pair *table, int tsize, double *power, int cores, int steps, double *T);

#endif
