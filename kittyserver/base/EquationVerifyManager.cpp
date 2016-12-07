#include "EquationVerifyManager.h"
#include <errno.h>
#include <stdio.h>
#include <zlib.h>
#include <iconv.h>
#include <math.h>
#include "zTime.h"
#include "zMisc.h"
void Equation::defalut_random()
{
	first = zMisc::randBetween(0,9);
	second = zMisc::randBetween(0,9);
	static char ops[]={'+','-','*'};
	DWORD index = zMisc::randBetween(0,sizeof(ops) - 1);
	op = ops[index];
	if (op == '-')
	{
		if (first < second)
		{
			int temp = second; 
			second = first;
			first = temp;
		}
	}
}
int Equation::calcResult()
{
	switch(op)
	{
	case '-': return first - second;
	case '+': return first + second;
	case '*': return first * second;  
	}	
	return 0;
}
void Equation::getNumber(int value,std::vector<std::string>& numbers)
{
	static std::string  tables[10][2] = {
		{"0","零"},
		{"1","一"},
		{"2","二"},
		{"3","三"},
		{"4","四"},
		{"5","五"},
		{"6","六"},
		{"7","七"},
		{"8","八"},
		{"9","九"}
	};
	std::vector<std::string> temps;
	if (!value)
	{
		numbers.push_back("0");
		return;
	}
	while (value != 0)
	{
		int temp = value % 10;
		DWORD index = zMisc::randBetween(0,1);
		std::string str = tables[temp][index];
		temps.push_back(str);
		value = value / 10;
	}
	while (temps.size())
	{
		numbers.push_back(temps.back());
		temps.pop_back();
	}
}
void Equation::getOp(char op,std::vector<std::string> &numbers)
{
	std::string table[3][2] = {
		{"+","加"},
		{"-","减"},
		{"*","乘"}
	};
	DWORD index = zMisc::randBetween(0,1);
	switch(op)
	{
		case '+':
		{
			numbers.push_back(table[0][index]);	
		}
		break;
		case '-':
		{
			numbers.push_back(table[1][index]);
		}
		break;
		case '*':
		{
			numbers.push_back(table[2][index]);
		}
		break;
	}
}
bool stNumberImage::getImage(stNumberImageData &data)
{
	if (!images.size())
	{
		return false;	
	}
	DWORD index = zMisc::randBetween(0,images.size() - 1);
        data = images[index];
	return true;	
}
bool EquationVerifyManager::getNumberImage(char c,stNumberImageData &data)
{
	std::map<char,stNumberImage>::iterator iter = numberImages.find(c);
	if (iter != numberImages.end())
	{
		return iter->second.getImage(data);
	}
	return false;
}
void EquationVerifyManager::makeImage(DWORD type, DWORD *images,WORD imageSize,BYTE *lpBuf,unsigned int& bufLen,stEquationVerify &equationVerify)
{
	bcopy(m_BackgroundBuf, m_ImageBuf, sizeof(m_ImageBuf));
	Equation equation;
	equation.defalut_random();
	std::vector<std::string> numbers;
	equation.getNumber(equation.first,numbers);
	equation.getOp(equation.op,numbers);
	equation.getNumber(equation.second,numbers);
	equationVerify.value = equation.calcResult();
	equationVerify.isNeed = true;
	equationVerify.type = type;
	std::vector<std::string> randoms;
/*	for (int i = 0; i < 24;i++)
	{
		char A = zMisc::randBetween('A','Z');
		randoms.push_back(A);
	}
*/
	std::random_shuffle(randoms.begin(), randoms.end());
	//ushort pixelM = 0;//zMisc::randBetween(0, 0xFFFF);
	DWORD offset = 100;
	for (DWORD index = 0;index < numbers.size(); index ++)
	{
		int x = zMisc::randBetween(offset * index ,offset * index + 64);
		int y = zMisc::randBetween(0,64);
		stWordVector wordvec;
		if (getUnicodeChar(numbers[index].c_str(),wordvec))
		{
			stWord* lpWord = wordvec.getWord(); // finally wanted
			if (!lpWord || lpWord->width == 0 || lpWord->height == 0)
				continue;
			int h = y;
			int bw = x;
			for (int _y = 0; _y < lpWord->height; ++_y)
			{
				for (int _x = 0; _x < lpWord->width; ++_x) 
				{
					if ((DWORD)_y * lpWord->width + _x >= lpWord->lpBuf.size())
					{
						break;	
					}
					BYTE a = lpWord->lpBuf[_y * lpWord->width + _x];				
					if (a && _x + bw <= EQUATION_IMAGESIZE && _y + h <= EQUATION_IMAGESIZE) 
					{	
						m_ImageBuf[(_y + h) * EQUATION_IMAGESIZE + _x + bw] = 0;  //a ? (a >> 4 << 12) | 0xFFFF : 0;
					}
				}
			}
		}
		else
		{
#ifdef _ALL_SUPER_GM
			Fir::logger->debug("算式验证数据，获取 %s 失败", numbers[index].c_str());
#endif
		}
	}

	DWORD count = zMisc::randBetween(0,2);
	for (DWORD index = 0;index < count && index < randoms.size(); index ++)
	{
		int x = zMisc::randBetween(0,320);
		int y = zMisc::randBetween(0,180);
		stWordVector wordvec;
		getUnicodeChar(randoms[index].c_str(),wordvec);
		stWord* lpWord = wordvec.getWord(); // finally wanted
		if (!lpWord || lpWord->width == 0 || lpWord->height == 0)
			continue;
		int h = y;
		int bw = x;
		for (int _y = 0; _y < lpWord->height; ++_y)
		{
			for (int _x = 0; _x < lpWord->width; ++_x) 
			{
				if ((DWORD)_y * lpWord->width + _x >= lpWord->lpBuf.size())
				{
					break;	
				}
				BYTE a = lpWord->lpBuf[_y * lpWord->width + _x];				
				if (a && _x + bw <= EQUATION_IMAGESIZE && _y + h <= EQUATION_IMAGESIZE) 
				{	
					m_ImageBuf[(_y + h) * EQUATION_IMAGESIZE + _x + bw] = 0;  //a ? (a >> 4 << 12) | 0xFFFF : 0;
				}
			}
		}
	}

	bufLen =  (EQUATION_IMAGEBUFSIZE * 2) +  (EQUATION_IMAGEBUFSIZE * 2) / 5 + 12;
	compressImage(lpBuf, bufLen, (BYTE*)m_ImageBuf, EQUATION_IMAGEBUFSIZE * 2);
#ifdef _ALL_SUPER_GM
	Fir::logger->debug("算式验证数据，压缩后大小 %u byte %lu", bufLen,numbers.size());
#endif

}
bool EquationVerifyManager::loadbmp565(const char* filename, ushort* lpBuf)
{
	FILE* f = fopen(filename, "r");
	if (!f) {
		Fir::logger->debug("不能读取位图文件 %s ...", filename);
		return false;
	}

	BitmapHeader		bmpFileHeader ;//= m_bmpHeader;
	BitmapInfoHeader	bmpInfoHeader ;// = m_bmpInfoHeader;

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
		DWORD index = (bmpInfoHeader.height - 1 - y) * bmpInfoHeader.width;
		if (0 == fread(&lpBuf[index], bmpInfoHeader.width * 2, 1, f)) {
			fclose(f);
			Fir::logger->debug("读取位图像素数据错误！ %s ...", filename);
			return false;
		}
	}

	fclose(f);

	return true;
}
int EquationVerifyManager::compressImage(BYTE* lpOut, unsigned int& outLen, BYTE* lpIn, unsigned int inLen)
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

