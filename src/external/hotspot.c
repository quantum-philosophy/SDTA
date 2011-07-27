#include "hotspot.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

void prepare_hotspot(char *floorplan, char *config, flp_t **, RC_model_t **);
void free_hotspot(flp_t *, RC_model_t *);

int read_vals(FILE *, double *);
int read_names(FILE *, char **);
void free_names(char **);
char **alloc_names(int, int);
void write_vals(FILE *, double *, int);
int fexist(char *);

int obtain_hotspot_model(char *floorplan, char *config,
	int *pnodes, HotSpotMatrix *negA, HotSpotMatrix *invC)
{
	int i, j;
	int nodes, cores;
	block_model_t *block;

    flp_t *flp;
    RC_model_t *model;

	if (!fexist(floorplan) || !fexist(config)) return E_IO;

	prepare_hotspot(floorplan, config, &flp, &model);

	block = model->block;
	cores = block->n_units;
	nodes = block->n_nodes;

	*negA = alloc_hotspot_matrix(nodes, nodes);
	*invC = alloc_hotspot_matrix(nodes, nodes);

	for (i = 0; i < nodes; i++) {
		for (j = 0; j < nodes; j++) {
			(*negA)[i][j] = block->b[i][j];
			(*invC)[i][j] = 0;
		}
		(*invC)[i][i] = block->inva[i];
	}

	*pnodes = nodes;

	free_hotspot(flp, model);

	return 0;
}

int solve_ssdtc_with_hotspot(char *floorplan, char *power, char *config,
	double tol, int maxit, int *psteps, int *pcores, HotSpotVector *pT,
	char *dump = NULL)
{
	int ret = 0;
	int i, j, k;
	int num, total, steps, cores, nodes;
	char **names;
	FILE *pin, *pdump = NULL;
	double *vals, *temp, **profile, *T0;
	double ts, error, max_error;
	HotSpotVector T;

	flp_t *flp;
	RC_model_t *model;

	if (dump) pdump = fopen(dump, "w");

	if (!fexist(floorplan) || !fexist(power) || !fexist(config)) return E_IO;

	prepare_hotspot(floorplan, config, &flp, &model);

	cores = model->block->flp->n_units;
	nodes = model->block->n_nodes;

	if (!(pin = fopen(power, "r"))) {
		ret = E_PROFILE;
		goto ret_hotspot;
	}

	names = alloc_names(MAX_UNITS, STR_SIZE);
	if (read_names(pin, names) != cores) {
		ret = E_NAMES;
		goto ret_names;
	}

	steps = 0;
	vals = dvector(MAX_UNITS);
	profile = dmatrix(MAX_POWER_CHUNK, nodes);

	/* Read the power profile */
	while ((num = read_vals(pin, vals)) != 0) {
		if (num != cores) {
			ret = E_VALUES;
			goto ret_profile;
		}

		steps++;
		if (steps > MAX_POWER_CHUNK) {
			ret = E_STEPS;
			goto ret_profile;
		}

		/* Permute */
		for (i = 0; i < cores; i++)
			profile[steps - 1][get_blk_index(flp, names[i])] = vals[i];
	}

	total = cores * steps;

	/* Initialize temperature */
	temp = hotspot_vector(model);
	set_temp(model, temp, model->config->init_temp);

	T = alloc_hotspot_vector(total);
	memset(T, 0, sizeof(T[0]) * total);

	ts = model->config->sampling_intvl;

	/* Perform +k+ repetitions of the computation process */
	if (tol > 0) {
		/* With error control */
		for (k = 0; k < maxit; k++) {
			max_error = 0;

			for (j = 0; j < steps; j++) {
				compute_temp(model, profile[j], temp, ts);
				T0 = &T[cores * j];
				for (i = 0; i < cores; i++) {
					if (temp[i] > T0[i]) error = temp[i] - T0[i];
					else error = T0[i] - temp[i];
					if (error > max_error) max_error = error;
				}
				memcpy(T0, temp, cores * sizeof(temp[0]));
				if (pdump) write_vals(pdump, temp, cores);
			}

			if (max_error < tol) break;
		}
	}
	else {
		/* Without error control */
		for (k = 0; k < maxit; k++) {
			for (j = 0; j < steps; j++) {
				compute_temp(model, profile[j], temp, ts);
				memcpy(&T[cores * j], temp, cores * sizeof(temp[0]));
				if (pdump) write_vals(pdump, temp, cores);
			}
		}
	}

	/* Permute back */
	for (j = 0; j < steps; j++) {
		T0 = &T[j * cores];
		for (i = 0; i < cores; i++)
			temp[i] = T0[get_blk_index(flp, names[i])] - KELVIN;
		memcpy(T0, temp, cores * sizeof(temp[0]));
	}

	*psteps = steps;
	*pcores = cores;
	*pT = T;

	ret = k;

	free_dvector(temp);

ret_profile:
	free_dmatrix(profile);
	free_dvector(vals);

ret_names:
	free_names(names);
	fclose(pin);

ret_hotspot:
	free_hotspot(flp, model);

	if (pdump) fclose(pdump);

	return ret;
}

