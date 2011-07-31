#include "hotspot.h"

#include <math.h>
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

	if (!fexist(floorplan) || !fexist(config)) return E_IO;

	prepare_hotspot(floorplan, config, &flp, &model);

	block = model->block;
	nodes = block->n_nodes;

	negA = (double *)alloc(sizeof(double) * nodes * nodes);
	if (!negA) {
		ret = E_MEM;
		goto ret_hotspot;
	}

	dinvC = (double *)alloc(sizeof(double) * nodes);
	if (!dinvC) {
		dealloc(negA);
		ret = E_MEM;
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
	int nodes, int steps, double tol, int maxit, double *T, char *dump)
{
	int ret = 0;
	int i, j, k, goon;
	int total, cores;
	FILE *pdump = NULL;
	double *temp, *T0;
	double ts;

	flp_t *flp;
	RC_model_t *model;

	if (!fexist(floorplan) || !fexist(config)) return E_IO;

	if (dump) pdump = fopen(dump, "w");

	prepare_hotspot(floorplan, config, &flp, &model);

	if (nodes != model->block->n_nodes) {
		ret = E_MISMATCH;
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
			goon = 0;

			for (j = 0; j < steps; j++) {
				compute_temp(model, &power[nodes * j], temp, ts);
				T0 = &T[nodes * j];

				/* NOTE: Only for cores. */
				for (i = 0; !goon && (i < cores); i++)
					if (abs(temp[i] - T0[i]) >= tol) goon = 1;

				memcpy(T0, temp, nodes * sizeof(temp[0]));

				/* NOTE: Only for cores. */
				if (pdump) write_vals(pdump, temp, cores);
			}

			if (!goon) break;
		}
	}
	else {
		/* Without error control */
		for (k = 0; k < maxit; k++) {
			for (j = 0; j < steps; j++) {
				compute_temp(model, &power[nodes * j], temp, ts);
				memcpy(&T[nodes * j], temp, nodes * sizeof(temp[0]));

				/* NOTE: Only for cores. */
				if (pdump) write_vals(pdump, temp, cores);
			}
		}
	}

	ret = k;

	free_dvector(temp);

ret_hotspot:
	free_hotspot(flp, model);

	if (pdump) fclose(pdump);

	return ret;
}

int solve_ssdtc_condensed_equation(char *floorplan, char *config, double *power,
	int nodes, int steps, double *T)
{
	int ret = 0;
	int total, cores;
	double ts;

	flp_t *flp;
	RC_model_t *model;

	if (!fexist(floorplan) || !fexist(config)) return E_IO;

	prepare_hotspot(floorplan, config, &flp, &model);

	if (nodes != model->block->n_nodes) {
		ret = E_MISMATCH;
		goto ret_hotspot;
	}

	cores = model->block->flp->n_units;
	total = cores * steps;

	ts = model->config->sampling_intvl;

	/* TODO */

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
