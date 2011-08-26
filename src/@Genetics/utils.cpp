#include <nr3.h>

bool fexist(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp) { fclose(fp); return true; }
	return false;
}

void transpose_matrix(const MatDoub &U, MatDoub &UT)
{
	int i, j;
	int n = U.nrows();
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			UT[j][i] = U[i][j];
}

void multiply_diagonal_matrix_matrix(const VecDoub &V, const MatDoub &M, MatDoub &R)
{
	int i, j;
	int n = M.nrows();
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			R[i][j] = V[i] * M[i][j];
}

void multiply_matrix_diagonal_matrix(const MatDoub &M, const VecDoub &V, MatDoub &R)
{
	int i, j;
	int n = M.nrows();
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			R[i][j] = V[j] * M[i][j];
}

void multiply_matrix_matrix(const MatDoub &M, const MatDoub &N, MatDoub &R)
{
	int i, j, k;
	int n = M.nrows();
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			R[i][j] = 0;
			for (k = 0; k < n; k++)
				R[i][j] += M[i][k] * N[k][j];
		}
	}
}

void multiply_matrix_matrix_diagonal_matrix(const MatDoub &M,
	const MatDoub &N, const VecDoub &V, MatDoub &R)
{
	int i, j, k;
	int n = M.nrows();
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			R[i][j] = 0;
			for (k = 0; k < n; k++)
				R[i][j] += M[i][k] * N[k][j];
			R[i][j] *= V[j];
		}
	}
}

void multiply_matrix_incomplete_vector(const MatDoub &M, const double *V,
	int m, double *R)
{
	int i, j;
	int n = M.nrows();
	for (i = 0; i < n; i++) {
		R[i] = 0;
		for (j = 0; j < m; j++)
			R[i] += M[i][j] * V[j];
	}
}

void multiply_matrix_vector_plus_vector(const MatDoub &M, const double *V,
	const double *A, double *R)
{
	int i, j;
	int n = M.nrows();
	for (i = 0; i < n; i++) {
		R[i] = 0;
		for (j = 0; j < n; j++)
			R[i] += M[i][j] * V[j];
		R[i] += A[i];
	}
}

void multiply_matrix_matrix_vector(const MatDoub &M, const MatDoub &N,
	const double *V, double *R)
{
	int i, j, k;
	int n = M.nrows();
	double sum;
	for (i = 0; i < n; i++) {
		R[i] = 0;
		for (j = 0; j < n; j++) {
			sum = 0;
			for (k = 0; k < n; k++)
				sum += M[i][k] * N[k][j];
			R[i] += sum * V[j];
		}
	}
}
