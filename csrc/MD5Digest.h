#ifndef __MD5_DIGEST_H__
#define __MD5_DIGEST_H__

#define MD5_LENGTH 32
#define MD5_LENGTH_0 (MD5_LENGTH + 1)

struct MD5Digest
{
	char data[MD5_LENGTH_0];
	MD5Digest(const char *input, int size);
};

#endif
