#ifndef __MEX_UTILS_H__
#define __MEX_UTILS_H__

#include <mex.h>
#include <string>

#include "common.h"

void from_matlab(double *dest, const double *src, size_t rows, size_t cols)
{
	for (size_t i = 0; i < rows; i++)
		for (size_t j = 0; j < cols; j++)
			dest[i * cols + j] = src[i + j * rows];
}

void to_matlab(double *dest, const double *src, size_t rows, size_t cols)
{
	for (size_t i = 0; i < rows; i++)
		for (size_t j = 0; j < cols; j++)
			dest[i + j * rows] = src[i * cols + j];
}

void from_matlab(const mxArray *array, matrix_t &matrix)
{
	if (!array) mexErrMsgTxt("The value is not given.");

	size_t rows = mxGetM(array);
	size_t cols = mxGetN(array);

	matrix.resize(rows, cols);
	from_matlab(matrix.pointer(), mxGetPr(array), rows, cols);
}

template<class T>
T from_matlab(const mxArray *array)
{
	if (!array) mexErrMsgTxt("The value is not given.");
	return (T)mxGetScalar(array);
}

template<class T>
T from_matlab(const mxArray *array, T value)
{
	if (!array) return value;
	return from_matlab<T>(array);
}

template<>
std::string from_matlab(const mxArray *array)
{
	if (!array) mexErrMsgTxt("The value is not given.");

	char *pointer = mxArrayToString(array);
	if (!pointer) mexErrMsgTxt("Cannot read the string.");

	std::string line(pointer);

	mxFree(pointer);

	return line;
}

template<>
std::string from_matlab(const mxArray *array, std::string value)
{
	if (!array) return value;
	return from_matlab<std::string>(array);
}

template<class T>
mxArray *to_matlab(const T scalar)
{
	mxArray *out = mxCreateDoubleMatrix(1, 1, mxREAL);
	double *_out = mxGetPr(out);
	*_out = (double)scalar;

	return out;
}

mxArray *to_matlab(const matrix_t &src)
{
	size_t rows = src.rows();
	size_t cols = src.cols();

	mxArray *mex = mxCreateDoubleMatrix(rows, cols, mxREAL);
	to_matlab(mxGetPr(mex), src.pointer(), rows, cols);

	return mex;
}

mxArray *to_matlab(const vector_t &src)
{
	size_t size = src.size();

	mxArray *mex = mxCreateDoubleMatrix(1, size, mxREAL);
	memcpy(mxGetPr(mex), &src[0], sizeof(double) * size);

	return mex;
}

#endif
