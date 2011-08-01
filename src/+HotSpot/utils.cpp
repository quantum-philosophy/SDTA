#include "utils.h"

#include <string.h>

int parse_structure_config(const mxArray *structure, str_pair **ptable)
{
	int tsize = mxGetNumberOfElements(structure);
	if (!tsize) return 0;

	int i;
	mxArray *value;
	const char *name;
	char *temp;

	str_pair *table = (str_pair *)mxCalloc(tsize, sizeof(str_pair));

	/* ATTENTION: Unsafe strcpy and sprintf, with hope for the best...
	 */
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
