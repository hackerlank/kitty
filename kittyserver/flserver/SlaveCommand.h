/**
 * \file
 * \version  $Id: SlaveCommand.h 2877 2005-09-12 12:16:19Z whj $
 * \author  ,
 * \date 2005年03月15日 12时11分04秒 CST
 * \brief 定义统一用户平台登陆服务器指令
 *
 * 
 */

#ifndef _SlaveCommand_h_
#define _SlaveCommand_h_

#include "zType.h"
#include "zNullCmd.h"
#include "FLCommand.h"

#pragma pack(1)

namespace CMD
{
	namespace Slave
	{
		const BYTE CMD_LOGIN = 1;
		const BYTE CMD_GYLIST = 2;
		const BYTE CMD_SESSION = 3;


		//////////////////////////////////////////////////////////////
		/// 登陆Master服务器指令
		//////////////////////////////////////////////////////////////
		const BYTE PARA_LOGIN = 1;
		struct t_LoginMaster : t_NullCmd
		{
			t_LoginMaster()
				: t_NullCmd(CMD_LOGIN, PARA_LOGIN) {};
		};

		const BYTE PARA_LOGIN_OK = 2;
		struct t_LoginMaster_OK : t_NullCmd
		{
			t_LoginMaster_OK()
				: t_NullCmd(CMD_LOGIN, PARA_LOGIN_OK) {};
		};
		//////////////////////////////////////////////////////////////
		/// 登陆Master服务器指令
		//////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////
		/// 定义网关信息相关指令
		//////////////////////////////////////////////////////////////
		const BYTE PARA_Master_GYLIST = 1;
		struct t_GYList_Master : t_NullCmd
		{
			GameZone_t gameZone;
			WORD wdServerID;			/**< 服务器编号 */
			BYTE pstrIP[MAX_IP_LENGTH];	/**< 服务器地址 */
			WORD wdPort;				/**< 服务器端口 */
			WORD wdNumOnline;			/**< 网关在线人数 */
			int  state;					/**< 服务器状态 */
			DWORD zoneGameVersion;
			WORD wdNetType;             /**< 网关网络类型，0电信，1网通 */
			t_GYList_Master()
				: t_NullCmd(CMD_GYLIST, PARA_Master_GYLIST) {};
		};

		const BYTE PARA_Master_RQGYLIST = 2;
		struct t_RQGYList_Master : t_NullCmd
		{
			t_RQGYList_Master()
				: t_NullCmd(CMD_GYLIST, PARA_Master_RQGYLIST) {};
		};

		const BYTE PARA_DISABLE_ALL = 3;
		struct t_Disable_All : t_NullCmd
		{
			GameZone_t gameZone;
			t_Disable_All()
				: t_NullCmd(CMD_GYLIST, PARA_DISABLE_ALL) {};
		};
		//////////////////////////////////////////////////////////////
		/// 定义网关信息相关指令
		//////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////
		/// 定义与登陆session相关指令
		//////////////////////////////////////////////////////////////
		const BYTE PARA_Master_NEWSESSION = 1;
		struct t_NewSession_Master : t_NullCmd
		{
			t_NewLoginSession session;
			DWORD loginTempID;

			t_NewSession_Master()
				: t_NullCmd(CMD_SESSION, PARA_Master_NEWSESSION) {};
		};

		const BYTE PARA_Master_IDINUSE = 2;
		struct t_idinuse_Master : t_NullCmd
		{
			DWORD accid;
			DWORD loginTempID;
			char name[48];

			t_idinuse_Master()
				: t_NullCmd(CMD_SESSION, PARA_Master_IDINUSE) { bzero(name, sizeof(name)); };
		};
		const BYTE PARA_Master_GATEWAY_NOAVL = 3;
		struct t_Gateway_NoAvl_Master : t_NullCmd
		{
			DWORD accid;
			DWORD loginTempID;
			char name[48];

			t_Gateway_NoAvl_Master()
				: t_NullCmd(CMD_SESSION, PARA_Master_GATEWAY_NOAVL) { bzero(name, sizeof(name)); };
		};
		//////////////////////////////////////////////////////////////
		/// 定义与登陆session相关指令
		//////////////////////////////////////////////////////////////
	};
};

#pragma pack()

#endif

