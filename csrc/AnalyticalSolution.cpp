#include "AnalyticalSolution.h"

AnalyticalSolution::AnalyticalSolution(
	size_t _processor_count, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	const double **conductivity, const double *capacitance) :

	processor_count(_processor_count),
	node_count(_node_count),
	sampling_interval(_sampling_interval),
	ambient_temperature(_ambient_temperature)
{
	size_t i, j;

	K.resize(node_count, node_count);
	sinvC.resize(node_count);

	L.resize(node_count);
	U.resize(node_count, node_count);
	UT.resize(node_count, node_count);

	G.resize(node_count, node_count);

	m_temp.resize(node_count, node_count);
	v_temp.resize(node_count);

	/* We have:
	 * C * dT/dt = A * T + B
	 */
	matrix_t &A = K;

	for (i = 0; i < node_count; i++) {
		for (j = 0; j < node_count; j++)
			A[i][j] = -conductivity[i][j];
		sinvC[i] = sqrt(1.0 / capacitance[i]);
	}

	matrix_t &D = A;

	/* We want to get rid of everything in front of dX/dt,
	 * but at the same time we want to keep the matrix in front of X
	 * symmetric, so we do the following substitution:
	 * Y = C^(1/2) * T
	 * D = C^(-1/2) * A * C^(-1/2)
	 * E = C^(-1/2) * B
	 *
	 * Eventually, we have:
	 * dY/dt = DY + E
	 */
	multiply_diagonal_matrix_matrix(sinvC, A, m_temp);
	multiply_matrix_diagonal_matrix(m_temp, sinvC, D);

#ifdef MEASURE_TIME
	struct timespec begin, end;
	Time::measure(&begin);
#endif

	/* Eigenvalue decomposition:
	 * D = U * L * UT
	 *
	 * Where:
	 * L = diag(l0, ..., l(n-1))
	 */
	EigenvalueDecomposition S(D, U, L);

#ifdef MEASURE_TIME
	Time::measure(&end);
	decomposition_time = Time::substract(&end, &begin);
#endif

	transpose_matrix(U, UT);

	/* Matrix exponential:
	 * K = exp(D * t) = U * exp(L * t) * UT
	 */
	for (i = 0; i < node_count; i++) v_temp[i] = exp(sampling_interval * L[i]);
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix(m_temp, UT, K);

	/* Coefficient matrix G:
	 * G = D^(-1) * (exp(D * t) - I) * C^(-1/2) =
	 * = U * diag((exp(t * l0) - 1) / l0, ...) * UT * C^(-1/2)
	 */
	for (i = 0; i < node_count; i++) v_temp[i] = (v_temp[i] - 1) / L[i];
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_diagonal_matrix(m_temp, UT, sinvC, G);
}

/******************************************************************************/

CondensedEquation::CondensedEquation(size_t _processor_count, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	const double **conductivity, const double *capacitance) :

	AnalyticalSolution(_processor_count, _node_count, _sampling_interval,
		_ambient_temperature, conductivity, capacitance)
{
}

void CondensedEquation::solve(const double *power, double *temperature,
	size_t step_count)
{
	size_t i, j, k;

	P.resize(step_count, node_count);
	Q.resize(step_count, node_count);
	Y.resize(step_count, node_count);

	Q.nullify();

	double total_time = sampling_interval * step_count;

	/* M = diag(1/(1 - exp(Tau * l0)), ...) */
	for (i = 0; i < node_count; i++)
		v_temp[i] = 1.0 / (1.0 - exp(total_time * L[i]));

	/* Q(0) = G * B(0) */
	multiply_matrix_incomplete_vector(G, power, processor_count, Q[0]);
	/* P(0) = Q(0) */
	__MEMCPY(P[0], Q[0], node_count);

	for (i = 1; i < step_count; i++) {
		/* Q(i) = G * B(i) */
		multiply_matrix_incomplete_vector(G, power + i * processor_count,
			processor_count, Q[i]);
		/* P(i) = K * P(i-1) + Q(i) */
		multiply_matrix_vector_plus_vector(K, P[i - 1], Q[i], P[i]);
	}

	/* Y(0) = U * M * UT * P(m-1), for M see above ^ */
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_vector(m_temp, UT, P[step_count - 1], Y[0]);

	/* Y(i+1) = K * Y(i) + Q(i) */
	for (i = 1; i < step_count; i++)
		multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i - 1], Y[i]);

	/* Return back to T from Y:
	 * T = C^(-1/2) * Y
	 *
	 * And do not forget about the ambient temperature.
	 */
	for (i = 0, k = 0; i < step_count; i++)
		for (j = 0; j < processor_count; j++, k++)
			temperature[k] = Y[i][j] * sinvC[j] + ambient_temperature;
}

