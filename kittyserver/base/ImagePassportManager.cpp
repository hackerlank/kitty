#include <errno.h>
#include <stdio.h>
#include <zlib.h>
#include "zMisc.h"
#include "ImagePassportManager.h"

ImagePassportManager::ImagePassportManager()
{
	m_SubImages.reserve(SUBIMAGECOUNT);
	bzero(&m_SubImages[0], sizeof(ushort*) * SUBIMAGECOUNT);

	for (int i = 0; i < SUBIMAGECOUNT; ++i) {
		ushort* lpBuf = FIR_NEW ushort[SUBIMAGEBUFSIZE];
		bzero(lpBuf, sizeof(ushort) * SUBIMAGEBUFSIZE);
		m_SubImages[i] = lpBuf;
	}
}

ImagePassportManager::~ImagePassportManager()
{
	for (size_t i = 0; i < m_SubImages.size(); ++i) {
		if (m_SubImages[i]) {
			SAFE_DELETE_VEC(m_SubImages[i]);
			m_SubImages[i] = NULL;
		}
	}
}

bool ImagePassportManager::loadbmp565(const char* filename, ushort* lpBuf)
{
	FILE* f = fopen(filename, "r");
	if (!f) {
		Fir::logger->debug("不能读取位图文件 %s ...", filename);
		return false;
	}

	BitmapHeader&		bmpFileHeader = m_bmpHeader;
	BitmapInfoHeader&	bmpInfoHeader = m_bmpInfoHeader;

	//读取BMP文件头
	if (0 == fread(&bmpFileHeader, sizeof(bmpFileHeader), 1, f)) {
		fclose(f);
		Fir::logger->debug("读取BMP文件头错误！ %s ...", filename);
		return false;
	}

	//验证文件头
	if (0x4D42 != bmpFileHeader._type) {
		fclose(f);
		Fir::logger->debug("验证位图文件头错误！ %s ...", filename);
		return false;
	}

	//读取BMP信息头
	if (0 == fread(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, f)) {
		fclose(f);
		Fir::logger->debug("读取BMP信息头错误！ %s ...", filename);
		return false;
	}

	if (56 != bmpInfoHeader.size ||
		1 != bmpInfoHeader.planes ||
		16 != bmpInfoHeader.bitCount ||
		3 != bmpInfoHeader.compression ||
		(unsigned int)(bmpInfoHeader.width * bmpInfoHeader.height * 2 + 2) != bmpInfoHeader.dataSize) {
			fclose(f);
			Fir::logger->debug("验证位图信息头错误！ %s ...", filename);
			return false;
		}

		//读取位图像素数据
			for (int y = 0; y < bmpInfoHeader.height; ++y) {
				if (0 == fread(&lpBuf[(bmpInfoHeader.height - 1 - y) * bmpInfoHeader.width], bmpInfoHeader.width * 2, 1, f)) {
					fclose(f);
					Fir::logger->debug("读取位图像素数据错误！ %s ...", filename);
					return false;
				}
			}

	fclose(f);

	return true;
}

bool ImagePassportManager::init()
{

	char imagepath[256];
	bzero(imagepath, 256);
	strncpy(imagepath, Fir::global["imagepassportpath"].c_str(), 255);

	for (int i = 0; i < SUBIMAGECOUNT; ++i) {	
		if (m_SubImages[i]) {
			char filename[256];
			snprintf(filename, sizeof(filename), "%s/%d.bmp", imagepath, i);

			if (!loadbmp565(filename, m_SubImages[i]))
				return false;
		} else
			return false;
		
	}

	char filename[256];
	snprintf(filename, sizeof(filename), "%s/background.bmp", imagepath);
	if (!loadbmp565(filename, m_BackgroundBuf))
		return false;

	Fir::seedp = time(NULL);

	Fir::logger->debug("初始化图形验证模块成功...");
	return true;
}

void ImagePassportManager::makeImage(BYTE* lpBuf, unsigned int& bufLen, int& specialX, int& specialY)
{
	bcopy(m_BackgroundBuf, m_ImageBuf, sizeof(m_ImageBuf));

	int m = zMisc::randBetween(0, SUBIMAGECOUNT-1);
	int s = zMisc::randBetween(0, SUBIMAGECOUNT-1);
	while (s == m)
		s = zMisc::randBetween(0, SUBIMAGECOUNT-1);

	ushort* lpMBuf = m_SubImages[m];
	ushort* lpSBuf = m_SubImages[s];

	ushort pixelM = zMisc::randBetween(0, 0xFFFF);
	ushort pixelS = zMisc::randBetween(0, 0xFFFF);

	for (int i = 0; i < 10; ++i) {
		int x = zMisc::randBetween(0, IMAGESIZE-SUBIMAGESIZE);
		int y = zMisc::randBetween(0, IMAGESIZE-SUBIMAGESIZE);

		for (int _y = 0; _y < SUBIMAGESIZE; ++_y) {
			for (int _x = 0; _x < SUBIMAGESIZE; ++_x) {
				if (lpMBuf[_y * SUBIMAGESIZE + _x] != 0xFFFF)
					m_ImageBuf[(y + _y) * IMAGESIZE + x + _x] = lpMBuf[_y * SUBIMAGESIZE + _x] + pixelM;
			}
		}
	}

	specialX = zMisc::randBetween(0, IMAGESIZE-SUBIMAGESIZE);
	specialY = zMisc::randBetween(0, IMAGESIZE-SUBIMAGESIZE);
	for (int _y = 0; _y < SUBIMAGESIZE; ++_y) {
		for (int _x = 0; _x < SUBIMAGESIZE; ++_x) {
			if (lpSBuf[_y * SUBIMAGESIZE + _x] != 0xFFFF)
				m_ImageBuf[(specialY + _y) * IMAGESIZE + specialX + _x] = lpSBuf[_y * SUBIMAGESIZE + _x] + pixelS;				
		}
	}

	compressImage(lpBuf, bufLen, (BYTE*)m_ImageBuf, IMAGEBUFSIZE * 2);

#ifdef _ALL_SUPER_GM
	Fir::logger->debug("图形验证数据，压缩后大小 %u byte", bufLen);
#endif
}

int ImagePassportManager::compressImage(BYTE* lpOut, unsigned int& outLen, BYTE* lpIn, unsigned int inLen)
{
	z_stream_s zipStream;
	bzero(&zipStream, sizeof(zipStream));
	zipStream.next_in = lpIn;
	zipStream.avail_in = inLen;
	zipStream.next_out = lpOut;
	zipStream.avail_out = outLen;

	int zipErr = 0;
	zipErr = deflateInit2(&zipStream, Z_BEST_SPEED, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
	if (Z_OK != zipErr)
		return zipErr;

	zipErr = deflate(&zipStream, Z_FINISH);
	if (Z_STREAM_END != zipErr) {
		deflateEnd(&zipStream);
		return (Z_OK == zipErr) ? Z_BUF_ERROR : zipErr;
	}

	outLen = zipStream.total_out;
	zipErr = deflateEnd(&zipStream);

	return zipErr;
}

