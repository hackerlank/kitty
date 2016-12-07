/**
 * \file
 * \version  $Id: UserCommand.h 13 2013-03-20 02:35:18Z  $
 * \author  李也白,yebai.li@newtouch.com.cn
 * \date 2005年07月14日 19时37分52秒 CST
 * \brief 用户信息服务类命令
 *
 * 
 */

#ifndef __UserCommand_H__
#define __UserCommand_H__

#include "zType.h"
#include "zNullCmd.h"

#pragma pack(1)

namespace CMD
{
#if 0
	namespace UserServer
	{
		const int SEQ_MAX_LENGTH = 32;		//交易序列号长度
		const int ID_MAX_LENGTH = 64;		//帐号长度
		const int REMARK_LENGTH = 200;		//备注长度
		const int CARDID_LENGTH = 32;
		const int PCARD_NO_LENGTH = 20;		//道具卡号长度
		const int CARD_PWD_LENGTH = 8;      //卡密码长度

		const BYTE CMD_USER_LOGON = 1; 		//登陆服务器命令
		const BYTE CMD_USER_CONSUME = 2; 	//玩家扣费命令
		const BYTE CMD_USER_FILLIN = 3; 	//玩家充值命令
		const BYTE CMD_USER_QBALANCE = 4; 	//帐号余额查询
		const BYTE CMD_USER_MCARD = 5; 		//玩家冲值月卡
		const BYTE CMD_USER_PCARD = 6; 		//玩家道具卡
		const BYTE CMD_USER_SCARD = 7;		//专区卡
		const BYTE CMD_USER_USE_SCARD = 8;  //使用专区卡
		const BYTE CMD_USER_USE_CARD = 9;   //使用卡
		const BYTE CMD_USER_PPCONSUME = 11;     //玩家赠点扣费命令
		struct t_logon : public CMD::t_NullCmd
		{
			char strIP[MAX_IP_LENGTH];
			unsigned short port;
			t_logon() : t_NullCmd(CMD_USER_LOGON)
			{
				bzero(strIP,sizeof(strIP));
				port = 0;
			}
		};
		//服务器同步命令
		struct t_logon_OK : t_NullCmd		
		{
			t_logon_OK()
				: t_NullCmd(CMD_USER_LOGON)
			{
				bzero(name, sizeof(name));
			};
			GameZone_t 	gameZone;						//游戏区
			char 		name[MAX_NAMESIZE];				//区名字
			BYTE 		netType;						//网络类型
		};

		struct t_cmd_consume : public CMD::t_NullCmd
		{
			t_cmd_consume() 
				: t_NullCmd(CMD_USER_CONSUME) 
				{ 
					bzero(tid, sizeof(tid));
					bzero(remark, sizeof(remark));
					bzero(ip, sizeof(ip));
				}
			unsigned int 	uid;						//UID
			int				point;						//扣费点数	
			int 			source;						//来源
			char			tid[SEQ_MAX_LENGTH +1];		//交易序列号
			char			remark[REMARK_LENGTH +1];	//备注
			char 			ip[MAX_IP_LENGTH +1];       //客户请求ip
			int				at;							//交易类型
		};

		struct t_cmd_fillin : public CMD::t_NullCmd
		{
			t_cmd_fillin() 
				: t_NullCmd(CMD_USER_FILLIN) 
				{ 
					bzero(account, ID_MAX_LENGTH +1);
					bzero(tid, SEQ_MAX_LENGTH +1);
					bzero(cardid, CARDID_LENGTH +1);
					bzero(remark, REMARK_LENGTH +1);
				}
			unsigned int uid;							//uid
			int				point;						//充值点数	
			int 			source;						//来源
			char			account[ID_MAX_LENGTH +1];	//玩家帐号
			char			tid[SEQ_MAX_LENGTH +1];		//交易序列号
			char			cardid[CARDID_LENGTH +1];	//充值卡号
			char			remark[REMARK_LENGTH +1];	//备注
			char 			ip[MAX_IP_LENGTH +1];     	//客户请求ip
			int				at;							//交易类型
		};

		struct t_cmd_qbalance : public CMD::t_NullCmd
		{
			t_cmd_qbalance() 
				: t_NullCmd(CMD_USER_QBALANCE) { }
			unsigned int	uid;						//UID
			char			account[ID_MAX_LENGTH +1];	//玩家帐号
			char			tid[SEQ_MAX_LENGTH +1];		//交易序列号
			int				at;							//交易类型
		};

		struct t_cmd_mcard : public CMD::t_NullCmd
		{
			t_cmd_mcard() 
				: t_NullCmd(CMD_USER_MCARD) 
				{ 
					bzero(account, sizeof(account));
					bzero(tid, sizeof(tid));
					bzero(remark, sizeof(remark));
					bzero(ip, sizeof(ip));
				}
			unsigned int 	uid;						//UID
			int				point;						//扣费点数	
			int 			source;						//来源
			char			account[ID_MAX_LENGTH +1];	//玩家帐号
			char			tid[SEQ_MAX_LENGTH +1];		//交易序列号
			char			remark[REMARK_LENGTH +1];	//备注
			char 			ip[MAX_IP_LENGTH +1];     	//客户请求ip
			int				at;							//交易类型
		};

