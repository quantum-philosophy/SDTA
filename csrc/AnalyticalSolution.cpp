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

	/* Eigenvalue decomposition:
	 * D = U * L * UT
	 *
	 * Where:
	 * L = diag(l0, ..., l(n-1))
	 */
	EigenvalueDecomposition S(D, U, L);

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

IterativeCondensedEquation::IterativeCondensedEquation(
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

size_t IterativeCondensedEquation::solve(const double *dynamic_power,
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

					error = abs(temperature[k] - tmp);
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

TransientAnalyticalSolution::TransientAnalyticalSolution(
	size_t _processor_count, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	const double **conductivity, const double *capacitance) :

	AnalyticalSolution(_processor_count, _node_count, _sampling_interval,
		_ambient_temperature, conductivity, capacitance)
{
	Q.resize(node_count);
	Q.nullify();
}

void TransientAnalyticalSolution::solve(
	const double *power, double *temperature, size_t step_count)
{
	size_t i, j, k;

	Y.resize(step_count, node_count);

	/* We start from zero temperature, therefore, the first
	 * multiplication by K is zero, hence:
	 *
	 * Y(1) = K * Y(0) + Q(0) = Q(0)
	 *
	 * NOTE: The indexes are shifted by 1 here, because we store
	 * Y(i) in the place of Y(i-1), since we are not interested in
	 * the initial temperature (we know it, it is ambient).
	 */
	multiply_matrix_incomplete_vector(G, power, processor_count, Y[0]);

	for (i = 1; i < step_count; i++) {
		/* Q(i) = G * B(i) */
		multiply_matrix_incomplete_vector(G, power + i * processor_count,
			processor_count, Q);

		/* Y(i) = K * Y(i-1) + Q(i) */
		multiply_matrix_vector_plus_vector(K, Y[i - 1], Q, Y[i]);
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

	/* Eigenvalue decomposition:
	 * D = U * L * UT
	 *
	 * Where:
	 * L = diag(l0, ..., l(n-1))
	 */
	EigenvalueDecomposition S(D, U, L);

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
