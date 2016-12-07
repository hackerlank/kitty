/**
 * \file
 * \version  $Id: zBase64.cpp 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2005年02月25日 14时20分42秒 CST
 * \brief base64编码解码函数
 *
 * 
 */


#include <iostream>
#include <string>
#include "zBase64.h"
#include <strings.h>
#include <string.h>
#include <stdlib.h>

namespace Fir
{
	static const char xor_map[4] = { '#', '*', '!', 'N' };

	void base64_encrypt(const std::string &input, std::string &output)
	{
		int len = input.length();
		len = (((len + 3) / 4) * 4);
		char buffer[len];
		char dest[(len / 4) * 6 + 1]; //NULL terminate

		bzero(buffer, sizeof(buffer));
		strncpy(buffer, input.c_str(), sizeof(buffer));

		for(int i = 0; i < len; i++)
		{
			buffer[i] ^= ~xor_map[i % 4];
		}
		bzero(dest, sizeof(dest));
		for(int i = 0; i < len / 4; i++) {
			strcpy(&dest[i * 6], l64a(*(int *)(&buffer[i * 4])));
			for(int j = i * 6; j < i * 6 + 6; j++) {
				if (dest[j] == '\0') dest[j] = '.';
			}
		}

		output = dest;
	}

	void base64_decrypt(const std::string &input, std::string &output)
	{
		int len = input.length();
		len = (((len + 5) / 6) * 6);
		char buffer[len];
		char dest[(len / 6) * 4 + 1]; //NULL terminate

		bzero(buffer, sizeof(buffer));
		strncpy(buffer, input.c_str(), sizeof(buffer));

		bzero(dest, sizeof(dest));
		for(int i = 0; i < len / 6; i++) {
			*((int *)(&dest[i * 4])) = a64l(&buffer[i * 6]);
		}
		for(int i = 0; i < (len / 6) * 4; i++)
		{
			dest[i] ^= ~xor_map[i % 4];
		}

		output = dest;
	}
};

