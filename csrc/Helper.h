#ifndef __HELPER_H__
#define __HELPER_H__

#include <sstream>

#include "common.h"

class Helper
{
	public:

	static void dump(const matrix_t &matrix, const char *filename)
	{
		dump(matrix.pointer(), matrix.rows(), matrix.cols(), filename);
	}

	static void dump(const double *matrix, size_t rows, size_t cols,
		const char *filename)
	{
		std::ofstream stream(filename);

		for (size_t i = 0; i < rows; i++) {
			for (size_t j = 0; j < cols; j++) {
				stream << matrix[i * cols + j];
				if (j + 1 < cols) stream << '\t';
			}
			stream << std::endl;
		}

		stream.close();
	}

	template<class T>
	static void permute(std::vector<T> &vector, const order_t &order)
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

	template<class T>
	static void permute(T *vector, const order_t &order)
	{
		size_t size = order.size();

		std::vector<T> permuted(size);

		for (size_t i = 0; i < size; i++)
			permuted[i] = vector[order[i]];

		for (size_t i = 0; i < size; i++)
			vector[i] = permuted[i];
	}

	template<class T>
	static void prioritize(std::vector<T> &vector, const priority_t &priority)
	{
		size_t size = vector.size();

		if (size != priority.size())
			throw std::runtime_error("Cannot permute the vector.");

		std::vector<std::pair<double, T> > permuted(size);

		for (size_t i = 0; i < size; i++)
			permuted[i] = std::pair<double, T>(priority[i], vector[i]);

		std::stable_sort(permuted.begin(), permuted.end(), Comparator<T>::pair);

		for (size_t i = 0; i < size; i++)
			vector[i] = permuted[i].second;
	}

	static void chomp(std::string &line)
	{
		size_t found;

		std::string white_spaces(" \t\f\v\n\r");

		found = line.find_first_not_of(white_spaces);
		if (found != std::string::npos) line = line.substr(found);

		found = line.find_last_not_of(white_spaces);
		if (found != std::string::npos) line.erase(found + 1);
		else line.clear();
	}

	static method_list_t method_list(const std::string &line)
	{
		std::stringstream stream(line);

		std::vector<std::string> chunks;

		while (!stream.eof()) {
			std::string chunk;
			stream >> chunk;
			if (!chunk.empty()) chunks.push_back(chunk);
		}

		size_t count = chunks.size();

		method_list_t list;

		if (!count) return list;

		double ratio;
		int last_name = -1;

		for (size_t i = 0; i < count; i++) {
			std::stringstream substream(chunks[i]);

			if (!(substream >> ratio)) {
				/* Probably, another name */
				last_name = i;
			}
			else {
				/* Probably, the ratio of the previous method */
				if (last_name < 0)
					throw std::runtime_error("The method string is invalid.");
				list.add(method_t(chunks[last_name], ratio));

				last_name = -1;
			}
		}

		/* If we cannot find the ratio for the last one,
		 * add it with some default.
		 */
		if (last_name >= 0)
			list.add(method_t(chunks[last_name], 1));

		return list;
	}
};

#endif
