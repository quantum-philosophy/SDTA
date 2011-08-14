#ifndef __UTILS_H__
#define __UTILS_H__

#include <mex.h>
#include <hotspot/util.h>

#define STRINGIFY(x) #x
#define STRINGIFY_DEF(x) STRINGIFY(x)

#ifndef HOTSPOT_VERSION
#define VERSION "UNKNOWN"
#else
#define VERSION STRINGIFY_DEF(HOTSPOT_VERSION)
#endif

int parse_structure_config(const mxArray *structure, str_pair **ptable);

void verify_and_fetch_properties(int nrhs, const mxArray *prhs[],
	char **floorplan, char **config);

#endif
