#include <mex.h>
#include <mat.h>

#include <stdexcept>
#include <iostream>
#include <string.h>

#include "Common.h"

std::ostream &operator<< (std::ostream &o, const std::vector<int> &vector)
{
	size_t size = vector.size();

	o << "[ ";
	for (size_t i = 0; i < size; i++) {
		o << vector[i];
		if (i < size - 1) o << ", ";
	}
	o << " ]" << std::endl;

	return o;
}

void dump_into_matlab(const char *filename, const char *name,
	const matrix_t &matrix)
{
	size_t i, j;
	size_t rows = matrix.rows();
	size_t cols = matrix.cols();

	mxArray *array = mxCreateDoubleMatrix(rows, cols, mxREAL);
	double *ptr = mxGetPr(array);

	for (i = 0; i < rows; i++)
		for (j = 0; j < cols; j++)
			ptr[rows * j + i] = matrix[i][j];

	MATFile *fp = matOpen(filename, "w");

	if (!fp)
		throw std::runtime_error("Cannot dump the given matrix.");

	matPutVariable(fp, name, array);
	matClose(fp);

	mxDestroyArray(array);
}
