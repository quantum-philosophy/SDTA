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

inline int fexist(char *filename)
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

int solve_ssdtc_original(char *floorplan, char *config, double *power,
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

int solve_ssdtc_condensed_equation(char *floorplan, char *config,
	str_pair *table, int tsize, double *power, int cores, int steps, double *T)
{
	int ret = 0;
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

	multiply_diagonal_matrix_matrix(sinvC, A, m_temp);
	multiply_matrix_diagonal_matrix(m_temp, sinvC, D);

	/* Eigenvalue decomposition */
	Symmeig S(D);

	VecDoub &L = S.d;
	MatDoub &U = S.z;
	MatDoub UT(nodes, nodes);

	transpose_matrix(U, UT);

	/* Matrix exponential */
	MatDoub &K = A;
	VecDoub v_temp(nodes);

	for (i = 0; i < nodes; i++) v_temp[i] = exp(ts * L[i]);
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix(m_temp, UT, K);

	/* Coefficient matrix G */
	MatDoub G(nodes, nodes);

	for (i = 0; i < nodes; i++) v_temp[i] = (v_temp[i] - 1) / L[i];
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_diagonal_matrix(m_temp, UT, sinvC, G);

	MatDoub P(steps, nodes);
	MatDoub Q(steps, nodes);

	multiply_matrix_incomplete_vector(G, &power[0], cores, Q[0]);
	copy_vector(P[0], Q[0], nodes);

	for (i = 1; i < steps; i++) {
		multiply_matrix_incomplete_vector(G, &power[i * cores], cores, Q[i]);
		multiply_matrix_vector_plus_vector(K, P[i - 1], Q[i], P[i]);
	}

	for (i = 0; i < nodes; i++)
		v_temp[i] = 1.0 / (1.0 - exp(ts * steps * L[i]));

	MatDoub Y(steps, nodes);

	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_vector(m_temp, UT, P[steps - 1], Y[0]);

	for (i = 1; i < steps; i++)
		multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i - 1], Y[i]);

	for (i = 0; i < steps; i++)
		for (j = 0; j < cores; j++)
			T[i * cores + j] = Y[i][j] * sinvC[j] + am;

	return ret;
}
