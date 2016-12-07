/**
 * \file
 * \version  $Id: zJpeg.cpp 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2005年04月13日 15时36分29秒 CST
 * \brief png格式的验证码生成器
 */


#include <gd.h>
#include <gdfontl.h>
#include <gdfonts.h>

#include "zMisc.h"
#include "zJpeg.h"

namespace Fir
{
	static unsigned char hexchars[] = "0123456789abcdef";

	void *jpegPassport(char *buffer, const int buffer_len, int *size)
	{
		int i;

		//生成图片
		gdImagePtr im;
		im = gdImageCreate(100, 20);
		if (im)
		{
			int white = gdImageColorAllocate(im, 0xff, 0xdf, 0xdf);  
			int confuseColor = gdImageColorAllocate(im, 0xe9, 0x00, 0x3b);  
			int textColor = gdImageColorAllocate(im, 0xe9, 0x0e, 0x5b);  

			gdImageFill(im, 0, 0, white);
			for(i = 0; i < 20; i++)
			{
				int cx = zMisc::randBetween(20, 80);
				int cy = zMisc::randBetween(2, 16);
				gdImageSetPixel(im, cx, cy, confuseColor);
			}

			//生成验证码
			for(i = 0; i < buffer_len - 1; i++)
			{
				buffer[i] = hexchars[zMisc::randBetween(0, 15)];
				gdImageChar(im,
						zMisc::randBetween(0, 1) ? gdFontGetLarge() : gdFontGetSmall(),
						zMisc::randBetween(10, 15) + i * 12,
						zMisc::randBetween(0, 5),
						buffer[i],
						textColor);
			}
			buffer[i] = '\0';

			/* Write JPEG using default quality */
			void *ret = gdImageJpegPtr(im, size, zMisc::randBetween(85, 95));

			gdImageDestroy(im);

			return ret;
		}
		else
			return NULL;
	}
};

