#include "mex_utils.h"

#include <string.h>

int parse_structure_config(const mxArray *structure, str_pair **ptable)
{
	if (mxGetNumberOfElements(structure) != 1) return 0;

	int tsize = mxGetNumberOfFields(structure);
	if (!tsize) return 0;

	int i;
	mxArray *value;
	const char *name;
	char *temp;

	str_pair *table = (str_pair *)mxCalloc(tsize, sizeof(str_pair));

	/* ATTENTION: Unsafe strcpy and sprintf, with hope for the best... */
	for (i = 0; i < tsize; i++) {
		value = mxGetFieldByNumber(structure, 0, i);
		name = mxGetFieldNameByNumber(structure, i);
		strcpy(table[i].name, name);
		if (mxIsChar(value)) {
			temp = mxArrayToString(value);
			strcpy(table[i].value, temp);
			mxFree(temp);
		}
		else if (mxIsNumeric(value)) {
			sprintf(table[i].value, "%f", mxGetScalar(value));
		}
		else {
			mxFree(table);
			return 0;
		}
	}

	*ptable = table;

	return tsize;
}

void verify_and_fetch_properties(int nrhs, const mxArray *prhs[],
	char **floorplan, char **config)
{
	if (nrhs < 1 || strcmp("HotSpot", mxGetClassName(prhs[0])) != 0)
		mexErrMsgTxt("The first input should be an instance of the Hotspot class.");

	/* Floorplan */
	mxArray *pfloorplan = mxGetProperty(prhs[0], 0, "floorplan");
	if (!pfloorplan) mexErrMsgTxt("Cannot read the floorplan property.");

	*floorplan = mxArrayToString(pfloorplan);
	if (!*floorplan) mexErrMsgTxt("Cannot read the floorplan property.");

	/* Hotspot config */
	mxArray *pconfig = mxGetProperty(prhs[0], 0, "config");
	if (!pfloorplan) mexErrMsgTxt("Cannot read the config property.");

	*config = mxArrayToString(pconfig);
	if (!*config) {
		mxFree(*floorplan);
		mexErrMsgTxt("Cannot read the config property.");
	}
}

void mex_matrix_to_c(double *dest, const double *src, size_t rows, size_t cols)
{
	for (size_t i = 0; i < rows; i++)
		for (size_t j = 0; j < cols; j++)
			dest[i * cols + j] = src[i + j * rows];
}

void c_matrix_to_mex(double *dest, const double *src, size_t rows, size_t cols)
{
	for (size_t i = 0; i < rows; i++)
		for (size_t j = 0; j < cols; j++)
			dest[i + j * rows] = src[i * cols + j];
}