/******************************************************************************/

LeakageCondensedEquation::LeakageCondensedEquation(
	size_t _processor_count, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	double **conductivity, const double *capacitance,
	const Leakage &_leakage) :

	CondensedEquation(_processor_count, _node_count, _sampling_interval,
		_ambient_temperature,
		(const double **)_leakage.setup(conductivity, _ambient_temperature),
		capacitance), leakage(_leakage)
{
}

size_t LeakageCondensedEquation::solve(const double *dynamic_power,
	double *temperature, double *total_power, size_t step_count)
{
	size_t i, j, k, it;
	double tmp, error, max_error;

	P.resize(step_count, node_count);
	Q.resize(step_count, node_count);
	Y.resize(step_count, node_count);

	Q.nullify();

	double total_time = sampling_interval * step_count;

	/* M = diag(1/(1 - exp(Tau * l0)), ...) */
	for (i = 0; i < node_count; i++)
		v_temp[i] = 1.0 / (1.0 - exp(total_time * L[i]));

	const size_t max_iterations = leakage.get_max_iterations();
	const double tolerance = leakage.get_tolerance();

	leakage.inject(ambient_temperature, dynamic_power, total_power, step_count);

	/* We come to the iterative part */
	for (it = 1;; it++) {
		/* Q(0) = G * B(0) */
		multiply_matrix_incomplete_vector(G, total_power, processor_count, Q[0]);
		/* P(0) = Q(0) */
		__MEMCPY(P[0], Q[0], node_count);

		for (i = 1; i < step_count; i++) {
			/* Q(i) = G * B(i) */
			multiply_matrix_incomplete_vector(G, total_power + i * processor_count,
				processor_count, Q[i]);
			/* P(i) = K * P(i-1) + Q(i) */
			multiply_matrix_vector_plus_vector(K, P[i - 1], Q[i], P[i]);
		}

		/* Y(0) = U * M * UT * P(m-1), for M see above ^ */
		multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
		multiply_matrix_matrix_vector(m_temp, UT, P[step_count - 1], Y[0]);

		/* Y(i+1) = K * Y(i) + Q(i) */
		for (i = 1; i < step_count; i++)
			multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i - 1], Y[i]);

		/* Return back to T from Y:
		 * T = C^(-1/2) * Y
		 *
		 * And do not forget about the ambient temperature. Also perform
		 * the error control.
		 */
		if (it < max_iterations) {
			/* There is a reason to check the error.
			 */
			max_error = 0;
			for (i = 0, k = 0; i < step_count; i++)
				for (j = 0; j < processor_count; j++, k++) {
					tmp = Y[i][j] * sinvC[j] + ambient_temperature;

					error = std::abs(temperature[k] - tmp);
					if (max_error < error) max_error = error;

					temperature[k] = tmp;
				}

			/* Still have some iterations left,
			 * the only question is the error.
			 */
			if (max_error < tolerance) break;
		}
		else {
			for (i = 0, k = 0; i < step_count; i++)
				for (j = 0; j < processor_count; j++, k++)
					temperature[k] = Y[i][j] * sinvC[j] + ambient_temperature;

			/* Limit of iterations is reached,
			 * quite right now.
			 */
			break;
		}

		leakage.inject(temperature, dynamic_power, total_power, step_count);
	}

	leakage.finalize(temperature, dynamic_power, total_power, step_count);

	return it;
}

/******************************************************************************/

BasicSteadyStateAnalyticalSolution::BasicSteadyStateAnalyticalSolution(
	size_t _processor_count, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	const double **conductivity, const double *capacitance) :

	AnalyticalSolution(_processor_count, _node_count, _sampling_interval,
		_ambient_temperature, conductivity, capacitance)
{
	R.resize(node_count, node_count);

	/* Solve:
	 * G * T = P
	 * C^(-1/2) * G * C^(-1/2) * C^(1/2) * T = C^(-1/2) * P
	 * Y = - U * L^(-1) * U^T * C^(-1/2) * P = R * P
	 *
	 * NOTE: Minus here because the eigenvalue decomposition is
	 * for the negative matrix.
	 */

	matrix_t m_temp2(node_count, node_count);

	for (size_t j = 0; j < node_count; j++) v_temp[j] = - 1.0 / L[j];
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix(m_temp, UT, m_temp2);
	multiply_matrix_diagonal_matrix(m_temp2, sinvC, R);
}

