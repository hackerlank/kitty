/**
 * \file
 * \version  $Id: LogonCommand.h 1062 2006-07-27 02:47:32Z song $
 * \author  王海军,wanghaijun@ztgame.com 
 * \date 2006年05月21日 20时04分46秒 CST
 * \brief 定义与登陆有关的指令,客户端,服务器,平台3方共享此文件
 *
 * 
 */

#ifndef _LOGONCOMMAND_H_
#define _LOGONCOMMAND_H_
#include "zNullCmd.h"
#include "zType.h"

#pragma pack(1)

namespace CMD
{

/// 登陆指令
const BYTE LOGON_USERCMD		= 104;

//////////////////////////////////////////////////////////////
// 登陆指令定义开始
//////////////////////////////////////////////////////////////

struct stLogonUserCmd : public stNullUserCmd
{
	stLogonUserCmd()
	{
		byCmd = LOGON_USERCMD;
	}
};

/// 客户端验证版本
const BYTE USER_VERIFY_VER_PARA = 120;
const DWORD GAME_VERSION = 1999;
struct stUserVerifyVerCmd  : public stLogonUserCmd
{
	stUserVerifyVerCmd()
	{
		byParam = USER_VERIFY_VER_PARA;
		version = GAME_VERSION;
	}
	DWORD reserve;	//保留字段
	DWORD version;
};

enum{
	LOGIN_RETURN_UNKNOWN=0, 					/// 未知错误                                          
	LOGIN_RETURN_VERSIONERROR=1,				/// 版本错误                                        
	LOGIN_RETURN_UUID=2,						/// UUID登陆方式没有实现                                
	LOGIN_RETURN_DB=3,						    /// 数据库出错                                        
	LOGIN_RETURN_PASSWORDERROR=4,				/// 帐号密码错误                                    
	LOGIN_RETURN_CHANGEPASSWORD=5,			    /// 修改密码成功                                
	LOGIN_RETURN_IDINUSE=6,					    /// ID正在被使用中                                  
	LOGIN_RETURN_IDINCLOSE=7,					/// ID被封                                            
	LOGIN_RETURN_GATEWAYNOTAVAILABLE=8,		    /// 网关服务器未开                            
	LOGIN_RETURN_USERMAX=9,					    /// 用户满                                          
	LOGIN_RETURN_ACCOUNTEXIST=10,				/// 账号已经存在                                    
	LOGON_RETURN_ACCOUNTSUCCESS=11,			    /// 注册账号成功                                
	LOGIN_RETURN_CHARNAMEREPEAT=12,			    /// 角色名称重复                                
	LOGIN_RETURN_USERDATANOEXIST=13,			/// 用户档案不存在                                
	LOGIN_RETURN_USERNAMEREPEAT=14,			    /// 用户名重复                                  
	LOGIN_RETURN_TIMEOUT=15,					/// 连接超时                                          
	LOGIN_RETURN_PAYFAILED=16,					/// 计费失败                                        
	LOGIN_RETURN_JPEG_PASSPORT=17,				/// 图形验证码输入错误                            
	LOGIN_RETURN_LOCK=18,				        /// 帐号被锁定                                      
	LOGIN_RETURN_WAITACTIVE=19,				    /// 帐号待激活                                    

