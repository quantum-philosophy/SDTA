#include <errno.h>

#include <nr3.h>
#include <eigen_sym.h>

#include "hotspot.h"
#include "matrix.h"

extern "C" {
#include <hotspot/flp.h>
#include <hotspot/temperature.h>
#include <hotspot/temperature_block.h>
}

void prepare_hotspot(char *floorplan, char *config,
	str_pair *add_table, int tsize, flp_t **flp, RC_model_t **model)
{
	int i;
	thermal_config_t thermal_config;

	thermal_config = default_thermal_config();

	if (config) {
		str_pair table[MAX_ENTRIES];
		i = read_str_pairs(&table[0], MAX_ENTRIES, config);
		thermal_config_add_from_strs(&thermal_config, table, i);
	}

	if (add_table && tsize) {
		thermal_config_add_from_strs(&thermal_config, add_table, tsize);
	}

	*flp = read_flp(floorplan, FALSE);

	*model = alloc_RC_model(&thermal_config, *flp);

	populate_R_model(*model, *flp);
	populate_C_model(*model, *flp);
}

inline void free_hotspot(flp_t *flp, RC_model_t *model)
{
	delete_RC_model(model);
	free_flp(flp, FALSE);
}

inline void dump_vector(FILE *fp, double *vals, int count)
{
	int i;
	for (i = 0; i < count; i++) fprintf(fp, "%f\t", vals[i]);
	fprintf(fp, "\n");
}

inline int fexist(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp) { fclose(fp); return 1; }
	return 0;
}

int obtain_coefficients(char *floorplan, char *config,
	double **pnegA, double **pdinvC,
	void *(*alloc)(size_t), void (*dealloc)(void *))
{
	int ret = 0;
	int i, nodes;
	double *negA, *dinvC;

    flp_t *flp;
    RC_model_t *model;
	block_model_t *block;

	if (!alloc) alloc = malloc;
	if (!dealloc) dealloc = free;

	if (!fexist(floorplan) || !fexist(config)) return -EIO;

	prepare_hotspot(floorplan, config, NULL, 0, &flp, &model);

	block = model->block;
	nodes = block->n_nodes;

	negA = (double *)alloc(sizeof(double) * nodes * nodes);
	if (!negA) {
		ret = -ENOMEM;
		goto ret_hotspot;
	}

	dinvC = (double *)alloc(sizeof(double) * nodes);
	if (!dinvC) {
		dealloc(negA);
		ret = -ENOMEM;
		goto ret_hotspot;
	}

	memcpy(dinvC, block->inva, sizeof(double) * nodes);

	for (i = 0; i < nodes; i++)
		memcpy(&negA[i * nodes], block->b[i], sizeof(double) * nodes);

	*pnegA = negA;
	*pdinvC = dinvC;

ret_hotspot:
	free_hotspot(flp, model);

	return nodes;
}

int solve_original(char *floorplan, char *config, double *power,
	int nodes, int steps, double tol, int minbad, int maxit, double *T)
{
	int ret = 0;
	int i, j, k;
	int total, cores;
	double *temp, *T0;
	double ts;
	int bad;

	flp_t *flp;
	RC_model_t *model;

	if (!fexist(floorplan) || !fexist(config)) return -EIO;

	prepare_hotspot(floorplan, config, NULL, 0, &flp, &model);

	if (nodes != model->block->n_nodes) {
		ret = -EINVAL;
		goto ret_hotspot;
	}

	cores = model->block->flp->n_units;
	total = cores * steps;

	/* Initialize temperature */
	temp = hotspot_vector(model);
	set_temp(model, temp, model->config->init_temp);

	ts = model->config->sampling_intvl;

	/* Perform +k+ repetitions of the computation process */
	if (tol > 0) {
		/* With error control */
		for (k = 0; k < maxit; k++) {
			bad = 0;

			for (j = 0; j < steps; j++) {
				compute_temp(model, &power[nodes * j], temp, ts);
				T0 = &T[nodes * j];

				/* NOTE: Only for cores. */
				for (i = 0; i < cores; i++)
					if (abs(temp[i] - T0[i]) >= tol) bad++;

				memcpy(T0, temp, nodes * sizeof(temp[0]));
			}

			if (bad <= minbad) break;
		}
	}
	else {
		/* Without error control */
		for (k = 0; k < maxit; k++) {
			for (j = 0; j < steps; j++) {
				compute_temp(model, &power[nodes * j], temp, ts);
				memcpy(&T[nodes * j], temp, nodes * sizeof(temp[0]));
			}
		}
	}

	ret = k;

	free_dvector(temp);

ret_hotspot:
	free_hotspot(flp, model);

	return ret;
}

int solve_condensed_equation(
	/* Configuration of HotSpot */
	char *floorplan, char *config, str_pair *table, int tsize,
	/* Supplied power */
	double *power, int cores, int steps,
	/* Final temperature */
	double *T)
{
	int i, j, total, nodes;
	double ts, am;

	flp_t *flp;
	RC_model_t *model;
	block_model_t *block;

	if (!fexist(floorplan) || !fexist(config)) return -EIO;

	prepare_hotspot(floorplan, config, table, tsize, &flp, &model);

	block = model->block;

	if (cores != block->flp->n_units) {
		free_hotspot(flp, model);
		return -EINVAL;
	}

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

	free_hotspot(flp, model);

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

	/* Y(0) = U * M * UT * P(m-1)
	 * M = diag(1/(1 - exp(m * l0)), ....)
	 */
	for (i = 0; i < nodes; i++)
		v_temp[i] = 1.0 / (1.0 - exp(ts * steps * L[i]));

	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_vector(m_temp, UT, P[steps - 1], Y[0]);

	/* Y(i+1) = K * Y(i) + Q(i) */
	for (i = 1; i < steps; i++)
		multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i - 1], Y[i]);

	/* Return back to T from Y:
	 * T = C^(-1/2) * Y
	 *
	 * And do not forget about the ambient temperature.
	 */
	for (i = 0; i < steps; i++)
		for (j = 0; j < cores; j++)
			T[i * cores + j] = Y[i][j] * sinvC[j] + am;

	return 0;
}

void inject_leakage(const double *dynamic_power, const double *vdd,
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

/* Initial leakage with the same ambient temperature */
void inject_leakage(const double *dynamic_power, const double *vdd,
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

int solve_condensed_equation_with_leakage(
	/* Configuration of HotSpot */
	char *floorplan, char *config, str_pair *table, int tsize,
	/* Dynamic power */
	int cores, int steps,
	const double *dynamic_power,
	/* Static power */
	const double *vdd, const double *ngate,
	/* Final temperature with error control */
	double *T, double tol, int maxit)
{
	int i, j, k, it, total, nodes;
	double ts, am, tmp, error, max_error;
	double *power;

	flp_t *flp;
	RC_model_t *model;
	block_model_t *block;

	if (!fexist(floorplan) || !fexist(config)) return -EIO;

	prepare_hotspot(floorplan, config, table, tsize, &flp, &model);

	block = model->block;

	if (cores != block->flp->n_units) {
		free_hotspot(flp, model);
		return -EINVAL;
	}

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

	free_hotspot(flp, model);

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

	inject_leakage(dynamic_power, vdd, ngate, cores, steps, am, power);

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

		if (max_error < tol) break;
		if (it >= maxit) break;

		inject_leakage(dynamic_power, vdd, ngate, cores, steps, T, power);
	}

	free(power);

	return it;
}
