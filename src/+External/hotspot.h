#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

#include <hotspot/flp.h>
#include <hotspot/temperature.h>
#include <hotspot/temperature_block.h>

#define E_PROFILE 	-1
#define E_NAMES 	-2
#define E_VALUES 	-3
#define E_STEPS 	-4
#define E_IO		-5
#define MAX_POWER_CHUNK 100
#define KELVIN 273.15

typedef double *HotSpotVector;
typedef double **HotSpotMatrix;

#define alloc_hotspot_matrix(rows, cols) (HotSpotMatrix)dmatrix(rows, cols)
#define free_hotspot_matrix(M) free_dmatrix((double **)M)

#define alloc_hotspot_vector(size) (HotSpotVector)dvector(size)
#define free_hotspot_vector(V) free_dvector((double *)V)

int obtain_hotspot_model(char *floorplan, char *config,
	int *nodes, HotSpotMatrix *negA, HotSpotMatrix *invC);

int solve_ssdtc_with_hotspot(char *floorplan, char *power, char *config,
	double tol, int maxit, int *steps, int *cores, HotSpotVector *T,
	char *dump);

int solve_sst_with_hotspot(char *floorplan, char *power, char *config,
	int *cores, HotSpotVector *T);

#endif
