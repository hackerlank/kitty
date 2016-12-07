/**
 * \file
 * \version  $Id: zUrl.h 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2005年04月13日 10时45分13秒 CST
 * \brief 封装一些url操作函数
 */


#ifndef _zUrl_h_
#define _zUrl_h_

#include <string>

namespace Fir
{
	char *url_encode(const char *s, int len, int *new_length);
	void url_encode(std::string &s);
	int url_decode(char *str, int len);
	void url_decode(std::string &str);
};

#endif