/******************************************************************************/

SteadyStateAnalyticalSolution::SteadyStateAnalyticalSolution(
	size_t _processor_count, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	const double **conductivity, const double *capacitance) :

	BasicSteadyStateAnalyticalSolution(
		_processor_count, _node_count, _sampling_interval,
		_ambient_temperature, conductivity, capacitance)
{
}

void SteadyStateAnalyticalSolution::solve(
	const double *power, double *temperature, size_t step_count)
{
	size_t i, j, k;

	for (i = 0, k = 0; i < step_count; i++) {
		multiply_matrix_incomplete_vector(
			R, power + i * processor_count, processor_count, v_temp);

		for (j = 0; j < processor_count; j++, k++)
			temperature[k] = v_temp[j] * sinvC[j] + ambient_temperature;
	}
}

/******************************************************************************/

LeakageSteadyStateAnalyticalSolution::LeakageSteadyStateAnalyticalSolution(
	size_t _processor_count, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	double **conductivity, const double *capacitance,
	const Leakage &_leakage) :

	BasicSteadyStateAnalyticalSolution(_processor_count, _node_count,
		_sampling_interval, _ambient_temperature,
		(const double **)_leakage.setup(conductivity, _ambient_temperature),
		capacitance), leakage(_leakage)
{
}

size_t LeakageSteadyStateAnalyticalSolution::solve(const double *dynamic_power,
	double *temperature, double *total_power, size_t step_count)
{
	size_t iterations, i, j, k;
	double tmp, error, max_error;

	const size_t max_iterations = leakage.get_max_iterations();
	const double tolerance = leakage.get_tolerance();

	leakage.inject(ambient_temperature, dynamic_power, total_power, step_count);

	for (iterations = 0; iterations < max_iterations; iterations++) {
		max_error = 0;
		for (i = 0, k = 0; i < step_count; i++) {
			multiply_matrix_incomplete_vector(
				R, total_power + i * processor_count, processor_count, v_temp);

			for (j = 0; j < processor_count; j++, k++) {
				tmp = v_temp[j] * sinvC[j] + ambient_temperature;

				error = std::abs(temperature[k] - tmp);
				if (max_error < error) max_error = error;

				temperature[k] = tmp;
			}
		}

		if (max_error < tolerance) {
			iterations++;
			break;
		}

		leakage.inject(temperature, dynamic_power, total_power, step_count);
	}

	leakage.finalize(temperature, dynamic_power, total_power, step_count);

	return iterations;
}

/******************************************************************************/

TransientAnalyticalSolution::TransientAnalyticalSolution(
	size_t _processor_count, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	const double **conductivity, const double *capacitance,
	size_t _max_iterations, double _tolerance, bool _warmup) :

	AnalyticalSolution(_processor_count, _node_count, _sampling_interval,
		_ambient_temperature, conductivity, capacitance),
	max_iterations(_max_iterations), tolerance(_tolerance), warmup(_warmup)
{
}

void TransientAnalyticalSolution::solve_fixed_iterations(
	const double *power, double *temperature, size_t step_count)
{
	size_t iterations, i, j, k;

	initialize(power, step_count);

	for (i = 1; i < step_count; i++) {
		/* Y(i) = K * Y(i-1) + Q(i)
		 *
		 * Note: Indexes are shifted here for Q.
		 */
		multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i - 1], Y[i]);
	}

	for (iterations = 1; iterations < max_iterations; iterations++) {
		/* Wrap around:
		 *
		 * Y(1) = K * Y(N_s - 1) + Q(N_s - 1)
		 */
		multiply_matrix_vector_plus_vector(
			K, Y[step_count - 1], Q[step_count - 1], Y[0]);

		for (i = 1; i < step_count; i++) {
			/* Y(i) = K * Y(i-1) + Q(i) */
			multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i - 1], Y[i]);
		}
	}

	/* Return back to T from Y:
	 * T = C^(-1/2) * Y
	 *
	 * And do not forget about the ambient temperature.
	 */
	for (i = 0, k = 0; i < step_count; i++)
		for (j = 0; j < processor_count; j++, k++)
			temperature[k] = Y[i][j] * sinvC[j] + ambient_temperature;
}

