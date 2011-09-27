#ifndef __MD5_DIGEST_H__
#define __MD5_DIGEST_H__

#include <vector>

#define MD5_LENGTH 32
#define MD5_LENGTH_0 (MD5_LENGTH + 1)

class MD5Digest
{
	public:

	char data[MD5_LENGTH_0];

	MD5Digest(const char *input, int size);
	MD5Digest(const std::vector<size_t> &vector);

	private:

	inline void compute(const char *input, int size);
};

#endif