int solve_sst_with_hotspot(char *floorplan, char *power, char *config,
	int *pcores, HotSpotVector *pT)
{
	int ret = 0;
	int i;
	int num, steps, cores, nodes;
	char **names;
	FILE *pin;
	double *vals, *overall_power;
	HotSpotVector T;

	flp_t *flp;
	RC_model_t *model;

	if (!fexist(floorplan) || !fexist(power) || !fexist(config)) return E_IO;

	prepare_hotspot(floorplan, config, &flp, &model);

	cores = model->block->flp->n_units;
	nodes = model->block->n_nodes;

	if (!(pin = fopen(power, "r"))) {
		ret = E_PROFILE;
		goto ret_hotspot;
	}

	names = alloc_names(MAX_UNITS, STR_SIZE);
	if (read_names(pin, names) != cores) {
		ret = E_NAMES;
		goto ret_names;
	}

	steps = 0;
	vals = dvector(MAX_UNITS);
	overall_power = hotspot_vector(model);

	/* Read the power profile */
	while ((num = read_vals(pin, vals)) != 0) {
		if (num != cores) {
			ret = E_VALUES;
			goto ret_profile;
		}

		/* Permute & sum up */
		for (i = 0; i < cores; i++)
			overall_power[get_blk_index(flp, names[i])] += vals[i];

		steps++;
	}

	for (i = 0; i < cores; i++) overall_power[i] /= (double)steps;

	T = hotspot_vector(model);
	steady_state_temp(model, overall_power, T);

	for (i = 0; i < cores; i++) T[i] = T[i] - KELVIN;

	*pcores = cores;
	*pT = T;

ret_profile:
	free_dvector(overall_power);
	free_dvector(vals);

ret_names:
	free_names(names);
	fclose(pin);

ret_hotspot:
	free_hotspot(flp, model);

	return ret;
}

void prepare_hotspot(char *floorplan, char *config,
	flp_t **flp, RC_model_t **model)
{
	int i;
	thermal_config_t thermal_config;

	thermal_config = default_thermal_config();

	if (config) {
		str_pair table[MAX_ENTRIES];
		i = read_str_pairs(&table[0], MAX_ENTRIES, config);
		thermal_config_add_from_strs(&thermal_config, table, i);
	}

	*flp = read_flp(floorplan, FALSE);

	*model = alloc_RC_model(&thermal_config, *flp);

	populate_R_model(*model, *flp);
	populate_C_model(*model, *flp);
}

void free_hotspot(flp_t *flp, RC_model_t *model)
{
	delete_RC_model(model);
	free_flp(flp, FALSE);
}

char **alloc_names(int nr, int nc)
{
	int i;
	char **m;

	m = (char **) calloc (nr, sizeof(char *));
	assert(m != NULL);
	m[0] = (char *) calloc (nr * nc, sizeof(char));
	assert(m[0] != NULL);

	for (i = 1; i < nr; i++)
    	m[i] =  m[0] + nc * i;

	return m;
}

void free_names(char **m)
{
	free(m[0]);
	free(m);
}

/*
 * read a single line of trace file containing names
 * of functional blocks
 */
int read_names(FILE *fp, char **names)
{
	char line[LINE_SIZE], temp[LINE_SIZE], *src;
	int i;

	/* skip empty lines	*/
	do {
		/* read the entire line	*/
		fgets(line, LINE_SIZE, fp);
		if (feof(fp))
			fatal("not enough names in trace file\n");
		strcpy(temp, line);
		src = strtok(temp, " \r\t\n");
	} while (!src);

	/* new line not read yet	*/
	if(line[strlen(line)-1] != '\n')
		fatal("line too long\n");

	/* chop the names from the line read	*/
	for(i=0,src=line; *src && i < MAX_UNITS; i++) {
		if(!sscanf(src, "%s", names[i]))
			fatal("invalid format of names\n");
		src += strlen(names[i]);
		while (isspace((int)*src))
			src++;
	}
	if(*src && i == MAX_UNITS)
		fatal("no. of units exceeded limit\n");

	return i;
}

/* read a single line of power trace numbers	*/
int read_vals(FILE *fp, double *vals)
{
	char line[LINE_SIZE], temp[LINE_SIZE], *src;
	int i;

	/* skip empty lines	*/
	do {
		/* read the entire line	*/
		fgets(line, LINE_SIZE, fp);
		if (feof(fp))
			return 0;
		strcpy(temp, line);
		src = strtok(temp, " \r\t\n");
	} while (!src);

	/* new line not read yet	*/
	if(line[strlen(line)-1] != '\n')
		fatal("line too long\n");

	/* chop the power values from the line read	*/
	for(i=0,src=line; *src && i < MAX_UNITS; i++) {
		if(!sscanf(src, "%s", temp) || !sscanf(src, "%lf", &vals[i]))
			fatal("invalid format of values\n");
		src += strlen(temp);
		while (isspace((int)*src))
			src++;
	}
	if(*src && i == MAX_UNITS)
		fatal("no. of entries exceeded limit\n");

	return i;
}

void write_vals(FILE *fp, double *vals, int count)
{
	int i;
	for (i = 0; i < count; i++) fprintf(fp, "%f\t", vals[i]);
	fprintf(fp, "\n");
}

int fexist(char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp) { fclose(fp); return 1; }
	return 0;
}
