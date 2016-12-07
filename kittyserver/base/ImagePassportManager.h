#ifndef _TRADEIMAGEMANAGER_H_
#define _TRADEIMAGEMANAGER_H_

#include <vector>
#include <list>
#include "Fir.h"
#include "zType.h"
#include "zSingleton.h"

#define IMAGESIZE			96
#define SUBIMAGESIZE		24
#define IMAGEBUFSIZE		IMAGESIZE*IMAGESIZE
#define SUBIMAGEBUFSIZE		SUBIMAGESIZE*SUBIMAGESIZE
#define SUBIMAGECOUNT		20

class ImagePassportManager : public Fir::Singleton<ImagePassportManager>
{
private:
	#pragma pack(1)

	struct BitmapHeader {
		ushort				_type;
		unsigned int		size;
		ushort				reserved1;
		ushort				reserved2;
		unsigned int		dataOffset;
	};

	struct BitmapInfoHeader {
		unsigned int		size;		//本结构大小
		int					width;		//位图宽
		int					height;		//位图高
		ushort				planes;		//永远为1
		ushort				bitCount;	//像素大小的位数，1\4\8\16\24\32
		unsigned int		compression;	
		unsigned int		dataSize;	//位数像素数据区域的大小
		int					xPelsPerMeter;
		int					yPelsPerMeter;
		unsigned int		clrUsed;
		unsigned int		clrImportant;
		unsigned int		mask[4];	//16位565位图，特殊掩码数据
	};

	#pragma pack()

	ushort					m_ImageBuf[IMAGEBUFSIZE];
	ushort					m_BackgroundBuf[IMAGEBUFSIZE];
	std::vector<ushort*>	m_SubImages;

	BitmapHeader	m_bmpHeader;
	BitmapInfoHeader m_bmpInfoHeader;

	bool loadbmp565(const char* filename, ushort* lpBuf);
	int compressImage(BYTE* lpOut, unsigned int& outLen, BYTE* lpIn, unsigned int inLen);

public:
	ImagePassportManager();
	virtual ~ImagePassportManager();

	bool init();

	void makeImage(BYTE* lpBuf, unsigned int& bufLen, int& specialX, int& specialY);

};

#endif
