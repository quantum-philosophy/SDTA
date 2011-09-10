#ifndef __COMMON_H__
#define __COMMON_H__

#include <iostream>
#include <vector>
#include <list>

#ifdef REAL_RANK
typedef double rank_t;
#else
typedef int rank_t;
#endif

class Task;
class Graph;

class Processor;
class Architecture;

class ListScheduler;

class DynamicPower;
class Hotspot;
class Lifetime;

template<class chromosome_t, class stats_t>
class GenericGLS;

typedef int tid_t;
typedef std::vector<tid_t> schedule_t;
typedef std::vector<Task *> task_vector_t;
typedef std::vector<rank_t> priority_t;

typedef int pid_t;
typedef std::vector<pid_t> mapping_t;
typedef std::vector<Processor *> processor_vector_t;

struct system_t
{
	std::vector<unsigned int> type;
	std::vector<std::vector<bool> > link;

	std::vector<double> frequency;
	std::vector<double> voltage;
	std::vector<unsigned long int> ngate;
	std::vector<std::vector<unsigned long int> > nc;
	std::vector<std::vector<double> > ceff;

	mapping_t mapping;
	schedule_t schedule;
	priority_t priority;
	double deadline;

	system_t(const char *filename);
};

struct price_t
{
	double lifetime;
	double energy;

	price_t() : lifetime(0), energy(0) {}
	price_t(double _lifetime, double _energy) :
		lifetime(_lifetime), energy(_energy) {}
};

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

template<class T>
struct print_t
{
	const std::vector<T> &vector;

	print_t(const std::vector<T> &_vector) : vector(_vector) {}
};

template<class T>
std::ostream &operator<< (std::ostream &o, const print_t<T> &print)
{
	size_t size = print.vector.size();

	o << "[ ";
	for (size_t i = 0; i < size; i++) {
		o << print.vector[i];
		if (i < size - 1) o << ", ";
	}
	o << " ]";

	return o;
}

template<>
std::ostream &operator<< (std::ostream &o, const print_t<price_t> &print);

#endif
