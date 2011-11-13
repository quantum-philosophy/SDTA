#ifndef __ANALYTICAL_SOLUTION_H__
#define __ANALYTICAL_SOLUTION_H__

#include "common.h"
#include "Leakage.h"

#ifdef MEASURE_TIME
#include "Helper.h"
#endif

class AnalyticalSolution
{
#ifdef MEASURE_TIME
	public:

	double decomposition_time;
#endif

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

	matrix_t m_temp;
	vector_t v_temp;

	public:

	AnalyticalSolution(size_t _processor_count, size_t _node_count,
		double _sampling_interval, double _ambient_temperature,
		const double **conductivity, const double *capacitance);
};

class CondensedEquation: public AnalyticalSolution
{
	protected:

	matrix_t P;
	matrix_t Q;
	matrix_t Y;

	public:

	CondensedEquation(size_t _processor_count, size_t _node_count,
		double _sampling_interval, double _ambient_temperature,
		const double **conductivity, const double *capacitance);

	/* NOTE: power should be of size (step_count x processor_count) */
	void solve(const double *power, double *temperature, size_t step_count);
};

class LeakageCondensedEquation: public CondensedEquation
{
	const Leakage &leakage;

	public:

	LeakageCondensedEquation(size_t _processor_count, size_t _node_count,
		double _sampling_interval, double _ambient_temperature,
		double **conductivity, const double *capacitance,
		const Leakage &_leakage);

	/* NOTE: dynamic_power should be of size (step_count x processor_count) */
	size_t solve(const double *dynamic_power,
		double *temperature, double *total_power, size_t step_count);
};

class SteadyStateAnalyticalSolution: public AnalyticalSolution
{
	public:

	SteadyStateAnalyticalSolution(size_t _processor_count, size_t _node_count,
		double _sampling_interval, double _ambient_temperature,
		const double **conductivity, const double *capacitance);

	void solve(const double *power, double *temperature, size_t step_count = 1);
};

class LeakageSteadyStateAnalyticalSolution: public AnalyticalSolution
{
	const Leakage &leakage;

	public:

	LeakageSteadyStateAnalyticalSolution(
		size_t _processor_count, size_t _node_count,
		double _sampling_interval, double _ambient_temperature,
		double **conductivity, const double *capacitance,
		const Leakage &_leakage);

	size_t solve(const double *dynamic_power,
		double *temperature, double *total_power, size_t step_count = 1);
};

class TransientAnalyticalSolution: public AnalyticalSolution
{
	const size_t max_iterations;
	const double tolerance;
	const bool warmup;

	matrix_t Q;
	matrix_t Y;

	public:

	TransientAnalyticalSolution(size_t _processor_count, size_t _node_count,
		double _sampling_interval, double _ambient_temperature,
		const double **conductivity, const double *capacitance,
		size_t _max_iterations, double _tolerance, bool _warmup);

	inline void solve(const double *power, double *temperature, size_t step_count)
	{
		if (tolerance == 0)
			solve_fixed_iterations(power, temperature, step_count);
		else
			solve_error_control(power, temperature, step_count);
	}

	private:

	void solve_fixed_iterations(
		const double *power, double *temperature, size_t step_count);
	void solve_error_control(
		const double *power, double *temperature, size_t step_count);
	void initialize(const double *power, size_t step_count);
};

class CoarseCondensedEquation
{
#ifdef MEASURE_TIME
	public:

	double decomposition_time;
#endif

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
