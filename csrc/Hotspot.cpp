#include "Processor.h"
#include "Architecture.h"

#include <stdexcept>
#include <string>
#include <nr3.h>
#include <eigen_sym.h>

#include "utils.h"
#include "Hotspot.h"

Hotspot::Hotspot(const std::string &floorplan, const std::string &config,
	str_pair *extra_table, size_t tsize)
{
	cfg = default_thermal_config();

	if (!config.empty()) {
		str_pair table[MAX_ENTRIES];
		int i = read_str_pairs(&table[0], MAX_ENTRIES,
			const_cast<char *>(config.c_str()));
		thermal_config_add_from_strs(&cfg, table, i);
	}

	if (extra_table && tsize)
		thermal_config_add_from_strs(&cfg, extra_table, tsize);

	flp = read_flp(const_cast<char *>(floorplan.c_str()), FALSE);
}

Hotspot::~Hotspot()
{
	free_flp(flp, FALSE);
}

void Hotspot::calc_coefficients(matrix_t &neg_a, vector_t &inv_c)
{
	size_t i, j;
	size_t node_count;

	RC_model_t *model;
	block_model_t *block;

	model = alloc_RC_model(&cfg, flp);
	block = model->block;

	populate_R_model(model, flp);
	populate_C_model(model, flp);

	node_count = model->block->n_nodes;

	neg_a.resize(node_count, node_count);
	inv_c.resize(node_count);

	for (i = 0; i < node_count; i++) {
		for (j = 0; j < node_count; j++)
			neg_a[i][j] = block->b[i][j];
		inv_c[i] = block->inva[i];
	}

	delete_RC_model(model);
}