		struct t_cmd_pcard : public CMD::t_NullCmd
		{
			t_cmd_pcard() 
				: t_NullCmd(CMD_USER_PCARD) 
				{ 
					bzero(tid, sizeof(tid));
					bzero(pcardid, sizeof(pcardid));
					bzero(ip, sizeof(ip));
				}
			unsigned int 	uid;						//UID
			int 			source;						//来源
			char			tid[SEQ_MAX_LENGTH +1];		//交易序列号
			char			pcardid[CARDID_LENGTH +1];  //充值卡号
			char 			ip[MAX_IP_LENGTH +1];       //客户请求ip
			int				at;							//交易类型
		};
		
		struct t_cmd_ppconsume : public CMD::t_NullCmd
		{
			t_cmd_ppconsume()
				: t_NullCmd(CMD_USER_PPCONSUME)
				{
					bzero(tid, sizeof(tid));
					bzero(remark, sizeof(remark));
					bzero(ip, sizeof(ip));
				}
			unsigned int    uid;                        //UID
			unsigned int    receiveuid;                 //赠点接受UID
			int             point;                      //扣费点数  
			int             source;                     //来源
			char            tid[SEQ_MAX_LENGTH +1];     //交易序列号
			char            remark[REMARK_LENGTH +1];   //备注
			char            ip[MAX_IP_LENGTH +1];       //客户请求ip
			int             at;                         //交易类型
		};

		//billclient与userserver通信操作码返回值
		enum
		{
			RET_OK = 0,									//成功
			RET_FAIL = -1,								//失败
			RET_ID_NOT_EXIST = -2,						//用户不存在
			RET_BALANCE_NOT_ENOUGH = -3,				//余额不足
			RET_PCARD_NOT_EXIST = -4,					//道具卡不存在
			RET_NOTUSE_GAMEZONE = -5,					//本道具卡不能在该区使用
			RET_PCARD_NOT_REUSE = -6,					//道具卡不能重复使用
			RET_SCARD_ERR = -7,							//专区卡卡号错误
			RET_SCARD_PASSWD_ERR = -8,					//专区卡密码错误
			RET_DB_ERROR = -9,							//数据库错误
			RET_RECEIVEID_NOT_EXIST = -10,               //赠点接受用户不存在
		};

		//billclient请求类型
		enum
		{
			AT_FILLIN = 0,								//充值
			AT_CONSUME = 1,								//扣费
			AT_SCARD = 2,								//专区卡
			AT_QBALANCE = 3,							//查询
			AT_MCARD = 4,								//月卡冲值
			AT_PCARD = 5,								//道具卡
			AT_FUND = 6,								//基金
			AT_BAOXIAN = 7,								//保险
			AT_SECOND_CONSUME = 8,                      //点兑换时间秒卡
			AT_MONTH_CONSUME = 9,                       //点兑换时间月卡
			AT_ERROR = -1,								//错误类型
			AT_PPCONSUME = 20,                          //赠点
		};

		//回复billclient子类型
		enum
		{
			SUBAT_INVALID = -1,							//billclient请求子类型
			SUBAT_GOLD = 4								//专区卡充值返回子类型//道具卡的子类型不能于该值重复
		};

		enum
		{
			KEEP = 0,									//保留
			ESALES_FILLIN = 1,							//电子商城充卡
			ESALES_CONSUME = -1							//电子商城消费
		};

		//服务器端的返回命令
		const BYTE CMD_USER_RET = 5;		
		struct t_cmd_ret : public CMD::t_NullCmd
		{
			t_cmd_ret() 
				: t_NullCmd(CMD_USER_RET) 
				{
					bzero(tid, sizeof(tid));
					balance = 0;
					bonus = 0;
					hadfilled = 0;
					ret = RET_FAIL;
					subat = SUBAT_INVALID;
					at = AT_ERROR;
					point = 0;
					uid = 0;
				}
			char			tid[SEQ_MAX_LENGTH + 1];	//交易序列号
			int 			balance;					//余额
			int 			bonus;						//积分
			int 			hadfilled;					//曾经充值的标志,1=曾经充值,0=没有
			int 			ret;						//命令返回代码
			int				subat;						//子类型
			int				at;							//交易类型
			int				point;						//点数
			unsigned int	uid;						//UID
		};

