#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

#include <unistd.h>
#include <sys/times.h>

int obtain_coefficients(char *floorplan, char *config,
	double **negA, double **invC, void *(*alloc)(size_t), void (*dealloc)(void *));

int solve_ssdtc_original(char *floorplan, char *config, double *power,
	int nodes, int steps, double tol, int maxit, double *T, char *dump);

int solve_ssdtc_condensed_equation(char *floorplan, char *config, double *power,
	int cores, int steps, double *T);

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
