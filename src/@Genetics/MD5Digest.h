#ifndef __MD5_DIGEST_H__
#define __MD5_DIGEST_H__

#include <iostream>
#include <vector>

class MD5Digest {
	public:

	unsigned char data[16];

	MD5Digest(const char *input, int size);
	MD5Digest(const std::vector<int> &vector);

	private:

	inline void calc(const char *input, int size);
};

class MD5DigestComparator
{
	public:

	inline bool operator()(const MD5Digest &one, const MD5Digest &another) const
	{
		for (int i = 0; i < 16; i++)
			if (one.data[i] != another.data[i]) return true;

		return false;
	}
};

std::ostream &operator<< (std::ostream &o, const MD5Digest &digest);

#endif
