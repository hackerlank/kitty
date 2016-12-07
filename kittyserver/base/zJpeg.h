/**
 * \file	zJpeg.h
 * \version  	$Id: zJpeg.h 13 2013-03-20 02:35:18Z  $
 * \author  	, @ztgame.com 
 * \date 	2007年04月14日 16时20分31秒 CST
 * \brief 	png格式的验证码生成器
 *
 * 
 */

#ifndef _zJpeg_h_
#define _zJpeg_h_

namespace Fir
{
		void *jpegPassport(char *buffer, const int buffer_len, int *size);
};

#endif

