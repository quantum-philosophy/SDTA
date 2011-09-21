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
#include <stdlib.h>

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
class ListScheduler;

class DynamicPower;
class Hotspot;
class Lifetime;

template<class CT, class PT, class ST>
class GenericEvolution;

class Evaluation;

typedef std::vector<Task *> task_vector_t;
typedef std::vector<Processor *> processor_vector_t;

typedef std::vector<rank_t> priority_t;

typedef std::vector<bool> bit_string_t;

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

/******************************************************************************/
/* Gene encoder                                                               */
/******************************************************************************/

class GeneEncoder
{
	public:

	template<class CT, class ET>
	static inline void encode(CT &chromosome, const ET &encoding)
	{
		size_t size = encoding.size();

		chromosome.resize(size);

		for (size_t i = 0; i < size; i++)
			chromosome[i] = encoding[i];
	}

	template<class CT, class ET>
	static inline void extend(CT &chromosome, const ET &encoding)
	{
		size_t offset = chromosome.size();
		size_t size = encoding.size();

		chromosome.resize(offset + size);

		for (size_t i = 0; i < size; i++)
			chromosome[offset + i] = encoding[i];
	}

	template<class CT, class ET1, class ET2>
	static inline void split(const CT &chromosome, ET1 &chunk1, ET2 &chunk2,
		size_t size1 = 0, size_t size2 = 0)
	{
		size_t length = chromosome.size();

		if (size1 == 0) {
			size1 = length / 2;
			chunk1.resize(size1);
		}
		else if (size1 > length)
			throw std::runtime_error("Cannot split.");

		if (size2 == 0) {
			size2 = length - size1;
			chunk2.resize(size2);
		}
		else if (size1 + size2 > length)
			throw std::runtime_error("Cannot split.");

		for (size_t i = 0; i < size1; i++)
			chunk1[i] = chromosome[i];

		for (size_t i = 0; i < size2; i++)
			chunk2[i] = chromosome[size1 + i];
	}
};

/******************************************************************************/
/* Rate                                                                       */
/******************************************************************************/

struct rate_t
{
	rate_t(double _min_rate, double _scale, double _exponent,
		const size_t &_step, double &_keep = dummy) :
		min_rate(_min_rate), scale(_scale), exponent(_exponent),
		step(_step), keep(_keep)
	{
		if (min_rate < 0 || min_rate > 1)
			std::runtime_error("The minimal rate is invalid.");
	}

	inline double get() const
	{
		return keep = std::max(min_rate,
			scale * std::exp(exponent * (double)step));
	}

	private:

	const double min_rate;
	const double scale;
	const double exponent;
	const volatile size_t &step;
	double &keep;

	static double dummy;
};

/******************************************************************************/
/* Random generator                                                           */
/******************************************************************************/

class Random
{
	static bool verbose;
	static int seed;

	public:

	static void set_seed(int seed, bool verbose = false)
	{
		Random::seed = seed;
		Random::verbose = verbose;
	}

	static int get_seed()
	{
		int seed = Random::seed;

		if (seed < 0) seed = time(NULL);

		if (verbose)
			std::cout << "Chosen seed: " << seed << std::endl;

		return seed;
	}

	static void reseed()
	{
		srand(get_seed());
	}

	static inline double uniform(double range = 1.0)
	{
		return range * (double)rand() / (double)RAND_MAX;
	}

	static inline int number(int range)
	{
		return double(range) * uniform();
	}

	static bool flip(double p)
	{
		return uniform() < p;
	}
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

	static price_t invalid()
	{
		return price_t(0, std::numeric_limits<double>::max());
	}
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

typedef std::vector<constrain_t> constrains_t;

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

class matrix_t: public std::vector<double>
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

/******************************************************************************/
/* Helpers                                                                    */
/******************************************************************************/

class Helper
{
	public:

	static void dump(const matrix_t &matrix, const char *filename)
	{
		std::ofstream stream(filename);

		size_t rows = matrix.rows();
		size_t cols = matrix.cols();

		for (size_t i = 0; i < rows; i++) {
			for (size_t j = 0; j < cols; j++) {
				stream << matrix[i][j];
				if (j + 1 < cols) stream << '\t';
			}
			stream << std::endl;
		}

		stream.close();
	}

	template<class T>
	static void permute(std::vector<T> &vector, const std::vector<size_t> &order)
	{
		size_t size = vector.size();

		if (size != order.size())
			throw std::runtime_error("Cannot permute the vector.");

		std::vector<T> permuted(size);

		for (size_t i = 0; i < size; i++)
			permuted[i] = vector[order[i]];

		for (size_t i = 0; i < size; i++)
			vector[i] = permuted[i];
	}
};

#endif