		//向CardSverver发送使用专区卡
		const BYTE PARA_USE_SCARD = 1;  	
		struct t_cmd_scard_use : public CMD::t_NullCmd
		{
			t_cmd_scard_use()
				: t_NullCmd(CMD_USER_USE_SCARD, PARA_USE_SCARD)
				{
					uid = 0;
					source = 0;
					bzero(tid, sizeof(tid));
					bzero(pcardid, sizeof(pcardid));
					bzero(passwd, sizeof(passwd));
					bzero(ip, sizeof(ip));
					bzero(account, sizeof(account));
					conTempID = 0;
				}
			unsigned int	uid;							//UID
			int				source;							//来源
			char			tid[SEQ_MAX_LENGTH +1];			//交易序列号
			char			pcardid[CARDID_LENGTH +1];		//充值卡号
			char			passwd[CARD_PWD_LENGTH +1];		//卡密码
			char			ip[MAX_IP_LENGTH +1];			//客户请求ip
			char			account[ID_MAX_LENGTH +1];		//玩家帐号
			unsigned int	conTempID;						//连接ID，CardServer原样返回
			int				at;								//交易类型
		};
		//CardSverver返回专区卡使用的结果
		const BYTE PARA_USE_SCARD_RET = 2;      
		struct t_cmd_scard_use_ret : public CMD::t_NullCmd
		{
			t_cmd_scard_use_ret()
				: t_NullCmd(CMD_USER_USE_SCARD, PARA_USE_SCARD_RET)
				{
					uid = 0;
					source = 0;
					bzero(tid, sizeof(tid));
					bzero(pcardid, sizeof(pcardid));
					bzero(passwd, sizeof(passwd));
					bzero(ip, sizeof(ip));
					bzero(account, sizeof(account));
					conTempID = 0;
					point = 0;
					ret = RET_FAIL;
					at = AT_ERROR;
				}
			unsigned int    uid;                            //UID
			int             source;                         //来源
			char            tid[SEQ_MAX_LENGTH +1];         //交易序列号
			char            pcardid[CARDID_LENGTH +1];		//充值卡号
			char            passwd[CARD_PWD_LENGTH +1];     //卡密码
			char            ip[MAX_IP_LENGTH +1];     		//客户请求ip
			char            account[ID_MAX_LENGTH +1];      //玩家帐号
			unsigned int    conTempID;                      //连接ID
			int             point;                          //点数
			int             ret;                            //命令返回代码
			int				at;								//交易类型
		};
		//向CardSverver发送使用卡指令
		const BYTE PARA_USE_CARD = 3;   
		struct t_cmd_card_use : public CMD::t_NullCmd
		{
			t_cmd_card_use()
				: t_NullCmd(CMD_USER_USE_CARD, PARA_USE_CARD)
				{
					bzero(account, sizeof(account));
					bzero(tid, sizeof(tid));
					bzero(cardid, sizeof(cardid));
					bzero(passwd, sizeof(passwd));
					bzero(ip, sizeof(ip));
					uid = 0;
					conTempID = 0;
					operateUid = 0;
					at = AT_ERROR;
				}
			int				source;							//来源
			char			account[ID_MAX_LENGTH +1];		//玩家帐号
			char			tid[SEQ_MAX_LENGTH +1];			//交易序列号
			char			cardid[CARDID_LENGTH +1];		//充值卡号
			char			passwd[CARD_PWD_LENGTH +1];		//卡密码
			char			ip[MAX_IP_LENGTH +1];			//客户请求ip
			unsigned int	uid;							//UID
			int				at;								//交易类型
			unsigned int    conTempID; 	
			unsigned int	operateUid;							//UID
		};
		//CardSverver返回使用卡指令的结果
		const BYTE PARA_USE_CARD_RET = 4;     
		struct t_cmd_card_use_ret : public CMD::t_NullCmd
		{
			t_cmd_card_use_ret()
				: t_NullCmd(CMD_USER_USE_CARD, PARA_USE_CARD_RET)
				{
					bzero(account, sizeof(account));
					bzero(tid, sizeof(tid));
					bzero(cardid, sizeof(cardid));
					bzero(passwd, sizeof(passwd));
					bzero(ip, sizeof(ip));
					point = 0;
					ret = RET_FAIL;
					uid = 0;
					operateUid = 0;
					conTempID = 0;
					at = AT_ERROR;
				}
			int				source;							//来源
			char			account[ID_MAX_LENGTH +1];		//玩家帐号
			char			tid[SEQ_MAX_LENGTH +1];			//交易序列号
			char			cardid[CARDID_LENGTH +1];		//充值卡号
			char			passwd[CARD_PWD_LENGTH +1];		//卡密码
			char			ip[MAX_IP_LENGTH +1];			//客户请求ip
			int				point;							//点数
			int				ret;							//命令返回代码
			unsigned int	uid;							//UID
			int				at;								//交易类型
			unsigned int    conTempID; 
			unsigned int	operateUid;							//操作者的UID
		};


		/*定义登陆CardSverver指令*/
		const BYTE CMD_CARD_LOGON = 1;
		const BYTE PARA_CARD_LOGON = 1;
		struct t_LogonCard : t_NullCmd
		{
			t_LogonCard()
				: t_NullCmd(CMD_CARD_LOGON, PARA_CARD_LOGON) {};
		};
	};
#endif
};

#pragma pack()

#endif

