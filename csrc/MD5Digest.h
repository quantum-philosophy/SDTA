#ifndef __MD5_DIGEST_H__
#define __MD5_DIGEST_H__

#include <stdio.h>

#include <openssl/md5.h>

#define MD5_LENGTH (2 * MD5_DIGEST_LENGTH)
#define MD5_LENGTH_0 (2 * MD5_DIGEST_LENGTH + 1)

struct MD5Digest
{
	char data[MD5_LENGTH_0];

	MD5Digest(const unsigned char *input, size_t size)
	{
		unsigned char hash[MD5_DIGEST_LENGTH];
		MD5(input, size, hash);

		for (size_t i = 0; i < MD5_DIGEST_LENGTH; i++)
			sprintf(data + 2 * i, "%02x", hash[i]);

		data[MD5_LENGTH] = '\0';
	}
};

#endif
