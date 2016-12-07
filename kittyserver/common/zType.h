/**
 * \file
 * \version  $Id: zType.h 15 2013-04-10 10:29:09Z  $
 * \author  ,@163.com
 * \date 2004年12月15日 19时16分10秒 CST
 * \brief 定义基本类型
 *
 * 
 */

#ifndef _zType_h_
#define _zType_h_

#include <time.h>
#include <strings.h>

#define SAFE_DELETE(x) { if (x) { delete (x); (x) = NULL; } }
#define SAFE_DELETE_VEC(x) { if (x) { delete [] (x); (x) = NULL; } }

#define GET_HERO_CARD_ID(x,y) x * 1000000 + y

//消息类型
enum MSG_TYPE
{
    PROTOBUF_TYPE = 0,   //protobuf消息
    STRUCT_TYPE  = 1,    //c++消息
};


/**
 * \brief 单字节无符号整数
 *
 */
typedef unsigned char BYTE;
typedef unsigned char byte;

/**
 * \brief 双字节无符号整数
 *
 */
typedef unsigned short WORD;

/**
 * \brief 双字节符号整数
 *
 */
typedef signed short SWORD;

/**
 * \brief 四字节无符号整数
 *
 */
typedef unsigned int DWORD;

/**
 * \brief 四字节符号整数
 *
 */
typedef signed int SDWORD;

/**
 * \brief 八字节无符号整数
 *
 */
typedef unsigned long QWORD;

/**
 * \brief 八字节符号整数
 *
 */
typedef signed long SQWORD;

/**
 * \brief 名字的最大长度
 */
#define MAX_NAMESIZE 32

/**
 * \brief MAC地址的最大长度
 */
#define MAX_MAC_ADDR 24

/**
 * \brief UUID最大长度
 */
#define MAX_IPHONE_UUID 40

/**
 * \brief 账号最大长度
 */
#define MAX_ACCNAMESIZE	48

/**
 * \brief IP地址最大长度
 *
 */
#define MAX_IP_LENGTH	16

#define RES_PATH_LENGTH 100

// 最大平台字符串长度
#define MAX_FLAT_LENGTH 100

/**
 * \brief 网关最大容纳用户数目
 *
 */
#define MAX_GATEWAYUSER 4000	

/**
 * \brief 密码最大长度
 *
 */
#define MAX_PASSWORD  16

/**
 * \brief 屏宽
 */
#define SCREEN_WIDTH 13

/**
 * \brief 屏高
 */
#define SCREEN_HEIGHT 19

#define MAX_HOSTSIZE	32
#define MAX_USERSIZE	32
#define MAX_DBSIZE		32

#define MAX_MEM_DB 10
#define MAX_TABLE_LIST 1000

/**
 * \brief 连接线程池的状态标记位
 *
 */
enum {
	state_none	=	0,							/**< 空的状态 */
	state_maintain	=	1 << 0,						/**< 维护中，暂时不允许建立新的连接 */
	state_normal_maintain   =       2,                                      /**< 正常维护中，暂时不允许建立新的连接 */
};

/**
 * \brief 数字密码
 */
#ifndef MAX_NUMPASSWORD
#define MAX_NUMPASSWORD	32
#endif

#pragma pack(1)
/**
 * \brief 定义游戏区
 * 对游戏进行分类，然后在同种游戏中再分区
 */
struct GameZone_t
{
	union
	{
		/**
		 * \brief 唯一编号
		 */
		DWORD id;
		struct
		{
			/**
			 * \brief 游戏分区编号
			 */
			WORD zone;
			/**
			 * \brief 游戏种类编号
			 */
			WORD game;
		};
	};

	GameZone_t()
	{
		this->game = 0;
		this->zone = 0;
	}
	GameZone_t(const GameZone_t &gameZone)
	{
		this->id = gameZone.id;
	}
	GameZone_t & operator= (const GameZone_t &gameZone)
	{
		this->id = gameZone.id;
		return *this;
	}
	bool operator== (const GameZone_t &gameZone) const
	{
		return this->id == gameZone.id;
	}
};


struct _null_cmd_ 
{
	explicit _null_cmd_ (const unsigned short id) : _id(id)
	{
	}


	_null_cmd_(const BYTE _cmd, const BYTE _para) : cmd(_cmd), para(_para)
	{
	}

	union
	{
		unsigned short	_id;

        struct
        {
            unsigned char byCmd;
            unsigned char byParam;
        };
        
        struct
        {
            unsigned char cmd; 
            unsigned char para;
        };

	};
};


#define paserMessage(msg,msglen,pat)  \
do \
{ \
	va_list ap; \
	bzero(msg, msglen); \
	va_start(ap, pat);      \
	vsnprintf(msg, msglen - 1, pat, ap);    \
	va_end(ap); \
}while(false)

//卡牌发送长度 
#define T_OBJECT_LENGTH  2000

#define MAX_UZLIB_CHAR  (400 * 1024) 

#pragma pack()
#endif

