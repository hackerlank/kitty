#ifndef _COMMAND_H
#define _COMMAND_H
#include "zNullCmd.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable : 4200)
#pragma warning(disable : 4201)
#endif

#define MAX_CHATINFO	256
#define MAX_LEAVEMESSAGE    150
#define MAX_LEAVEMESSAGENUM    50  

#define BINARY_VERSION      20060313
#pragma pack(1)

#ifdef _USE_CMD_NAMESPACE
#define _CMD_NAMESPACE_BEGIN namespace CMD {
#define _CMD_NAMESPACE_END	 };
#else
#define _CMD_NAMESPACE_BEGIN 
#define _CMD_NAMESPACE_END	
#endif

_CMD_NAMESPACE_BEGIN

//BEGIN_ONE_CMD

/// 空指令
const BYTE NULL_USERCMD			= 0;
/// 时间指令
const BYTE TIME_USERCMD			= 2;


//BEGIN_ONE_CMD
//////////////////////////////////////////////////////////////
// 空指令定义开始
//////////////////////////////////////////////////////////////

struct stTimerUserCmd : public stNullUserCmd
{
       stTimerUserCmd()
       {
               cmd = TIME_USERCMD;
       }
};

/// 网关向用户发送游戏时间
const BYTE GAMETIME_TIMER_USERCMD_PARA = 1;
struct stGameTimeTimerUserCmd : public stTimerUserCmd 
{
	stGameTimeTimerUserCmd()
	{
		para = GAMETIME_TIMER_USERCMD_PARA;
	}

	QWORD qwGameTime;			/**< 游戏时间 */
};

/// 网关向用户请求时间
const BYTE REQUESTUSERGAMETIME_TIMER_USERCMD_PARA = 2;
struct stRequestUserGameTimeTimerUserCmd : public stTimerUserCmd
{
	stRequestUserGameTimeTimerUserCmd()
	{
		para = REQUESTUSERGAMETIME_TIMER_USERCMD_PARA;
	}

};

/// 用户向网关发送当前游戏时间
const BYTE USERGAMETIME_TIMER_USERCMD_PARA  = 3;
struct stUserGameTimeTimerUserCmd : public stTimerUserCmd
{
	stUserGameTimeTimerUserCmd()
	{
		para = USERGAMETIME_TIMER_USERCMD_PARA;
	}

	DWORD dwUserTempID;			/**< 用户临时ID */
	QWORD qwGameTime;			/**< 用户游戏时间 */
};

/// 用户ping命令(服务器原样返回)
const BYTE PING_TIMER_USERCMD_PARA = 4;
struct stPingTimeTimerUserCmd : public stTimerUserCmd
{
	stPingTimeTimerUserCmd()
	{
		para = PING_TIMER_USERCMD_PARA;
	}

};

struct user_base_info
{
	QWORD serverStartTime;  //服务器启动时间
	char name[MAX_NAMESIZE];//角色名字
	BYTE sex;//性别 0为未设置 1为男 2为女
	WORD level;//当前等级
	DWORD gold;//金子
	DWORD gem;//灵石
	QWORD charid;   //玩家角色id
	DWORD dwServerCurrentTime; // 服务器当前时间
};

/// 定义物品格子类型
enum{
	OBJECTCELLTYPE_NONE,        /// 不是格子，用于丢弃或捡到物品
	OBJECTCELLTYPE_COMMON,      /// 普通物品格子
	OBJECTCELLTYPE_EQUIP,       /// 装备
	OBJECTCELLTYPE_MOUSE,       /// 鼠标
	OBJECTCELLTYPE_TRADE,       /// 自己的交易格子
	OBJECTCELLTYPE_OTHERTRADE,  /// 对方的交易格子
	OBJECTCELLTYPE_BANK,        /// 银行
	OBJECTCELLTYPE_SELL,        /// 卖
	OBJECTCELLTYPE_STORE,       /// 仓库
	OBJECTCELLTYPE_EQUIPSHOW,   /// 非自己穿着的装备
	OBJECTCELLTYPE_PACKAGE,    /// 包裹的格子
	OBJECTCELLTYPE_MAKE,       /// 合成、升级，镶嵌的格子
	OBJECTCELLTYPE_MYSHOP,      /// 自己摊位的格子
	OBJECTCELLTYPE_OTHERSSHOP,  /// 别的玩家摊位的格子
	OBJECTCELLTYPE_MAIL,        /// 邮件系统的格子
	OBJECTCELLTYPE_COUNTRY_SAVEBOX, /// 国家倉库
	OBJECTCELLTYPE_PET,       /// 宠物包裹
	OBJECTCELLTYPE_GIFTBOX,     //宝盒的格子
	OBJECTCELLTYPE_GOD_EQUIP,   //神器
	OBJECTCELLTYPE_RENT_STORE,  //租借的仓库
	OBJECTCELLTYPE_STONE = 20,  //改造千年寒铁
	OBJECTCELLTYPE_GIFT = 21,   //新礼包的物品格子
	OBJECTCELLTYPE_NEWBOX = 23, //新宝箱的物品格子 
	OBJECTCELLTYPE_GOUHUO = 24, //篝火鉴定酒 
	OBJECTCELLTYPE_PVP = 25,    //PVP包裹
	OBJECTCELLTYPE_SECRET = 26, //秘境包裹
	OBJECTCELLTYPE_TIANMA_EQUIP = 256,          // 天马合成格子
};


_CMD_NAMESPACE_END

#pragma pack()

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif

