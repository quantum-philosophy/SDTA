#include <stdexcept>

#include <nr3.h>
#include <eigen_sym.h>

#include "Hotspot.h"
#include "utils.h"

Hotspot::Hotspot(char *floorplan, char *config, str_pair *extra_table, int rows)
{
	if (!fexist(floorplan) || !fexist(config))
		throw std::runtime_error("The configuration files do not exist");

	cfg = default_thermal_config();

	if (config) {
		str_pair table[MAX_ENTRIES];
		int i = read_str_pairs(&table[0], MAX_ENTRIES, config);
		thermal_config_add_from_strs(&cfg, table, i);
	}

	if (extra_table && rows)
		thermal_config_add_from_strs(&cfg, extra_table, rows);

	flp = read_flp(floorplan, FALSE);
}

Hotspot::~Hotspot()
{
	free_flp(flp, FALSE);
}

unsigned int Hotspot::solve(double *dynamic_power, int steps, const double *vdd,
	const double *ngate, double *T, double tol, int maxit)
{
	int i, j, k, it, total, cores, nodes;
	double ts, am, tmp, error, max_error;
	double *power;

	RC_model_t *model;
	block_model_t *block;

	model = alloc_RC_model(&cfg, flp);
	block = model->block;

	populate_R_model(model, flp);
	populate_C_model(model, flp);

	cores = flp->n_units;
	nodes = model->block->n_nodes;
	total = cores * steps;

	ts = model->config->sampling_intvl;
	am = model->config->ambient;

	/* We have:
	 * C * dT/dt = A * T + B
	 */
	MatDoub A(nodes, nodes);
	VecDoub sinvC(nodes);

	for (i = 0; i < nodes; i++) {
		for (j = 0; j < nodes; j++)
			A[i][j] = -block->b[i][j];
		sinvC[i] = sqrt(block->inva[i]);
	}

	delete_RC_model(model);

	MatDoub &D = A;
	MatDoub m_temp(nodes, nodes);

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
	MatDoub UT(nodes, nodes);

	transpose_matrix(U, UT);

	/* Matrix exponential:
	 * K = exp(D * t) = U * exp(L * t) UT
	 */
	MatDoub &K = A;
	VecDoub v_temp(nodes);

	for (i = 0; i < nodes; i++) v_temp[i] = exp(ts * L[i]);
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix(m_temp, UT, K);

	/* Coefficient matrix G:
	 * G = D^(-1) * (exp(D * t) - I) * C^(-1/2) =
	 * = U * diag((exp(t * l0) - 1) / l0, ...) * UT * C^(-1/2)
	 */
	MatDoub G(nodes, nodes);

	for (i = 0; i < nodes; i++) v_temp[i] = (v_temp[i] - 1) / L[i];
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_diagonal_matrix(m_temp, UT, sinvC, G);

	MatDoub P(steps, nodes);
	MatDoub Q(steps, nodes);
	MatDoub Y(steps, nodes);

	/* M = diag(1/(1 - exp(m * l0)), ....) */
	for (i = 0; i < nodes; i++)
		v_temp[i] = 1.0 / (1.0 - exp(ts * steps * L[i]));

	power = (double *)malloc(sizeof(double) * steps * cores);

	Hotspot::inject_leakage(dynamic_power, vdd, ngate, cores, steps, am, power);

	/* We come to the iterative part */
	for (it = 0;;) {
		/* Q(0) = G * B(0) */
		multiply_matrix_incomplete_vector(G, &power[0], cores, Q[0]);
		/* P(0) = Q(0) */
		copy_vector(P[0], Q[0], nodes);

		for (i = 1; i < steps; i++) {
			/* Q(i) = G * B(i) */
			multiply_matrix_incomplete_vector(G, &power[i * cores], cores, Q[i]);
			/* P(i) = K * P(i-1) + Q(i) */
			multiply_matrix_vector_plus_vector(K, P[i - 1], Q[i], P[i]);
		}

		/* Y(0) = U * M * UT * P(m-1), for M see above ^ */
		multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
		multiply_matrix_matrix_vector(m_temp, UT, P[steps - 1], Y[0]);

		/* Y(i+1) = K * Y(i) + Q(i) */
		for (i = 1; i < steps; i++)
			multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i - 1], Y[i]);

		/* Return back to T from Y:
		 * T = C^(-1/2) * Y
		 *
		 * And do not forget about the ambient temperature. Also perform
		 * the error control.
		 */
		k = 0;
		max_error = 0;
		for (i = 0; i < steps; i++)
			for (j = 0; j < cores; j++, k++) {
				tmp = Y[i][j] * sinvC[j] + am;

				error = abs(T[k] - tmp);
				if (max_error < error) max_error = error;

				T[k] = tmp;
			}

		it++;

		if (max_error < tol || it >= maxit) break;

		Hotspot::inject_leakage(dynamic_power, vdd, ngate, cores, steps, T, power);
	}

	/* Return the power back with the leakage part */
	copy_vector(dynamic_power, power, steps * cores);

	free(power);

	return it;
}

void Hotspot::inject_leakage(const double *dynamic_power, const double *vdd,
	const double *ngate, int cores, int steps, const double *T, double *total_power)
{
	int i, j, k;
	double favg;

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

	k = 0;
	for (i = 0; i < steps; i++)
		for (j = 0; j < cores; j++, k++) {
			favg = __A * T[k] * T[k] *
				exp((__alpha * vdd[j] + __beta) / T[k]) +
				__B * exp(__gamma * vdd[j] + __delta);

			total_power[k] = dynamic_power[k] +
				ngate[j] * __Is * favg * vdd[j];
		}
}

void Hotspot::inject_leakage(const double *dynamic_power, const double *vdd,
	const double *ngate, int cores, int steps, double T, double *total_power)
{
	int i, j, k;
	double favg;

	k = 0;
	for (i = 0; i < steps; i++)
		for (j = 0; j < cores; j++, k++) {
			favg = __A * T * T *
				exp((__alpha * vdd[j] + __beta) / T) +
				__B * exp(__gamma * vdd[j] + __delta);

			total_power[k] = dynamic_power[k] +
				ngate[j] * __Is * favg * vdd[j];
		}
}
