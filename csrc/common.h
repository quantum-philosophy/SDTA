#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdexcept>
#include <vector>
#include <list>
#include <utility>
#include <limits>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <stdlib.h>
#include <string.h>

#include "Random.h"

#define __DELETE(some) 			\
	do { 						\
		if (some) delete some; 	\
		some = NULL;			\
	} while(0)

#ifdef REAL_RANK
typedef double rank_t;
#else
typedef int rank_t;
#endif

typedef int tid_t;
typedef int pid_t;

class Task;
class Graph;

class Processor;
class Architecture;

class Schedule;
class BasicListScheduler;

class Hotspot;
class Lifetime;

template<class CT, class PT, class ST>
class Evolution;

class Evaluation;

typedef std::vector<Task *> task_vector_t;
typedef std::vector<Processor *> processor_vector_t;

typedef std::vector<rank_t> priority_t;

typedef std::vector<bool> bit_string_t;
typedef std::vector<size_t> order_t;

#ifdef REAL_RANK
class layout_t;
class mapping_t;

class mapping_t: public std::vector<pid_t>
{
	public:

	mapping_t(size_t _size = 0) : std::vector<pid_t>(_size) {}
	mapping_t(size_t _size, pid_t _init) : std::vector<pid_t>(_size, _init) {}

	operator layout_t() const;
};

class layout_t: public std::vector<rank_t>
{
	public:

	layout_t(size_t _size = 0) : std::vector<rank_t>(_size) {}
	layout_t(size_t _size, rank_t _init) : std::vector<rank_t>(_size, _init) {}

	operator mapping_t() const;
};
#else
typedef std::vector<pid_t> mapping_t;
typedef std::vector<rank_t> layout_t;
#endif

struct method_t
{
	std::string name;
	double ratio;

	method_t(const std::string &_name, double _ratio) :
		name(_name), ratio(_ratio) {}
};

struct method_list_t: private std::vector<method_t>
{
	double ratio;
	size_t count;

	method_list_t() : ratio(0), count(0) {}

	inline void add(const method_t &method)
	{
		push_back(method);
		ratio += method.ratio;
		count++;
	}

	inline size_t choose() const
	{
		size_t i = 0;

		if (count > 1) {
			double roulette = Random::uniform(ratio);
			while ((roulette -= (*this)[i].ratio) > 0) i++;
		}
#ifndef SHALLOW_CHECK
		else if (!count) throw std::runtime_error("There are no methods.");
#endif

		return i;
	}

	using std::vector<method_t>::operator[];
	using std::vector<method_t>::size;
};

/******************************************************************************/
/* Rate                                                                       */
/******************************************************************************/

struct rate_t
{
	rate_t(double _min_rate, double _scale, double _exponent,
		const size_t &_step) :
		min_rate(_min_rate), scale(_scale), exponent(_exponent),
		step(_step)
	{
		if (min_rate < 0 || min_rate > 1)
			std::runtime_error("The minimal rate is invalid.");
	}

	inline double get() const
	{
		return std::max(min_rate, scale * std::exp(exponent * (double)step));
	}

	private:

	const double min_rate;
	const double scale;
	const double exponent;
	const volatile size_t &step;
};

/******************************************************************************/
/* Evaluation                                                                 */
/******************************************************************************/

struct price_t
{
	double lifetime;
	double energy;

	price_t() : lifetime(0), energy(0) {}
	price_t(double _lifetime, double _energy) :
		lifetime(_lifetime), energy(_energy) {}

#ifdef VERIFY_CACHING
	inline bool operator!=(const price_t &another)
	{
		return lifetime != another.lifetime || energy != another.energy;
	}
#endif
};

/******************************************************************************/
/* Constrains                                                                 */
/******************************************************************************/

class Constrain;

struct constrain_t
{
	rank_t max;
	rank_t min;
	bit_string_t peers;

	constrain_t() : max(-1), min(-1) {}

	inline rank_t random() const
	{
		return min + Random::uniform(max - min);
	}

	inline bool tight() const
	{
		return (max - min) <= 1;
	}

	inline bool has_peer(size_t id) const
	{
#ifndef SHALLOW_CHECK
		if (id >= peers.size())
			throw std::runtime_error("Cannot find the peer.");
#endif

		return peers[id];
	}
};

class constrains_t: public std::vector<constrain_t>
{
	template<class CT, class PT, class ST>
	friend class Evolution;

	bool m_fixed_layout;
	layout_t layout;

	public:

	constrains_t(size_t _size = 0) :
		std::vector<constrain_t>(_size), m_fixed_layout(false) {}

	inline void set_layout(const layout_t &layout)
	{
		this->layout = layout;
		m_fixed_layout = true;
	}

	inline const layout_t &get_layout() const
	{
		return layout;
	}

	inline bool fixed_layout() const
	{
		return m_fixed_layout;
	}
};

/******************************************************************************/
/* System                                                                     */
/******************************************************************************/

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
	priority_t priority;
	double deadline;

	system_t(const std::string &filename);
};

/******************************************************************************/
/* Calculations                                                               */
/******************************************************************************/

typedef std::vector<double> vector_t;

class matrix_t: public vector_t
{
	size_t m_rows;
	size_t m_cols;

	public:

	matrix_t(size_t _rows = 0, size_t _cols = 0) :
		vector_t(_rows * _cols, 0), m_rows(_rows), m_cols(_cols) {}

	inline double *operator[] (unsigned int row)
	{
		return &vector_t::operator[](m_cols * row);
	}

	inline const double *operator[] (unsigned int row) const
	{
		return &vector_t::operator[](m_cols * row);
	}

	inline double *pointer()
	{
		return &vector_t::operator[](0);
	}

	inline const double *pointer() const
	{
		return &vector_t::operator[](0);
	}

	inline void resize(size_t _rows, size_t _cols)
	{
		m_rows = _rows; m_cols = _cols;
		vector_t::resize(_rows * _cols);
	}

	inline void resize(const matrix_t &another)
	{
		resize(another.m_rows, another.m_cols);
	}

	inline size_t rows() const
	{
		return m_rows;
	}

	inline size_t cols() const
	{
		return m_cols;
	}

	inline void nullify()
	{
		memset(pointer(), 0, sizeof(double) * m_rows * m_cols);
	}
};

/******************************************************************************/
/* Output                                                                     */
/******************************************************************************/

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

std::ostream &operator<< (std::ostream &o, const price_t &price);
std::ostream &operator<< (std::ostream &o, const constrain_t &constrain);

/******************************************************************************/
/* Comparisons                                                                */
/******************************************************************************/

template<class T>
class Comparator
{
	public:

	static bool pair(const std::pair<double, T> &first,
		const std::pair<double, T> &second)
	{
		return first.first < second.first;
	}
};

#endif
