#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

#include <hotspot/flp.h>
#include <hotspot/temperature.h>
#include <hotspot/temperature_block.h>

#define E_PROFILE 	-1
#define E_VALUES 	-2
#define E_IO		-3
#define E_MISMATCH	-4
#define E_NAMES		-5
#define KELVIN 273.15

typedef double *HotSpotVector;
typedef double **HotSpotMatrix;
typedef int (*printf_t)(const char *, ...);

#define alloc_hotspot_matrix(rows, cols) (HotSpotMatrix)dmatrix(rows, cols)
#define free_hotspot_matrix(M) free_dmatrix((double **)M)

#define alloc_hotspot_vector(size) (HotSpotVector)dvector(size)
#define free_hotspot_vector(V) free_dvector((double *)V)

int obtain_hotspot_model(char *floorplan, char *config,
	int *nodes, HotSpotMatrix *negA, HotSpotMatrix *invC);

int solve_ssdtc_with_hotspot(char *floorplan, char *config, double *power,
	int nodes, int steps, double tol, int maxit, double *T, char *dump,
	printf_t printf);

int solve_sst_with_hotspot(char *floorplan, char *power, char *config,
	int *cores, HotSpotVector *T);

#endif
