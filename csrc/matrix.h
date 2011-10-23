#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits>

#define __ALLOC(size) \
	(double *)malloc(sizeof(double) * size)

#define __FREE(some) \
	do { \
		if (some) free(some); \
		some = NULL; \
	} while(0)

#define __MEMSET(some, value, size) \
	memset(some, value, sizeof(double) * size)

#define __NULLIFY(some, size) \
	memset(some, 0, sizeof(double) * size)

#define __MEMCPY(dst, src, size) \
	memcpy(dst, src, sizeof(double) * size)

struct vector_t
{
	size_t _size;
	double *_data;

	vector_t() : _size(0), _data(NULL)
	{
	}

	vector_t(size_t __size) : _size(__size)
	{
		_data = __ALLOC(_size);
	}

	vector_t(size_t __size, double value) : _size(__size)
	{
		_data = __ALLOC(_size);

		if (value == 0) nullify();
		else {
			for (size_t i = 0; i < _size; i++)
				_data[i] = value;
		}
	}

	vector_t(const vector_t &another) : _size(another._size)
	{
		_data = __ALLOC(_size);
		__MEMCPY(_data, another._data, _size);
	}

	~vector_t()
	{
		__FREE(_data);
	}

	inline void nullify()
	{
		__NULLIFY(_data, _size);
	}

	inline void resize(size_t __size)
	{
		if (_size == __size) return;

		__FREE(_data);

		_size = __size;
		_data = __ALLOC(_size);
	}

	inline void shrink(size_t __size)
	{
#ifndef SHALLOW_CHECK
		if (_size < __size)
			throw std::runtime_error("Cannot shrink.");
#endif

		_size = __size;
	}

	inline double &operator[](unsigned int i)
	{
		return _data[i];
	}

	inline const double &operator[](unsigned int i) const
	{
		return _data[i];
	}

	inline vector_t &operator=(const vector_t &another)
	{
		if (_size != another._size) {
			__FREE(_data);

			_size = another._size;
			_data = __ALLOC(_size);
		}

		__MEMCPY(_data, another._data, _size);
		return *this;
	}

	inline operator double *()
	{
		return _data;
	}

	inline operator const double *() const
	{
		return _data;
	}

	inline double *pointer()
	{
		return _data;
	}

	inline const double *pointer() const
	{
		return _data;
	}

	inline size_t size() const
	{
		return _size;
	}
};

struct matrix_t: public vector_t
{
	size_t _rows;
	size_t _cols;

	matrix_t() : vector_t(), _rows(0), _cols(0)
	{
	}

	matrix_t(size_t __rows, size_t __cols) :
		vector_t(__rows * __cols), _rows(__rows), _cols(__cols)
	{
	}

	matrix_t(const matrix_t &another) :
		vector_t(another), _rows(another._rows), _cols(another._cols)
	{
	}

	inline double *operator[](unsigned int row)
	{
		return &_data[_cols * row];
	}

	inline const double *operator[](unsigned int row) const
	{
		return &_data[_cols * row];
	}

	inline matrix_t &operator=(const matrix_t &another)
	{
		_rows = another._rows; _cols = another._cols;
		(void)vector_t::operator=(another);
		return *this;
	}

	inline void resize(size_t __rows, size_t __cols)
	{
		_rows = __rows; _cols = __cols;
		vector_t::resize(_rows * _cols);
	}

	inline void resize(const matrix_t &another)
	{
		_rows = another._rows; _cols = another._cols;
		vector_t::resize(_rows * _cols);
	}

	inline void shrink(size_t __rows)
	{
		_rows = __rows;
		vector_t::shrink(_rows * _cols);
	}

	inline size_t rows() const
	{
		return _rows;
	}

	inline size_t cols() const
	{
		return _cols;
	}
};

void transpose_matrix(
	const matrix_t &U, matrix_t &UT);
void multiply_diagonal_matrix_matrix(
	const vector_t &V, const matrix_t &M, matrix_t &R);
void multiply_matrix_diagonal_matrix(
	const matrix_t &M, const vector_t &V, matrix_t &R);
void multiply_matrix_matrix(
	const matrix_t &M, const matrix_t &N, matrix_t &R);
void multiply_matrix_matrix(
	const matrix_t &M, const matrix_t &N, double *R);
void multiply_matrix_matrix_diagonal_matrix(
	const matrix_t &M, const matrix_t &N, const vector_t &V, matrix_t &R);
void multiply_matrix_incomplete_vector(
	const matrix_t &M, const double *V, int m, double *R);
void multiply_matrix_vector_plus_vector(
	const matrix_t &M, const double *V, const double *A, double *R);
void multiply_matrix_vector_plus_vector(
	size_t n, const double *M, const double *V, const double *A, double *R);
void multiply_matrix_matrix_vector(
	const matrix_t &M, const matrix_t &N, const double *V, double *R);
void multiply_matrix_vector(
	const matrix_t &M, const vector_t &V, double *R);

class EigenvalueDecomposition
{
	static const double epsilon;
	const int n;

	matrix_t &z;
	vector_t &d, e;

	public:

	EigenvalueDecomposition(const matrix_t &M, matrix_t &U, vector_t &L) :
		n(M.rows()), z(U), d(L), e(n)
	{
		z = M;
		tred2();
		tqli();
		eigsrt(d, z);
	}

	private:

	void tred2();
	void tqli();
	void eigsrt(vector_t &d, matrix_t &v) const;

	inline double abs(const double &a) const
	{
		return a < 0 ? (-a) : a;
	}

	inline double sign(const double &a, const double &b) const
	{
		return (double)(b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a));
	}

	inline double sqr(const double &a) const
	{
		return a * a;
	}

	inline double pythag(const double &a, const double &b) const
	{
		double absa = abs(a), absb = abs(b);
		return (absa > absb ? absa * sqrt(1.0 + sqr(absb / absa)) :
			(absb == 0.0 ? 0.0 : absb * sqrt(1.0 + sqr(absa / absb))));
	}
};

#endif
