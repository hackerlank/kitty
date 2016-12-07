//玩家登录命令定义，client<->server
#ifndef _LOGIN_USER_COMMAND_NEW_H_
#define _LOGIN_USER_COMMAND_NEW_H_

#include <string>
#include "zType.h"
#include "Command.h"
#include "nullcmd.h"
#pragma pack(1)

namespace CMD
{ 

//BEGIN_ONE_CMD
//////////////////////////////////////////////////////////////
/// 定义登录命令开始
//////////////////////////////////////////////////////////////

/// 客户端验证版本
const DWORD GAME_VERSION = 1999;
struct ELEGANT_DECLARE_CMD(stUserVerifyVerCmd_CS)
{
	stUserVerifyVerCmd_CS()
	{
		version = GAME_VERSION;
	}

	DWORD reserve;	//保留字段
	DWORD version;
};

enum{
	LOGIN_RETURN_UNKNOWN = 0,                   /// 未知错误
	LOGIN_RETURN_VERSIONERROR = 1,              /// 版本错误
	LOGIN_RETURN_UUID = 2,                      /// UUID登陆方式没有实现
	LOGIN_RETURN_DB = 3,                        /// 数据库出错
	LOGIN_RETURN_PASSWORDERROR = 4,             /// 帐号密码错误
	LOGIN_RETURN_CHANGEPASSWORD = 5,            /// 修改密码成功
	LOGIN_RETURN_IDINUSE = 6,                   /// ID正在被使用中
	LOGIN_RETURN_IDINCLOSE = 7,                 /// ID被封
	LOGIN_RETURN_GATEWAYNOTAVAILABLE = 8,       /// 网关服务器未开
	LOGIN_RETURN_USERMAX = 9,                   /// 用户满
	LOGIN_RETURN_ACCOUNTEXIST = 10,              /// 账号已经存在
	LOGIN_RETURN_ACCOUNTSUCCESS = 11,            /// 注册账号成功

	LOGIN_RETURN_CHARNAMEREPEAT = 12,            /// 角色名称重复
	LOGIN_RETURN_USERDATANOEXIST = 13,           /// 用户档案不存在
	LOGIN_RETURN_USERNAMEREPEAT = 14,            /// 用户名重复
	LOGIN_RETURN_TIMEOUT = 15,                   /// 连接超时
	LOGIN_RETURN_PAYFAILED  = 16,                 /// 计费失败
	LOGIN_RETURN_JPEG_PASSPORT = 17,             /// 图形验证码输入错误
	LOGIN_RETURN_LOCK = 18,              /// 帐号被锁定
	LOGIN_RETURN_WAITACTIVE = 19,                /// 帐号待激活
	LOGIN_RETURN_NEWUSER_OLDZONE = 20,           ///新账号不允许登入旧的游戏区 
	LOGIN_RETURN_UUID_ERROR = 21,                   ///登录UUID错误
	LOGIN_RETURN_USER_TOZONE = 22,           ///角色已登录战区,不允许创建角色
	LOGIN_RETURN_CHANGE_LOGIN = 23,              /// 跨区登陆验证失败
	LOGIN_RETURN_MATRIX_ERROR = 24,              /// 登录矩阵卡密码错误
	LOGIN_RETURN_MATRIX_NEED  = 25,               /// 提示玩家需要输入矩阵卡密码
	LOGIN_RETURN_MATRIX_LOCK = 26,				/// 提示玩家矩阵卡被锁（六个小时后解锁）
	LOGIN_RETURN_MATRIX_DOWN = 27,               /// 与矩阵卡验证服务器失去连接,无法进行矩阵卡验证
	LOGIN_RETURN_OLDUSER_NEWZONE = 28,				//旧帐号不允许登陆新区
	LOGIN_RETURN_IMG_LOCK  = 29,		//图形验证连续错误3次,角色被锁定
	LOGIN_RETURN_PASSPOD_PASSWORDERROR=30,
	LOGIN_RETURN_PASSPOD_DOWN=31,
	LOGIN_RETURN_BUSY	= 32,		//服务器繁忙
	LOGIN_RETURN_FORBID	= 33,		//帐号被封停
	LOGIN_RETURN_IMG_LOCK2  = 34,      //图形验证连续错误9次，角色被锁定
	LOGIN_RETURN_USERINLOGIN = 35,	//用户已经登录
	LOGIN_RETURN_CHARNAME_INVALID = 36, // 昵称含有敏感词汇
};

/// 登陆失败后返回的信息
struct ELEGANT_DECLARE_CMD(stServerReturnLoginFailedCmd_SC)
{
	stServerReturnLoginFailedCmd_SC()
	{
		byReturnCode = 0;
	}
	BYTE byReturnCode;			/**< 返回的子参数 */
};

enum
{
	IPHONE_UUID_LOGIN 	= 0,	//游客登陆

	//编号    简称    说明                                目前支持系统
	//1       ga      GA巨人账号系统                      android
	//2       gaapple GA巨人账号系统，苹果商店支付        ios

