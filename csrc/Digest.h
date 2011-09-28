#ifndef __DIGEST_H__
#define __DIGEST_H__

#include <stdio.h>
#include <openssl/md5.h>

struct Digest
{
	static const size_t LENGTH = 2 * MD5_DIGEST_LENGTH;
	static const size_t LENGTH_0 = 2 * MD5_DIGEST_LENGTH + 1;

	char data[LENGTH_0];

	Digest(const unsigned char *input, size_t size)
	{
		unsigned char hash[MD5_DIGEST_LENGTH];
		MD5(input, size, hash);

		for (size_t i = 0; i < MD5_DIGEST_LENGTH; i++)
			sprintf(data + 2 * i, "%02x", hash[i]);

		data[LENGTH] = '\0';
	}
};

#endif