void TransientAnalyticalSolution::solve_error_control(
	const double *power, double *temperature, size_t step_count)
{
	size_t iterations, i, j, k;
	double tmp, error, max_error;

	initialize(power, step_count);

	for (i = 1; i < step_count; i++) {
		multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i], Y[i]);
	}

	for (i = 0, k = 0; i < step_count; i++)
		for (j = 0; j < processor_count; j++, k++)
			temperature[k] = Y[i][j] * sinvC[j] + ambient_temperature;

	for (iterations = 1; iterations < max_iterations; iterations++) {
		multiply_matrix_vector_plus_vector(K, Y[step_count - 1], Q[0], Y[0]);

		for (i = 1; i < step_count; i++) {
			multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i], Y[i]);
		}

		max_error = 0;
		for (i = 0, k = 0; i < step_count; i++)
			for (j = 0; j < processor_count; j++, k++) {
				tmp = Y[i][j] * sinvC[j] + ambient_temperature;

				error = std::abs(temperature[k] - tmp);
				if (max_error < error) max_error = error;

				temperature[k] = tmp;
			}

		if (max_error < tolerance) break;
	}
}

void TransientAnalyticalSolution::initialize(const double *power, size_t step_count)
{
	size_t i, j;

	Y.resize(step_count, node_count);
	Q.resize(step_count, node_count);

	for (i = 0; i < step_count; i++) {
		/* Q(i) = G * B(i) */
		multiply_matrix_incomplete_vector(G, power + i * processor_count,
			processor_count, Q[i]);
	}

	if (warmup) {
		/* Solve:
		 * G * T = P
		 * C^(-1/2) * G * C^(-1/2) * C^(1/2) * T = C^(-1/2) * P
		 * Y = - U * L^(-1) * U^T * C^(-1/2) * P
		 *
		 * NOTE: Minus here because the eigenvalue decomposition is
		 * for the negative matrix.
		 */

		/* C^(-1/2) * P (average power) */
		v_temp.nullify();
		for (i = 0; i < processor_count; i++) {
			for (j = 0; j < step_count; j++)
				v_temp[i] = v_temp[i] + power[j * processor_count + i];
			v_temp[i] = sinvC[i] * v_temp[i] / double(step_count);
		}

		/* U^T * C^(-1/2) * P */
		multiply_matrix_incomplete_vector(UT, v_temp, processor_count, Y[0]);

		/* L^(-1) * U^T * C^(-1/2) * P */
		for (i = 0; i < node_count; i++) v_temp[i] = - Y[0][i] / L[i];

		/* U * L^(-1) * U^T * C^(-1/2) * P */
		multiply_matrix_vector(U, v_temp, Y[0]);
	}
	else {
		/* We start from zero temperature, therefore, the first
		 * multiplication by K is zero, hence:
		 *
		 * Y(1) = K * Y(0) + Q(0) = Q(0)
		 */
		__MEMCPY(Y[0], Q[0], node_count);
	}
}

/******************************************************************************/

