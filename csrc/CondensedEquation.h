#ifndef __CONDENSED_EQUATION_H__
#define __CONDENSED_EQUATION_H__

#include "common.h"
#include "Leakage.h"

class CondensedEquation
{
	protected:

	const size_t processor_count;
	const size_t node_count;

	const double sampling_interval;
	const double ambient_temperature;

	/* Matrix exponent */
	matrix_t K;

	vector_t sinvC;
	matrix_t G;

	/* Eigenvector decomposition
	 *
	 * K = U * L * UT
	 */
	vector_t L;
	matrix_t U;
	matrix_t UT;

	matrix_t P;
	matrix_t Q;
	matrix_t Y;

	matrix_t m_temp;
	vector_t v_temp;

	public:

	CondensedEquation(size_t _processor_count, size_t _node_count,
		double _sampling_interval, double _ambient_temperature,
		const double **conductivity, const double *inv_capacitance);

	/* NOTE: power should be of size (step_count x processor_count) */
	void solve(const double *power, double *temperature, size_t step_count);
};

class LeakageCondensedEquation: public CondensedEquation
{
	static const double tol = 0.01;
	static const size_t maxit = 10;

	Leakage leakage;

	public:

	LeakageCondensedEquation(const processor_vector_t &processors,
		size_t _node_count, double _sampling_interval, double _ambient_temperature,
		const double **conductivity, const double *inv_capacitance);

	/* NOTE: dynamic_power should be of size (step_count x processor_count) */
	size_t solve(const double *dynamic_power,
		double *temperature, double *total_power, size_t step_count);
};

#endif
