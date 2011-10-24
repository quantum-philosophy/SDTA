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
	 * D = U * L * UT
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

class CoarseCondensedEquation
{
	protected:

	const size_t processor_count;
	const size_t node_count;
	const double ambient_temperature;

	matrix_t D;
	vector_t sinvC;
	matrix_t K;
	matrix_t H;
	matrix_t G;

	/* Eigenvector decomposition
	 *
	 * D = U * L * UT
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

	CoarseCondensedEquation(size_t processor_count, size_t node_count,
		const double **conductivity, const double *capacitance,
		double ambient_temperature);

	void solve(double total_time, const vector_t &time,
		const matrix_t &power, matrix_t &temperature);

	protected:

	void calculate_Q(double t, const double *power, double *Q);
	void calculate_K(double t, double *K);
};

#endif