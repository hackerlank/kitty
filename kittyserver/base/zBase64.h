/**
 * \file
 * \version  $Id: zBase64.h 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2005年02月25日 14时20分42秒 CST
 * \brief base64编码解码函数
 *
 * 
 */


#ifndef _zBase64_h_
#define _zBase64_h_

#include <crypt.h>
#include <string>

namespace Fir
{
	extern void base64_encrypt(const std::string &input, std::string &output);
	extern void base64_decrypt(const std::string &input, std::string &output);
};

#endif