void Hotspot::solve(const matrix_t &m_power, matrix_t &m_temperature)
{
	size_t i, j, k, it;
	size_t processor_count, node_count, step_count, total;
	double ts, am, tmp;

	RC_model_t *model;
	block_model_t *block;

	processor_count = flp->n_units;
	step_count = m_power.rows();
	total = processor_count * step_count;

	if (processor_count != m_power.cols())
		throw std::runtime_error("The floorplan does not match the given power.");

	model = alloc_RC_model(&cfg, flp);
	block = model->block;

	populate_R_model(model, flp);
	populate_C_model(model, flp);

	node_count = model->block->n_nodes;

	m_temperature.resize(m_power);

	double *temperature = m_temperature.pointer();

	ts = model->config->sampling_intvl;
	am = model->config->ambient;

	/* We have:
	 * C * dT/dt = A * T + B
	 */
	MatDoub A(node_count, node_count);
	VecDoub sinvC(node_count);

	for (i = 0; i < node_count; i++) {
		for (j = 0; j < node_count; j++)
			A[i][j] = -block->b[i][j];
		sinvC[i] = sqrt(block->inva[i]);
	}

	delete_RC_model(model);

	MatDoub &D = A;
	MatDoub m_temp(node_count, node_count);

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
	Symmeig S(D);

	VecDoub &L = S.d;
	MatDoub &U = S.z;
	MatDoub UT(node_count, node_count);

	transpose_matrix(U, UT);

	/* Matrix exponential:
	 * K = exp(D * t) = U * exp(L * t) UT
	 */
	MatDoub &K = A;
	VecDoub v_temp(node_count);

	for (i = 0; i < node_count; i++) v_temp[i] = exp(ts * L[i]);
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix(m_temp, UT, K);

	/* Coefficient matrix G:
	 * G = D^(-1) * (exp(D * t) - I) * C^(-1/2) =
	 * = U * diag((exp(t * l0) - 1) / l0, ...) * UT * C^(-1/2)
	 */
	MatDoub G(node_count, node_count);

	for (i = 0; i < node_count; i++) v_temp[i] = (v_temp[i] - 1) / L[i];
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_diagonal_matrix(m_temp, UT, sinvC, G);

	MatDoub P(step_count, node_count);
	MatDoub Q(step_count, node_count);
	MatDoub Y(step_count, node_count);

	/* M = diag(1/(1 - exp(m * l0)), ....) */
	for (i = 0; i < node_count; i++)
		v_temp[i] = 1.0 / (1.0 - exp(ts * step_count * L[i]));

	/* Q(0) = G * B(0) */
	multiply_matrix_incomplete_vector(G, m_power[0], processor_count, Q[0]);
	/* P(0) = Q(0) */
	copy_vector(P[0], Q[0], node_count);

	for (i = 1; i < step_count; i++) {
		/* Q(i) = G * B(i) */
		multiply_matrix_incomplete_vector(G, m_power[i],
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
			temperature[k] = Y[i][j] * sinvC[j] + am;
}

size_t Hotspot::solve(const Architecture *architecture,
	const matrix_t &m_dynamic_power, matrix_t &m_temperature,
	matrix_t &m_total_power, double tol, size_t maxit)
{
	size_t i, j, k, it;
	size_t processor_count, node_count, step_count, total;
	double ts, am, tmp, error, max_error;

	RC_model_t *model;
	block_model_t *block;

	processor_count = flp->n_units;
	step_count = m_dynamic_power.rows();
	total = processor_count * step_count;

	if (processor_count != m_dynamic_power.cols())
		throw std::runtime_error("The floorplan does not match the given power.");

	model = alloc_RC_model(&cfg, flp);
	block = model->block;

	populate_R_model(model, flp);
	populate_C_model(model, flp);

	node_count = model->block->n_nodes;

	m_temperature.resize(m_dynamic_power);
	m_total_power.resize(m_dynamic_power);

	const double *dynamic_power = m_dynamic_power.pointer();
	double *temperature = m_temperature.pointer();
	double *total_power = m_total_power.pointer();

	ts = model->config->sampling_intvl;
	am = model->config->ambient;

	/* We have:
	 * C * dT/dt = A * T + B
	 */
	MatDoub A(node_count, node_count);
	VecDoub sinvC(node_count);

	for (i = 0; i < node_count; i++) {
		for (j = 0; j < node_count; j++)
			A[i][j] = -block->b[i][j];
		sinvC[i] = sqrt(block->inva[i]);
	}

	delete_RC_model(model);

	MatDoub &D = A;
	MatDoub m_temp(node_count, node_count);

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
	Symmeig S(D);

	VecDoub &L = S.d;
	MatDoub &U = S.z;
	MatDoub UT(node_count, node_count);

	transpose_matrix(U, UT);

	/* Matrix exponential:
	 * K = exp(D * t) = U * exp(L * t) UT
	 */
	MatDoub &K = A;
	VecDoub v_temp(node_count);

	for (i = 0; i < node_count; i++) v_temp[i] = exp(ts * L[i]);
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix(m_temp, UT, K);

	/* Coefficient matrix G:
	 * G = D^(-1) * (exp(D * t) - I) * C^(-1/2) =
	 * = U * diag((exp(t * l0) - 1) / l0, ...) * UT * C^(-1/2)
	 */
	MatDoub G(node_count, node_count);

	for (i = 0; i < node_count; i++) v_temp[i] = (v_temp[i] - 1) / L[i];
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_diagonal_matrix(m_temp, UT, sinvC, G);

	MatDoub P(step_count, node_count);
	MatDoub Q(step_count, node_count);
	MatDoub Y(step_count, node_count);

	/* M = diag(1/(1 - exp(m * l0)), ....) */
	for (i = 0; i < node_count; i++)
		v_temp[i] = 1.0 / (1.0 - exp(ts * step_count * L[i]));

	Hotspot::inject_leakage(architecture, processor_count,
		step_count, dynamic_power, am, total_power);

	/* We come to the iterative part */
	for (it = 0;;) {
		/* Q(0) = G * B(0) */
		multiply_matrix_incomplete_vector(G, m_total_power[0], processor_count, Q[0]);
		/* P(0) = Q(0) */
		copy_vector(P[0], Q[0], node_count);

		for (i = 1; i < step_count; i++) {
			/* Q(i) = G * B(i) */
			multiply_matrix_incomplete_vector(G, m_total_power[i],
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
		max_error = 0;
		for (i = 0, k = 0; i < step_count; i++)
			for (j = 0; j < processor_count; j++, k++) {
				tmp = Y[i][j] * sinvC[j] + am;

				error = abs(temperature[k] - tmp);
				if (max_error < error) max_error = error;

				temperature[k] = tmp;
			}

		it++;

		if (max_error < tol || it >= maxit) break;

		Hotspot::inject_leakage(architecture, processor_count, step_count,
			dynamic_power, temperature, total_power);
	}

	return it;
}

void Hotspot::inject_leakage(const Architecture *architecture,
	size_t processor_count, size_t step_count, const double *dynamic_power,
	const double *temperature, double *total_power)
{
	/* Pleak = Ngate * Iavg * Vdd
	 * Iavg(T, Vdd) = Is(T0, V0) * favg(T, Vdd)
	 *
	 * Where the scaling factor:
	 * f(T, Vdd) = A * T^2 * e^((alpha * Vdd + beta)/T) +
	 *   B * e^(gamma * Vdd + delta)
	 *
	 * From:
	 * "Temperature and Supply Voltage Aware Performance and
	 * Power Modeling at Microarchitecture Level"
	 */

	size_t i, j, k;
	double temp, favg, voltage;
	unsigned long int ngate;

	const processor_vector_t &processors = architecture->processors;

	for (i = 0, k = 0; i < step_count; i++) {
		for (j = 0; j < processor_count; j++, k++) {
			temp = temperature[k];
			voltage = processors[j]->voltage;
			ngate = processors[j]->ngate;

			favg = A * temp * temp *
				exp((alpha * voltage + beta) / temp) +
				B * exp(gamma * voltage + delta);

			total_power[k] = dynamic_power[k] + ngate * Is * favg * voltage;
		}
	}
}

void Hotspot::inject_leakage(const Architecture *architecture,
	size_t processor_count, size_t step_count, const double *dynamic_power,
	double temperature, double *total_power)
{
	size_t i, j, k;
	double favg, voltage;
	unsigned long int ngate;

	const processor_vector_t &processors = architecture->processors;

	for (i = 0, k = 0; i < step_count; i++) {
		for (j = 0; j < processor_count; j++, k++) {
			voltage = processors[j]->voltage;
			ngate = processors[j]->ngate;

			favg = A * temperature * temperature *
				exp((alpha * voltage + beta) / temperature) +
				B * exp(gamma * voltage + delta);

			total_power[k] = dynamic_power[k] + ngate * Is * favg * voltage;
		}
	}
}
