#ifndef __COMMON_H__
#define __COMMON_H__

#include <mex.h>
#include <vector>
#include <list>

class Task;
class Graph;

class Processor;
class Architecture;

class ListScheduler;
class GeneticListScheduler;

class DynamicPower;

class Hotspot;

typedef int tid_t;
typedef std::vector<tid_t> schedule_t;
typedef std::vector<Task *> task_vector_t;
typedef std::vector<double> priority_t;

typedef int pid_t;
typedef std::vector<pid_t> mapping_t;
typedef std::vector<Processor *> processor_vector_t;

typedef std::vector<double> vector_t;

class matrix_t : public std::vector<double>
{
	size_t m_rows;
	size_t m_cols;

	public:

	matrix_t(size_t _rows = 0, size_t _cols = 0) :
		std::vector<double>(_rows * _cols, 0), m_rows(_rows), m_cols(_cols) {}

	inline double *operator[] (unsigned int row)
	{
		return &std::vector<double>::operator[](m_cols * row);
	}

	inline const double *operator[] (unsigned int row) const
	{
		return &std::vector<double>::operator[](m_cols * row);
	}

	inline double *pointer()
	{
		return &std::vector<double>::operator[](0);
	}

	inline const double *pointer() const
	{
		return &std::vector<double>::operator[](0);
	}

	inline void resize(size_t _rows, size_t _cols)
	{
		m_rows = _rows; m_cols = _cols;
		std::vector<double>::resize(_rows * _cols);
	}

	inline void resize(const matrix_t &another)
	{
		resize(another.m_rows, another.m_cols);
	}

	inline size_t rows() const { return m_rows; }
	inline size_t cols() const { return m_cols; }
};

std::ostream &operator<< (std::ostream &o, const std::vector<int> &v);

void dump_into_matlab(const char *filename, const char *name,
	const matrix_t &matrix);

#endif