	//5       iapppay 爱贝云计费                          android

	//20      qihu    Qihu 360平台                        android
	//21      jiuyao  91无线平台                          android,ios
	//22      pp      PP助手                              ios

	//1001    adsage  艾德思奇，广告平台                  ios
	//1002    domob   多盟，广告平台                      ios
	//1003    immob   力美，广告平台                      ios
};



struct stPhoneInfo
{   
	char phone_uuid[100];  // 机器唯一码
	char pushid[100];      // 推送id
	char phone_model[100]; // 机型
	char resolution[100];  // 分辨率
	char opengl[100];      // opengl
	char cpu[100];
	char ram[100];
	char os[100];          // 操作系统
	stPhoneInfo()
	{   
		bzero(phone_uuid,sizeof(phone_uuid));
		bzero(pushid,sizeof(pushid));
		bzero(phone_model,sizeof(phone_model));
		bzero(resolution,sizeof(resolution));
		bzero(opengl,sizeof(opengl));
		bzero(cpu,sizeof(cpu));
		bzero(ram,sizeof(ram));
		bzero(os,sizeof(os));
	}

	bool hasPhone() const
	{
		std::string phone(phone_uuid);
		if(phone == "")
			return false;

		return true;
	}

	std::string toString() const
	{
		std::string result;
		result = result + "phone_uuid=" + phone_uuid + ",";
		result = result + "pushid=" + pushid + ",";
		result = result + "phone_model=" + phone_model + ",";
		result = result + "resolution=" + resolution + ",";
		result = result + "opengl=" + opengl + ",";
		result = result + "cpu=" + cpu + ",";
		result = result + "ram=" + ram + ",";
		result = result + "os=" + os;
		return result;
	}
};  

/// 客户端登录网关服务器发送UUID帐号和密码，IPHONE客户端
struct ELEGANT_DECLARE_CMD(stIphoneLoginUserCmd_CS) 
{
	stIphoneLoginUserCmd_CS()
	{
		accid = 0;
		acctype = IPHONE_UUID_LOGIN;
		bzero(password, sizeof(password));
		bzero(account, sizeof(account));
		bzero(szMAC,sizeof(szMAC));
		bzero(szFlat,sizeof(szFlat));
	}

	DWORD accid; //平台返回的ACCID，填写在这儿,用于登录验证,时间戳
	DWORD  acctype;	//登陆类型
	DWORD loginTempID; //平台生成的登录临时ID，用于验证登录有效性,SuperServer/FLClient.cpp中自增生成
	char account[MAX_ACCNAMESIZE];//帐号，通行证用户填通行证帐号，非通行证用户填，IPHONE UUID
	char password[MAX_PASSWORD];//非通行证用户填登录密码
	char szMAC[MAX_MAC_ADDR]; // MAC地址
	char szFlat[MAX_FLAT_LENGTH]; // 平台字符串	
	stPhoneInfo phone; // 机器信息
};


/// 创建主角，client->server
struct ELEGANT_DECLARE_CMD(stCreateNewRoleUserCmd_CS)
{
	stCreateNewRoleUserCmd_CS()
	{
		bySex = 0;
		bzero(strRoleName,sizeof(strRoleName));
		flatid = 0;
		heroid = 0;
	}
	
	char strRoleName[MAX_NAMESIZE];
	BYTE bySex;
	DWORD flatid; // 平台id 用来生成邀请码
	stPhoneInfo phone; // 机器信息
	DWORD heroid; // 选择英雄id
};


/// 登录成功，返回角色主要信息
struct ELEGANT_DECLARE_CMD(stReturnUserInfoUserCmd_SC)
{
	stReturnUserInfoUserCmd_SC()
	{
		bzero(&u, sizeof(u));
	}
	user_base_info u;
};

/// 绑定帐号
struct ELEGANT_DECLARE_CMD(stBindAccountUserCmd_CS)
{	
	stBindAccountUserCmd_CS()	
	{		
		bzero(account, sizeof(account));	
		bzero(passwd, sizeof(passwd));	
		type = 0;	
	}	
	char account[MAX_ACCNAMESIZE];
	char passwd[MAX_PASSWORD];	
	DWORD type;//0,巨人通行证
};

//请求获取公共聊天信息 c->s 进行获取 | s->c 表示收到消息
struct ELEGANT_DECLARE_CMD(stReqReceiveWorldChatCmd_CSC)
{
	stReqReceiveWorldChatCmd_CSC()
	{

	}
};

//请求关闭公共聊天信息
struct ELEGANT_DECLARE_CMD(stReqCloseWorldChatCmd_CSC)
{
	stReqCloseWorldChatCmd_CSC()
	{

	}
};

// 通知玩家被踢下线
struct ELEGANT_DECLARE_CMD(stNotifyUserKickOut_SC)
{
	stNotifyUserKickOut_SC()
	{
	}
};

//////////////////////////////////////////////////////////////
// 登陆指令定义结束
//////////////////////////////////////////////////////////////

}

#pragma pack()

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
