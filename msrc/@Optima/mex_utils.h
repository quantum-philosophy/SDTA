#ifndef __MEX_UTILS_H__
#define __MEX_UTILS_H__

#include <mex.h>
#include <string>

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

std::string array_to_string(const mxArray *array)
{
	char *pointer = mxArrayToString(array);

	if (!pointer)
		mexErrMsgTxt("Cannot read the string.");

	std::string line(pointer);

	mxFree(pointer);

	return line;
}

#endif
