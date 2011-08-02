#ifndef __UTILS_H__
#define __UTILS_H__

#include <mex.h>
#include <hotspot/util.h>

#include <unistd.h>
#include <sys/times.h>

#define STRINGIFY(x) #x
#define STRINGIFY_DEF(x) STRINGIFY(x)

#ifndef HOTSPOT_VERSION
#define VERSION "UNKNOWN"
#else
#define VERSION STRINGIFY_DEF(HOTSPOT_VERSION)
#endif

int parse_structure_config(const mxArray *structure, str_pair **ptable);

#define define_timer(name) \
	struct tms __start_ ## name ## _tm; \
	struct tms __stop_ ## name ## _tm; \
	clock_t __start_ ## name; \
	clock_t __stop_ ## name;

#define start_timer(name) \
	__start_ ## name = times(&__start_ ## name ## _tm)

#define stop_timer(name) \
	__stop_ ## name = times(&__stop_ ## name ## _tm)

#define timer_result(name) \
	((double)(__stop_ ## name  - __start_ ## name) / sysconf(_SC_CLK_TCK))

#endif
