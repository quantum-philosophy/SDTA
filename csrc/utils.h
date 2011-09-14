#ifndef __UTILS_H__
#define __UTILS_H__

#include <nr3.h>

#define copy_vector(dst, src, n) memcpy(dst, src, sizeof(double) * n)

void transpose_matrix(const MatDoub &U, MatDoub &UT);
void multiply_diagonal_matrix_matrix(const VecDoub &V, const MatDoub &M, MatDoub &R);
void multiply_matrix_diagonal_matrix(const MatDoub &M, const VecDoub &V, MatDoub &R);
void multiply_matrix_matrix(const MatDoub &M, const MatDoub &N, MatDoub &R);
void multiply_matrix_matrix_diagonal_matrix(const MatDoub &M,
	const MatDoub &N, const VecDoub &V, MatDoub &R);
void multiply_matrix_incomplete_vector(const MatDoub &M, const double *V,
	int m, double *R);
void multiply_matrix_vector_plus_vector(const MatDoub &M, const double *V,
	const double *A, double *R);
void multiply_matrix_matrix_vector(const MatDoub &M, const MatDoub &N,
	const double *V, double *R);

#endif
