#include "matrix.h"

void transpose_matrix(
	const matrix_t &U, matrix_t &UT)
{
	int i, j;
	int n = U.rows();
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			UT[j][i] = U[i][j];
}

void multiply_diagonal_matrix_matrix(
	const vector_t &V, const matrix_t &M, matrix_t &R)
{
	int i, j;
	int n = M.rows();
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			R[i][j] = V[i] * M[i][j];
}

void multiply_matrix_diagonal_matrix(
	const matrix_t &M, const vector_t &V, matrix_t &R)
{
	int i, j;
	int n = M.rows();
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			R[i][j] = V[j] * M[i][j];
}

void multiply_matrix_matrix(
	const matrix_t &M, const matrix_t &N, matrix_t &R)
{
	int i, j, k;
	int n = M.rows();
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			R[i][j] = 0;
			for (k = 0; k < n; k++)
				R[i][j] += M[i][k] * N[k][j];
		}
	}
}

void multiply_matrix_matrix_diagonal_matrix(
	const matrix_t &M, const matrix_t &N, const vector_t &V, matrix_t &R)
{
	int i, j, k;
	int n = M.rows();
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			R[i][j] = 0;
			for (k = 0; k < n; k++)
				R[i][j] += M[i][k] * N[k][j];
			R[i][j] *= V[j];
		}
	}
}

void multiply_matrix_incomplete_vector(
	const matrix_t &M, const double *V, int m, double *R)
{
	int i, j;
	int n = M.rows();
	for (i = 0; i < n; i++) {
		R[i] = 0;
		for (j = 0; j < m; j++)
			R[i] += M[i][j] * V[j];
	}
}

void multiply_matrix_vector_plus_vector(
	const matrix_t &M, const double *V, const double *A, double *R)
{
	int i, j;
	int n = M.rows();
	for (i = 0; i < n; i++) {
		R[i] = 0;
		for (j = 0; j < n; j++)
			R[i] += M[i][j] * V[j];
		R[i] += A[i];
	}
}

void multiply_matrix_matrix_vector(
	const matrix_t &M, const matrix_t &N, const double *V, double *R)
{
	int i, j, k;
	int n = M.rows();
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

const double EigenvalueDecomposition::epsilon = std::numeric_limits<double>::epsilon();

void EigenvalueDecomposition::tred2()
{
	int l,k,j,i;
	double scale,hh,h,g,f;
	for (i=n-1;i>0;i--) {
		l=i-1;
		h=scale=0.0;
		if (l > 0) {
			for (k=0;k<i;k++)
				scale += abs(z[i][k]);
			if (scale == 0.0)
				e[i]=z[i][l];
			else {
				for (k=0;k<i;k++) {
					z[i][k] /= scale;
					h += z[i][k]*z[i][k];
				}
				f=z[i][l];
				g=(f >= 0.0 ? -sqrt(h) : sqrt(h));
				e[i]=scale*g;
				h -= f*g;
				z[i][l]=f-g;
				f=0.0;
				for (j=0;j<i;j++) {
					z[j][i]=z[i][j]/h;
					g=0.0;
					for (k=0;k<j+1;k++)
						g += z[j][k]*z[i][k];
					for (k=j+1;k<i;k++)
						g += z[k][j]*z[i][k];
					e[j]=g/h;
					f += e[j]*z[i][j];
				}
				hh=f/(h+h);
				for (j=0;j<i;j++) {
					f=z[i][j];
					e[j]=g=e[j]-hh*f;
					for (k=0;k<j+1;k++)
						z[j][k] -= (f*e[k]+g*z[i][k]);
				}
			}
		} else
			e[i]=z[i][l];
		d[i]=h;
	}
	d[0]=0.0;
	e[0]=0.0;
	for (i=0;i<n;i++) {
		if (d[i] != 0.0) {
			for (j=0;j<i;j++) {
				g=0.0;
				for (k=0;k<i;k++)
					g += z[i][k]*z[k][j];
				for (k=0;k<i;k++)
					z[k][j] -= g*z[k][i];
			}
		}
		d[i]=z[i][i];
		z[i][i]=1.0;
		for (j=0;j<i;j++) z[j][i]=z[i][j]=0.0;
	}
}

void EigenvalueDecomposition::tqli()
{
	int m,l,iter,i,k;
	double s,r,p,g,f,dd,c,b;
	for (i=1;i<n;i++) e[i-1]=e[i];
	e[n-1]=0.0;
	for (l=0;l<n;l++) {
		iter=0;
		do {
			for (m=l;m<n-1;m++) {
				dd=abs(d[m])+abs(d[m+1]);
				if (abs(e[m]) <= epsilon*dd) break;
			}
			if (m != l) {
				if (iter++ == 30) throw("Too many iterations in tqli");
				g=(d[l+1]-d[l])/(2.0*e[l]);
				r=pythag(g,1.0);
				g=d[m]-d[l]+e[l]/(g+sign(r,g));
				s=c=1.0;
				p=0.0;
				for (i=m-1;i>=l;i--) {
					f=s*e[i];
					b=c*e[i];
					e[i+1]=(r=pythag(f,g));
					if (r == 0.0) {
						d[i+1] -= p;
						e[m]=0.0;
						break;
					}
					s=f/r;
					c=g/r;
					g=d[i+1]-p;
					r=(d[i]-g)*s+2.0*c*b;
					d[i+1]=g+(p=s*r);
					g=c*r-b;
					for (k=0;k<n;k++) {
						f=z[k][i+1];
						z[k][i+1]=s*z[k][i]+c*f;
						z[k][i]=c*z[k][i]-s*f;
					}
				}
				if (r == 0.0 && i >= l) continue;
				d[l] -= p;
				e[l]=g;
				e[m]=0.0;
			}
		} while (m != l);
	}
}

void EigenvalueDecomposition::eigsrt(vector_t &d, matrix_t &v) const
{
	int k;
	int n=d.size();
	for (int i=0;i<n-1;i++) {
		double p=d[k=i];
		for (int j=i;j<n;j++)
			if (d[j] >= p) p=d[k=j];
		if (k != i) {
			d[k]=d[i];
			d[i]=p;
			for (int j=0;j<n;j++) {
				p=v[j][i];
				v[j][i]=v[j][k];
				v[j][k]=p;
			}
		}
	}
}