bool EquationVerifyManager::clear()
{
	for (std::vector<stFreeTypeFace*>::iterator it = m_freetypeFonts.begin(); it != m_freetypeFonts.end(); ++it)
	{
		(*it)->destroy();
		SAFE_DELETE(*it);
	}
	m_freetypeFonts.clear();

	m_fontBuffer.clear();

	return true;
}
bool EquationVerifyManager::init()
{
	bzero(m_ImageBuf,EQUATION_IMAGEBUFSIZE);
	bzero(m_BackgroundBuf,EQUATION_IMAGEBUFSIZE);
	char imagepath[256];
	bzero(imagepath, 256);
	strncpy(imagepath, Fir::global["imagepassportpath"].c_str(), 255);

	char filename[256];
	snprintf(filename, sizeof(filename), "%s/new_background.bmp", imagepath);
	if (!loadbmp565(filename, m_BackgroundBuf))
		return false;
	zXMLParser xml;
	if (!xml.initFile(Fir::global["configdir"] + "equationVerifyconfig.xml")) {
		Fir::logger->error("[算式图形验证码]打开 equationVerifyconfig.xml 配置文件失败");
		return false;
	}

	xmlNodePtr root = xml.getRootNode("config");
	if (!root)
		return false;
	xmlNodePtr fontLibraryNode = xml.getChildNode(root, "FontLibrary");
	if (fontLibraryNode) 
	{
		xmlNodePtr infomationNode = xml.getChildNode(fontLibraryNode, "Infomation");
		while (infomationNode)
		{
			std::string filename;
			DWORD fontsize = 0;				
			xml.getNodePropStr(infomationNode, "fontFilename", filename);
			xml.getNodePropNum(infomationNode, "fontSize", &fontsize, sizeof(fontsize));
			filename = Fir::global["imagepassportpath"] + "/" + filename;
			initializeFreeType(filename.c_str(), fontsize);
#ifdef ALL_SUPER_GM 
			break;
#endif
			infomationNode = xml.getNextNode(infomationNode, "Infomation");
		}
	}


	xmlNodePtr numberPtr = xml.getChildNode(root,"number");
	DWORD count = 0;
	while (numberPtr)
	{
		std::string numberImage="";	
		xml.getNodePropStr(numberPtr,"image",numberImage);
		char filename[256]={'\0'};
		stNumberImageData data;

		if (!numberImage.size() && numberImage != "")
		{
			snprintf(filename, sizeof(filename), "%s/%s.bmp", imagepath,numberImage.c_str());
			if (!loadbmp565(filename, &data.data[0]))
			{
				Fir::logger->trace("[算式图形验证] 加载 %s 失败 ",filename);
				return false;
			}
		}
		else
		{
			Fir::logger->trace("[算式图形验证] 加载 %s 失败 ",filename);
		}
		std::string tag;
		xml.getNodePropStr(numberPtr,"value",tag);
		if (!tag.empty())
		{
			char value =(char) *tag.c_str();
			numberImages[value].images.push_back(data);	
		}
		count ++;
		numberPtr = xml.getNextNode(numberPtr,"number");
	}	
	Fir::logger->trace("[算式图形验证] 加载 %lu 组数据 %u个字符",numberImages.size(),count);
	return true;
}
bool EquationVerifyManager::getUnicodeChar(const char *instr,stWordVector& word)
{
	WORD out[24]={'\0'};
	size_t inlen = strlen(instr);

	if (getUnicodeChar((char*)instr,inlen,out,24))
	{
		word.value = out[1];
		std::map<WORD,stWordVector>::iterator iter = m_fontBuffer.find(out[1]);
		if (iter == m_fontBuffer.end())	
		{
			makeWord(&word);
			m_fontBuffer[out[1]] = word;
		}
		else
		{
			word = iter->second;
		}
		return true;
	}
	return false;
}
bool EquationVerifyManager::getUnicodeChar(char* in, size_t inlen, WORD* out, size_t outlen)
{
	iconv_t cd = iconv_open("UNICODE", "GB2312");
	if (cd == (iconv_t)-1)
		return false;

	bzero(out, sizeof(WORD) * outlen);

	size_t outbuflen = outlen;

	if ((size_t)-1 == iconv(cd, &in, &inlen, (char**)&out, &outbuflen))
	{
		iconv_close(cd);
		return false;
	}
	iconv_close(cd);
	
	return true;
}
void EquationVerifyManager::transformFreetype(stFreeTypeFace* lpFreeType, FT_Vector pen)
{
	//FT_GlyphSlot slot;
	FT_Matrix	 matrix;

	//slot = lpFreeType->ftFace->glyph;
	
	double angle1 = cos(zMisc::randBetween(0, 5));
	matrix.xx = (FT_Fixed)(zMisc::randBetween(0x15000L, 0x25000L));
	matrix.xy = (FT_Fixed)(angle1 * zMisc::randBetween(0x5000L, 0x12000L));
	matrix.yx = (FT_Fixed)(angle1 * zMisc::randBetween(0x5000L, 0x12000L));
	matrix.yy = (FT_Fixed)(zMisc::randBetween(0x15000L, 0x25000L));

	FT_Set_Transform(lpFreeType->ftFace, &matrix, &pen);
}

