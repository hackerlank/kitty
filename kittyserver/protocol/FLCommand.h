#ifndef _FLCommand_h_
#define _FLCommand_h_

#include "zType.h"
#include "zNullCmd.h"
#include "EncDec.h"
#include "messageType.h"
#pragma pack(1)

struct t_NewLoginSession
{
	DWORD loginTempID;

	WORD wdGatewayID;
	char pstrIP[MAX_IP_LENGTH];
	WORD wdPort;

	GameZone_t gameZone;

	char passwd[MAX_ACCNAMESIZE];

	DES_cblock des_key;//des密钥

	char client_ip[MAX_IP_LENGTH];
	WORD wdNetType;             /**< 网关网络类型，0电信，1网通 */

	DWORD acctype;    //用户所属平台类型
	char account[MAX_ACCNAMESIZE]; //平台账号

	DWORD login_ret; // 登录结果 0正常 1把同账号踢下线
	
	t_NewLoginSession()
	{
		bzero(this,sizeof(*this));
	}

	t_NewLoginSession(const t_NewLoginSession& session)
	{   
		*this = session;
	}   
	t_NewLoginSession & operator= (const t_NewLoginSession &session)
	{   
		loginTempID = session.loginTempID;

		wdGatewayID = session.wdGatewayID;
		bcopy(session.pstrIP, pstrIP, sizeof(pstrIP));
		wdPort = session.wdPort;

		gameZone = session.gameZone;

		bcopy(session.passwd, passwd, sizeof(passwd));

		bcopy(session.des_key, des_key, sizeof(des_key));

		bcopy(session.client_ip, client_ip, sizeof(client_ip));
		wdNetType = session.wdNetType;

		acctype = session.acctype;
		bcopy(session.account, account, sizeof(account));

		login_ret = 0;

		return *this;
	} 
};

namespace CMD
{
	namespace FL
	{
#if 0
		const BYTE CMD_LOGIN = 1;
		const BYTE CMD_GYLIST = 2;
		const BYTE CMD_SESSION = 3;
#endif
        struct FLNullCmd : t_NullCmd
        {
            FLNullCmd() : t_NullCmd(FLCMD,PARA_NULL)
            {

            }
        };

		//////////////////////////////////////////////////////////////
		/// 登陆FL服务器指令
		//////////////////////////////////////////////////////////////
		const BYTE PARA_LOGIN = 1;
		struct t_LoginFL : FLNullCmd 
		{
			char strIP[MAX_IP_LENGTH];
			unsigned short port;

			t_LoginFL()
            {
                para = PARA_LOGIN;
                bzero(strIP,sizeof(strIP));
                port = 0;
            }

		};

		const BYTE PARA_LOGIN_OK = 2;
		struct t_LoginFL_OK :FLNullCmd 
		{
			GameZone_t gameZone;
			char name[MAX_NAMESIZE];
			BYTE netType;

			t_LoginFL_OK() 
			{
                para = PARA_LOGIN_OK;
				bzero(name, sizeof(name));
                netType = 0;
			};
		};
		//////////////////////////////////////////////////////////////
		/// 登陆FL服务器指令
		//////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////
		/// 定义网关信息相关指令
		//////////////////////////////////////////////////////////////
		const BYTE PARA_FL_GYLIST = 3;
		struct t_GYList_FL : FLNullCmd 
		{
			WORD wdServerID;			/**< 服务器编号 */
			BYTE pstrIP[MAX_IP_LENGTH];	/**< 服务器地址 */
			WORD wdPort;				/**< 服务器端口 */
			WORD wdNumOnline;			/**< 网关在线人数 */
			int  state;					/**< 服务器状态 */
			float zoneGameVersion;
			WORD wdNetType;				/**< 网关网络类型，0电信，1网通 */
			t_GYList_FL()
            {
                para = PARA_FL_GYLIST;
                wdServerID = 0;
                bzero(pstrIP,sizeof(pstrIP));
                wdPort = 0;
                wdNumOnline = 0;
                state = 0;
                zoneGameVersion = 0;
                wdNetType = 0;
            }
		};

		const BYTE PARA_FL_RQGYLIST = 4;
		struct t_RQGYList_FL : FLNullCmd 
		{
			t_RQGYList_FL() 
            {
                para = PARA_FL_RQGYLIST;
            }
		};
		//////////////////////////////////////////////////////////////
		/// 定义网关信息相关指令
		//////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////
		/// 定义与登陆session相关指令
		//////////////////////////////////////////////////////////////
		const BYTE PARA_SESSION_NEWSESSION = 5;
		struct t_NewSession_Session : FLNullCmd 
		{
			t_NewLoginSession session;
			t_NewSession_Session()
            {
                para = PARA_SESSION_NEWSESSION;
            }
		};

		const BYTE PARA_SESSION_IDINUSE = 6;
		struct t_idinuse_Session : FLNullCmd 
		{
			DWORD loginTempID;

			t_idinuse_Session()	
			{
                para = PARA_SESSION_IDINUSE;
				loginTempID = 0;
			};
		};

		// 大区注册人数
		const BYTE PARA_SESSION_REG_ROLE_COUNT = 7;
        struct t_RegRoleCount_Session : FLNullCmd 
        {
            t_RegRoleCount_Session()
            {
                para = PARA_SESSION_REG_ROLE_COUNT;
                role_count = 0;
            }
            DWORD role_count;
        };

		//////////////////////////////////////////////////////////////
		/// 定义与登陆session相关指令
		//////////////////////////////////////////////////////////////
	};
};


#pragma pack()

#endif

