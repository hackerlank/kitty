#pragma once
#include "vector"
#include "map"
#include "Fir.h"
#include "zType.h"
#include "zSingleton.h"
#include "zXMLParser.h"
#include <ft2build.h>
#include "zMisc.h"
#include FT_FREETYPE_H
#define EQUATION_IMAGESIZE		384	
#define EQUATION_SUBIMAGESIZE		64	
#define EQUATION_IMAGEBUFSIZE		384 * 192	
#define EQUATION_SUBIMAGEBUFSIZE	EQUATION_SUBIMAGESIZE*EQUATION_SUBIMAGESIZE
struct stWord
{
	int		width;
	int		height;
	std::vector<BYTE> lpBuf;
};
	
struct stWordVector
{
	WORD value;
	std::vector<stWord>	array;
	
	stWordVector()
	{
		value = 0;
	}

	stWord* getWord()
	{
		if (!array.size()) return NULL;
		return &(array[zMisc::randBetween(0, array.size() - 1)]);
	}
};

struct Equation{
	int first; // 第3个数字
	int second; // 第2个数字
	char op; // + - * 
	Equation()
	{
		first = 0;
		second = 0;
		op = '*';
	}
	/*
	 * 生成随机的算式
	 **/
	void defalut_random();
	/*
	 * 计算等式结果  
	 **/
	int calcResult();
	/*
	 * 将数字化为各个数字
	 * 第1个为高位数字
	 **/
	static void getNumber(int value,std::vector<std::string>& numbers);

	static void getOp(char op,std::vector<std::string> & );
};
struct stNumberImageData{
	ushort data[EQUATION_SUBIMAGEBUFSIZE];
	stNumberImageData &operator = (const stNumberImageData &data)
	{
		bcopy(data.data,this->data,EQUATION_SUBIMAGEBUFSIZE);
		return *this;
	}
	stNumberImageData()
	{
		bzero(data,EQUATION_SUBIMAGEBUFSIZE);
	}
	ushort getShort(DWORD index)
	{
		if (EQUATION_SUBIMAGEBUFSIZE > index)
		{
			return data[index];
		}
		return 0;
	}
};
struct stNumberImage{
	std::vector<stNumberImageData> images;
	bool getImage(stNumberImageData &data);
};
class EquationVerifyManager:public Fir::Singleton<EquationVerifyManager>
{
public:
	EquationVerifyManager(){}
	~EquationVerifyManager(){clear();}
	bool init();
	std::map<char,stNumberImage> numberImages;
	enum{
		__VERI_NULL__ = 0, // 空验证
		__TANK_BUF_VERI__ = 1,// 战车BUF 验证
		__TANK_MATAIN__VERI__ = 2,// 战车修理 验证
		__TANK_ATK_VERI__ = 3, // 战车攻击 验证
		__BIKE_VERI__ = 4,
	};
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

	ushort					m_ImageBuf[EQUATION_IMAGEBUFSIZE];
	ushort					m_BackgroundBuf[EQUATION_IMAGEBUFSIZE];

	BitmapHeader	m_bmpHeader;
	BitmapInfoHeader m_bmpInfoHeader;

	bool loadbmp565(const char* filename, ushort* lpBuf);
	int compressImage(BYTE* lpOut, unsigned int& outLen, BYTE* lpIn, unsigned int inLen);


	struct stEquationVerify{
		time_t lasttime; // 上次请求时间
		bool isNeed; // 检查是否需要验证
		int value; // 当前位置
		DWORD type; // 类型 
		bool delayValid; // 延迟验证
		bool isValid(const int value,DWORD nowTime,DWORD delay)
		{
			if (delayValid) return false; // 延迟验证阶段 验证无效
			if (lasttime && lasttime + delay > nowTime)
			{
				if (!isNeed) return false;
				return (value == this->value);
			}
			return false;
		}
		bool isDelayValid()
		{
			return delayValid;	
		}
		// 检查验证时间是否超过
		bool checkTimeOut(DWORD nowTime,DWORD delay)
		{
			if (lasttime && lasttime + delay >= nowTime)
				return false;
			return true;	
		}
		void clear()
		{
			value = 0;
			lasttime = 0;
			isNeed = false;
			type = __VERI_NULL__;
		}
		void incErrCount()
		{
			errCount ++;
		}
		stEquationVerify()
		{
			clear();
			errCount = 0;
			delayValid = false;
		}
		WORD errCount;
	};
	void makeImage(DWORD type, DWORD *images,WORD imageSize,BYTE *lpBuf,unsigned int& bufLen,stEquationVerify& equationVerify);
	bool getNumberImage(char c,stNumberImageData &data); 
	
	bool initializeFreeType(const char* filename, const DWORD fontsize);
	bool makeWord(stWordVector* lpWord);
	struct stFreeTypeFace
	{		
		std::string			filename;
		int					fontsize;
		FT_Library			freeTypeLib;
		FT_Face				ftFace;
		FT_GlyphSlot		ftGlyphSlot;
		
		stFreeTypeFace(const char* _filename, const int _fontsize)
			: filename(_filename), fontsize(_fontsize), freeTypeLib(NULL), ftFace(NULL), ftGlyphSlot(NULL)
		{
		}

		bool init()
		{
			if (!FT_Init_FreeType(&freeTypeLib))
			{
				if (!FT_New_Face(freeTypeLib, filename.c_str(), 0, &ftFace))
				{
					if (ftFace)
					{
						if (!FT_Select_Charmap(ftFace, FT_ENCODING_UNICODE))
						{
							if (!FT_Set_Char_Size(ftFace, 0, (FT_UInt)(fontsize << 6), 96, 96))
							{
								if (!FT_Set_Pixel_Sizes(ftFace, 0, (FT_UInt)fontsize))
								{
									ftGlyphSlot = ftFace->glyph;
									ftFace->style_flags = 0;
									return true;
								}
							}
						}
					}
				}
			}

			destroy();

			return false;

		}

		void destroy()
		{
			if (ftFace)
				FT_Done_Face(ftFace);

			if (freeTypeLib)
				FT_Done_FreeType(freeTypeLib);

			ftGlyphSlot = NULL;
			ftFace = NULL;			
			freeTypeLib = NULL;
		}
	};

	std::vector<stFreeTypeFace*>	m_freetypeFonts; // 外貌列表
	std::map<WORD,stWordVector>		m_fontBuffer; //{unicode,font}
	void transformFreetype(stFreeTypeFace* lpFreeType, FT_Vector pen);
	bool getUnicodeChar(char* in, size_t inlen, WORD* out, size_t outlen);
	bool getUnicodeChar(const char *value,stWordVector&);
	bool clear();
};