	LOGIN_RETURN_NEWUSER_OLDZONE=20,            /// 新账号不允许登入旧的游戏区              
	LOGIN_RETURN_UUID_ERROR=21,                 /// 登录UUID错误                            
	LOGIN_RETURN_USER_TOZONE=22,                /// 角色已登录战区,不允许创建角色           
	LOGIN_RETURN_CHANGE_LOGIN=23,               /// 跨区登陆验证失败                        
	LOGIN_RETURN_MATRIX_ERROR=24,               /// 登录矩阵卡密码错误                      
	LOGIN_RETURN_MATRIX_NEED=25,				/// 提示玩家需要输入矩阵卡密码                      
	LOGIN_RETURN_MATRIX_LOCK=26,				/// 提示玩家矩阵卡被锁（六个小时后解锁）            
	LOGIN_RETURN_MATRIX_DOWN=27,				/// 与矩阵卡验证服务器失去连接,无法进行矩阵卡验证   
	LOGIN_RETURN_OLDUSER_NEWZONE=28,          /// 旧帐号不允许登陆新区       
	LOGIN_RETURN_IMG_LOCK=29,		           //图形验证连续错误,角色被锁定
	LOGIN_RETURN_PASSPOD_PASSWORDERROR=30,  ///密保密码错误
	LOGIN_RETURN_PASSPOD_DOWN=31,                   //与密保服务器失去连接
	LOGIN_RETURN_BUSY=32,						//游戏区忙，游戏用错误码
	LOGIN_RETURN_FORBID=33,					//角色被封，游戏用错误码
	LOGIN_RETURN_IMG_LOCK2=34,				//图形码连续输入错误，游戏用错误码
	LOGIN_RETURN_MAINTAIN=35	,				//游戏区正常维护中，原来的错误码LOGIN_RETURN_GATEWAYNOTAVAILABLE表示非正常维护
	LOGIN_RETURN_TDCODE_GEN_ERROR=36,  		///获取二维码失败
	LOGIN_RETURN_TDCODE_DOWN=37,                   //二维码服务不可用,请输入帐号密码登陆
	LOGIN_RETURN_TOKEN_ERROR=38,                   //token验证失败
	LOGIN_RETURN_TOKEN_TOO_QUICK=39,         // TOKEN验证太快，意思说上次验证还没结束，就开始新的验证
	LOGIN_RETURN_TOKEN_TIMEOUT=40,           // TOKEN验证超时
	LOGIN_RETURN_SHOW_MSG = 41,              // 显示后面的错误消息
};

/// 登陆失败后返回的信息
const BYTE SERVER_RETURN_LOGIN_FAILED = 3;
struct stServerReturnLoginFailedCmd : public stLogonUserCmd
{
	stServerReturnLoginFailedCmd()
	{
		byParam = SERVER_RETURN_LOGIN_FAILED;
		size = 0;
	}
	BYTE byReturnCode;			/**< 返回的子参数 */
	WORD size;
	BYTE data[0];				// 对应的字符串错误消息
	DWORD getSize(){return sizeof(*this) + size;}
} ;

/// 登陆成功，返回网关服务器地址端口以及密钥等信息
const BYTE SERVER_RETURN_LOGIN_OK = 4;
struct stServerReturnLoginSuccessCmd : public stLogonUserCmd 
{
	stServerReturnLoginSuccessCmd()
	{
		byParam = SERVER_RETURN_LOGIN_OK;

		bzero(account,MAX_ACCNAMESIZE);
		login_ret = 0;
	}

	DWORD dwUserID;
	DWORD loginTempID;
	char pstrIP[MAX_IP_LENGTH];
	WORD wdPort;
	char account[MAX_ACCNAMESIZE];

	union{
		struct{
			BYTE randnum[58];
			BYTE keyOffset;	// 密匙在 key 中的偏移
		};
		BYTE key[256];	// 保存密匙，整个数组用随机数填充
	};
	DWORD state;		//帐号状态
	DWORD login_ret; // 0正常 1把同账号踢下线

	DWORD getSize(){return sizeof(*this);}
};

/// IPHONE客户端用帐号申请登陆
const BYTE IPHONE_USER_REQUEST_LOGIN_PARA = 1;
struct stIphoneUserRequestLoginCmd : public stLogonUserCmd
{
	stIphoneUserRequestLoginCmd()
	{
		byParam = IPHONE_USER_REQUEST_LOGIN_PARA;

		userType = 0;
		bzero(account,sizeof(account));
		game = 0;
		zone = 0;
		wdNetType = 0;
		uid = 0;
		bzero(token,sizeof(token));
	}

	WORD userType; 
	char account[MAX_ACCNAMESIZE];              //玩家数字账号
	 char pstrPassword[33];	            //<用户密码 
	WORD game;			            //<游戏类型编号，目前一律添0 
	WORD zone;				    //<游戏区编号 
	WORD wdNetType;            		    //<网关网络类型，0电信，1网通 
	QWORD uid;                                  //用户uid
	char token[MAX_NAMESIZE];                   //token字符串长度
	char phone_uuid[MAX_ACCNAMESIZE]; // 机器唯一uuid;
};


};

#pragma pack()


#endif

