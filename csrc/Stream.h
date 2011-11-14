#ifndef __STREAM_H__
#define __STREAM_H__

class InputStream
{
	static const size_t line_size = 65536;
	static const size_t max_units = 8192;
	static const size_t chunk_size = 1000;

	FILE *file;
	size_t unit_count;

	public:

	InputStream(const char *filename) : unit_count(0)
	{
		file = fopen(filename, "r");

		if (!file)
			throw std::runtime_error("Cannot open the input stream.");

		read_header();
	}

	InputStream(const std::string &filename) : unit_count(0)
	{
		file = fopen(filename.c_str(), "r");

		if (!file)
			throw std::runtime_error("Cannot open the input stream.");

		read_header();
	}

	~InputStream()
	{
		if (file) fclose(file);
	}

	inline size_t size() const
	{
		return unit_count;
	}

	bool read(double *value)
	{
		int result;

		for (size_t i = 0; i < unit_count; i++) {
			result = fscanf(file, "%lf", &value[i]);

			if (result == EOF) {
				if (i > 0)
					throw std::runtime_error("The stream file is invalid.");
				return false;
			}
			else if (result <= 0) {
				throw std::runtime_error("The stream file is invalid.");
			}
		}

		return true;
	}

	void read(matrix_t &matrix)
	{
		read(matrix, unit_count);
	}

	void read(matrix_t &matrix, size_t cols)
	{
		if (cols < unit_count)
			throw std::runtime_error("The number of columns is invalid.");

		size_t chunks = 1;

		matrix.resize(chunk_size, cols);

		size_t i = 0;

		while (read(matrix[i])) {
			i++;

			if (i >= chunks * chunk_size) {
				chunks++;
				matrix.extend(chunks * chunk_size);
			}
		}

		matrix.shrink(i);
	}

	private:

	void read_header()
	{
		char line[line_size];

		/* Skip the header */
		if (fgets(line, line_size, file) <= 0)
			throw std::runtime_error("The stream file is invalid.");

		char *token = strtok(line, " \r\t\n");

		while (token) {
			unit_count++;

			if (unit_count > max_units)
				throw std::runtime_error("Too many units.");

			token = strtok(NULL, " \r\t\n");
		}
	}
};

class OutputStream
{
	FILE *file;
	size_t unit_count;

	public:

	OutputStream(const char *filename, size_t _unit_count) :
		unit_count(_unit_count)
	{
		file = fopen(filename, "w");

		if (!file)
			throw std::runtime_error("Cannot open the output stream.");
	}

	OutputStream(const std::string &filename, size_t _unit_count) :
		unit_count(_unit_count)
	{
		file = fopen(filename.c_str(), "w");

		if (!file)
			throw std::runtime_error("Cannot open the output stream.");
	}

	~OutputStream()
	{
		if (file) fclose(file);
	}

	void write(const double *value)
	{
		for (size_t i = 0; i < unit_count; i++)
			fprintf(file, "%lf\t", value[i]);

		fprintf(file, "\n");
	}

	void write(const matrix_t &matrix)
	{
		if (matrix.cols() != unit_count)
			throw std::runtime_error("The stream matrix is invalid.");

		size_t rows = matrix.rows();
		for (size_t i = 0; i < rows; i++) write(matrix[i]);
	}
};

#endif