bool EquationVerifyManager::makeWord(stWordVector* lpWord)
{
	if (!lpWord) return false;
	if (m_freetypeFonts.empty()) return false;
	//WORD index = zMisc::randBetween(0,m_freetypeFonts.size());
	stFreeTypeFace* lpFreeType = m_freetypeFonts[0];
	if (lpFreeType && lpFreeType->init())
	{
		FT_Vector pen = { 30 * 64, (96 - 20) * 64 };
		transformFreetype(lpFreeType, pen);

		if (!FT_Load_Char(lpFreeType->ftFace, lpWord->value, FT_LOAD_RENDER))
		{
			FT_Bitmap& bitmap = lpFreeType->ftGlyphSlot->bitmap;
			stWord word;
			word.width = bitmap.width;
			word.height =  bitmap.rows;
			word.lpBuf.resize(bitmap.width * bitmap.rows);
			for (int _y = 0; _y < bitmap.rows; ++_y)
			{
				for (int _x = 0; _x < bitmap.width; ++_x) 
				{
					word.lpBuf[_y * bitmap.width + _x] = bitmap.buffer[_y * bitmap.pitch + _x];			
				}
			}

			lpWord->array.push_back(word);			
		}

		lpFreeType->destroy();
		return true;
	}
	return false;
}
bool EquationVerifyManager::initializeFreeType(const char* filename, const DWORD fontsize)
{
	stFreeTypeFace*  lpFreeTypeFace = FIR_NEW stFreeTypeFace(filename, fontsize);
	m_freetypeFonts.push_back(lpFreeTypeFace);
	return true;
}