CoarseCondensedEquation::CoarseCondensedEquation(
	size_t _processor_count, size_t _node_count,
	const double **conductivity, const double *capacitance,
	double _ambient_temperature) :

	processor_count(_processor_count),
	node_count(_node_count),
	ambient_temperature(_ambient_temperature)
{
	size_t i, j;

	D.resize(node_count, node_count);
	sinvC.resize(node_count);
	K.resize(node_count, node_count);

	L.resize(node_count);
	U.resize(node_count, node_count);
	UT.resize(node_count, node_count);

	H.resize(node_count, node_count);
	G.resize(node_count, node_count);

	m_temp.resize(node_count, node_count);
	v_temp.resize(node_count);

	/* We have:
	 * C * dT/dt = A * T + B
	 */
	matrix_t &A = D;

	for (i = 0; i < node_count; i++) {
		for (j = 0; j < node_count; j++)
			A[i][j] = -conductivity[i][j];
		sinvC[i] = sqrt(1.0 / capacitance[i]);
	}

	/* We want to get rid of everything in front of dX/dt,
	 * but at the same time we want to keep the matrix in front of X
	 * symmetric, so we do the following substitution:
	 * Y = C^(1/2) * T
	 * D = C^(-1/2) * A * C^(-1/2)
	 * E = C^(-1/2) * B
	 *
	 * Eventually, we have:
	 * dY/dt = DY + E
	 */
	multiply_diagonal_matrix_matrix(sinvC, A, m_temp);
	multiply_matrix_diagonal_matrix(m_temp, sinvC, D);

#ifdef MEASURE_TIME
	struct timespec begin, end;
	Time::measure(&begin);
#endif

	/* Eigenvalue decomposition:
	 * D = U * L * UT
	 *
	 * Where:
	 * L = diag(l0, ..., l(n-1))
	 */
	EigenvalueDecomposition S(D, U, L);

#ifdef MEASURE_TIME
	Time::measure(&end);
	decomposition_time = Time::substract(&end, &begin);
#endif

	transpose_matrix(U, UT);

	/* Coefficient matrix G:
	 * G = D^(-1) * (exp(D * t) - I) * C^(-1/2) =
	 * = U * diag((exp(t * l0) - 1) / l0, ...) * UT * C^(-1/2) =
	 * = U * diag((exp(t * l0) - 1) / l0, ...) * H
	 *
	 * H = UT * C^(-1/2)
	 */
	multiply_matrix_diagonal_matrix(UT, sinvC, H);
}

void CoarseCondensedEquation::solve(double total_time,
	const vector_t &time, const matrix_t &_power, matrix_t &_temperature)
{
	size_t step_count = time.size();
	_temperature.resize(_power);

	const double *power = _power;
	double *temperature = _temperature;

	size_t i, j, k;

	K.resize(step_count, node_count * node_count);
	P.resize(step_count, node_count);
	Q.resize(step_count, node_count);
	Y.resize(step_count, node_count);

	Q.nullify();

	/* Q(0) = G(0) * B(0) =
	 * = U * diag(0) * UT * C^(-1/2) * B(0) =
	 * = U * diag(0) * H * B(0)
	 */
	calculate_Q(time[0], power, Q[0]);

	/* P(0) = Q(0) */
	__MEMCPY(P[0], Q[0], node_count);

	calculate_K(time[0], K[0]);

	for (i = 1; i < step_count; i++) {
		/* Q(i) = G(i) * B(i) */
		calculate_Q(time[i], power + i * processor_count, Q[i]);

		calculate_K(time[i], K[i]);

		/* P(i) = K(i) * P(i-1) + Q(i) */
		multiply_matrix_vector_plus_vector(
			node_count, K[i], P[i - 1], Q[i], P[i]);
	}

	/* M = diag(1/(1 - exp(Tau * l0)), ...) */
	for (i = 0; i < node_count; i++)
		v_temp[i] = 1.0 / (1.0 - exp(total_time * L[i]));

	/* Y(0) = U * M * UT * P(m-1), for M see above ^ */
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_vector(m_temp, UT, P[step_count - 1], Y[0]);

	/* Y(i+1) = K(i) * Y(i) + Q(i) */
	for (i = 1; i < step_count; i++)
		multiply_matrix_vector_plus_vector(
			node_count, K[i - 1], Y[i - 1], Q[i - 1], Y[i]);

	/* Return back to T from Y:
	 * T = C^(-1/2) * Y
	 *
	 * And do not forget about the ambient temperature.
	 */
	for (i = 0, k = 0; i < step_count; i++)
		for (j = 0; j < processor_count; j++, k++)
			temperature[k] = Y[i][j] * sinvC[j] + ambient_temperature;
}

void CoarseCondensedEquation::calculate_Q(
	double t, const double *P, double *Q)
{
	multiply_matrix_incomplete_vector(H, P, processor_count, v_temp);

	for (size_t i = 0; i < node_count; i++)
		v_temp[i] = ((exp(t * L[i]) - 1) / L[i]) * v_temp[i];

	multiply_matrix_vector(U, v_temp, Q);
}

void CoarseCondensedEquation::calculate_K(double t, double *K)
{
	/* Matrix exponential:
	 * K = exp(D * t) = U * exp(L * t) * UT
	 */
	for (size_t i = 0; i < node_count; i++)
		v_temp[i] = exp(t * L[i]);

	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix(m_temp, UT, K);
}
