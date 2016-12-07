#ifndef _SuperCommand_h_
#define _SuperCommand_h_

#include "zType.h"
#include "zNullCmd.h"
#include "FLCommand.h"
#include <string.h>
#include "messageType.h"

#pragma pack(1)

namespace CMD
{

	namespace SUPER
	{
#if 0
		const BYTE CMD_STARTUP	= 1;
		const BYTE CMD_GATEWAY	= 4;
		const BYTE CMD_SESSION	= 5;
		const BYTE CMD_RECORD	= 6;
		const BYTE CMD_SCENE	= 7;
#endif   
        struct SuperServerNull : t_NullCmd
        {
            SuperServerNull()
            {
                cmd = SUPERCMD;
            }
        };

		//////////////////////////////////////////////////////////////
		// 定义启动相关指令
		//////////////////////////////////////////////////////////////
		const BYTE PARA_STARTUP_REQUEST = 1;
		struct t_Startup_Request : SuperServerNull 
		{
			WORD wdServerType;
			char pstrIP[MAX_IP_LENGTH];
			t_Startup_Request()
			{
                para = PARA_STARTUP_REQUEST;
                wdServerType = 0;
				bzero(pstrIP, sizeof(pstrIP));
			};
		};

		const BYTE PARA_STARTUP_RESPONSE = 2;
		struct t_Startup_Response : SuperServerNull 
		{
			WORD wdServerID;
			WORD wdPort;
			char pstrExtIP[MAX_IP_LENGTH];
			WORD wdExtPort;
			WORD wdNetType; // 服务器，所以在网络，0电信，1网通 
            char pstrTable[MAX_TABLE_LIST];
			t_Startup_Response()
			{
                para = PARA_STARTUP_RESPONSE;
				bzero(pstrExtIP, sizeof(pstrExtIP));
                bzero(pstrTable, sizeof(pstrTable));
				wdServerID = 0;
				wdPort = 0;
				wdExtPort = 0;
				wdNetType = 0;
			};
		};

		struct ServerEntry
		{
			WORD wdServerID;
			WORD wdServerType;
			char pstrName[MAX_NAMESIZE];
			char pstrIP[MAX_IP_LENGTH];
			WORD wdPort;
			char pstrExtIP[MAX_IP_LENGTH];
			WORD wdExtPort;
			WORD state;
            char pstrTable[MAX_TABLE_LIST];
			ServerEntry()
			{
				wdServerID = 0;
				wdServerType = 0;
				bzero(pstrName, sizeof(pstrName));
				bzero(pstrIP, sizeof(pstrIP));
				wdPort = 0;
				bzero(pstrExtIP, sizeof(pstrExtIP));
				wdExtPort = 0;
				state = 0;
                bzero(pstrTable, sizeof(pstrTable));

			}
			ServerEntry(const ServerEntry& se)
			{
				wdServerID = se.wdServerID;
				wdServerType = se.wdServerType;
				bcopy(se.pstrName, pstrName, sizeof(pstrName));
				bcopy(se.pstrIP, pstrIP, sizeof(pstrIP));
				wdPort = se.wdPort;
				bcopy(se.pstrTable, pstrTable, sizeof(pstrTable));
				wdExtPort = se.wdExtPort;
				state = se.state;
                bcopy(se.pstrExtIP, pstrExtIP, sizeof(pstrExtIP));
			}
			ServerEntry & operator= (const ServerEntry &se)
			{
				wdServerID = se.wdServerID;
				wdServerType = se.wdServerType;
				bcopy(se.pstrName, pstrName, sizeof(pstrName));
				bcopy(se.pstrIP, pstrIP, sizeof(pstrIP));
				wdPort = se.wdPort;
				bcopy(se.pstrExtIP, pstrExtIP, sizeof(pstrExtIP));
				wdExtPort = se.wdExtPort;
				state = se.state;
                bcopy(se.pstrExtIP, pstrExtIP, sizeof(pstrExtIP));
				return *this;
			}
		};
		const BYTE PARA_STARTUP_SERVERENTRY_NOTIFYME = 3;
		struct t_Startup_ServerEntry_NotifyMe : SuperServerNull 
		{
			WORD size;
			ServerEntry entry[0];
			t_Startup_ServerEntry_NotifyMe()
            {
                para = PARA_STARTUP_SERVERENTRY_NOTIFYME;
                size = 0;
            }
		};
		const BYTE PARA_STARTUP_SERVERENTRY_NOTIFYOTHER = 4;
		struct t_Startup_ServerEntry_NotifyOther : SuperServerNull 
		{
			WORD srcID;
			ServerEntry entry;
			t_Startup_ServerEntry_NotifyOther()
            {
                para = PARA_STARTUP_SERVERENTRY_NOTIFYOTHER;
                srcID = 0;
            }
		};

		const BYTE PARA_STARTUP_OK = 5;
		struct t_Startup_OK : SuperServerNull 
		{
			WORD wdServerID;
			t_Startup_OK()
            {
                para = PARA_STARTUP_OK;
                wdServerID = 0;
            }
		};

		const BYTE PARA_GAMETIME = 6;
		struct t_GameTime : SuperServerNull 
		{
			QWORD qwGameTime;
			QWORD qwStartTime;
			t_GameTime()
            {
                para = PARA_GAMETIME;
                qwGameTime = 0;
                qwStartTime = 0;
            }
		};
       /*
		const BYTE PARA_RESTART_SERVERENTRY_NOTIFYOTHER = 7;
		struct t_restart_ServerEntry_NotifyOther : SuperServerNull 
		{
			WORD srcID;
			WORD dstID;
			t_restart_ServerEntry_NotifyOther()
            {
                para = PARA_RESTART_SERVERENTRY_NOTIFYOTHER;
                srcID = 0;
                dstID = 0;
            }
		};
        */
		//////////////////////////////////////////////////////////////
		// 定义启动相关指令
		//////////////////////////////////////////////////////////////


		// 设置区ID
		const BYTE PARA_ZONE_ID = 8;
		struct t_ZoneID : SuperServerNull 
		{
			GameZone_t zone;
            char name[MAX_NAMESIZE];
			t_ZoneID()
			{
                para = PARA_ZONE_ID;
                bzero(name,sizeof(name));
			}
		};
		
		// Super -> Gateway
		const BYTE PARA_GATE_RECONNECT_SCENE = 9;
		struct t_GateReconnectScene : SuperServerNull
		{
			ServerEntry entry;
			t_GateReconnectScene()
            {
                para = PARA_GATE_RECONNECT_SCENE;
            }

		};

		// Super->Gateway 通知网关结束掉某个ip和端口的连接
		const BYTE PARA_GATE_TERMINATE_CONNECT = 10;
		struct t_GateTerminateConnect : SuperServerNull
		{
			char pstrIP[MAX_IP_LENGTH];
			WORD port;
			t_GateTerminateConnect()
			{
                para = PARA_GATE_TERMINATE_CONNECT;
				bzero(pstrIP,sizeof(pstrIP));
				port = 0;
			}
		};

		const BYTE PARA_SUPER2GATE_KICK_OUT_USER  = 11;
        struct t_Super2GateKickOutUser : SuperServerNull 
        {
            t_Super2GateKickOutUser() 
            {
                para = PARA_SUPER2GATE_KICK_OUT_USER;
                acctype = 0;
                bzero(account,sizeof(account));
            }
            
            DWORD acctype;    //用户所属平台类型
            char account[MAX_ACCNAMESIZE]; //平台账号
        };
		
		//////////////////////////////////////////////////////////////
		// 定义服务器管理器与会话服务器交互的指令
		//////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////
		// 定义服务器管理器与场景服务器交互的指令
		//////////////////////////////////////////////////////////////

		// 来自场景转发给AllZone的消息
		const BYTE PARA_FORWARD_ALLZONE_FROM_SCENE = 12;
		struct t_ForwardAllZoneFromScene : SuperServerNull 
        {
            t_ForwardAllZoneFromScene()
            {
                para = PARA_FORWARD_ALLZONE_FROM_SCENE;
                size = 0;
            }
			DWORD size;
            char data[0];
            DWORD getSize() const { return sizeof(*this) + size; }
        };
		
		const BYTE PARA_FORWARD_SCENE_FROM_ALLZONE = 13;
        struct t_ForwardSceneFromAllZone : SuperServerNull 
        {
            t_ForwardSceneFromAllZone()
            {
                para = PARA_FORWARD_SCENE_FROM_ALLZONE;
                datasize = 0;
                charid = 0;
            }
            QWORD charid; 
            DWORD datasize;
            char data[0];
            DWORD getSize() const { return sizeof(*this) + datasize; }
        };

        const BYTE PARA_STARTUP = 14;
        struct t_StartUp : SuperServerNull
        {
            t_StartUp()
            {
                para = PARA_STARTUP;
            }
        };
        
        const BYTE PARA_CHANGE_GAMETIME = 15;
		struct t_ChangeGameTime : SuperServerNull 
		{
            char time[MAX_NAMESIZE];
			t_ChangeGameTime()
            {
                para = PARA_CHANGE_GAMETIME;
                bzero(time,sizeof(time));
            }
		};
        //其他同步消息:
        const BYTE PARA_STARTUP_SERVERENTRY_NOTIFYME2 = 16;
		struct t_Startup_ServerEntry_NotifyMe2 : SuperServerNull 
		{
			WORD size;
			ServerEntry entry[0];
			t_Startup_ServerEntry_NotifyMe2()
            {
                para = PARA_STARTUP_SERVERENTRY_NOTIFYME2;
                size = 0;
            }
		};

        const BYTE PARA_CLOSE_SERVERENTRY_NOTIFYME2 = 17;
		struct t_CLOSE_ServerEntry_NotifyMe2 : SuperServerNull 
		{
            DWORD ServerID;
			t_CLOSE_ServerEntry_NotifyMe2()
            {
                para = PARA_CLOSE_SERVERENTRY_NOTIFYME2;
                ServerID = 0;
            }
		};
        
       //重新加载配置
        const BYTE PARA_RELOAD_CONFIG = 18;
        struct t_ReloadConfig : SuperServerNull
        {
            t_ReloadConfig()
            {
                para = PARA_RELOAD_CONFIG;
            }
        };
        //其他同步消息:
        const BYTE PARA_STARTUP_SERVERENTRY_MeStart = 19;
		struct t_Startup_ServerEntry_MeStart : SuperServerNull 
		{
			ServerEntry entry;
			t_Startup_ServerEntry_MeStart()
            {
                para = PARA_STARTUP_SERVERENTRY_MeStart;
            }
		};

        //同步心跳包:
        const BYTE PARA_Change_HeartTime = 20;
		struct t_ChangeHeartTime : SuperServerNull 
		{
            DWORD heartTime;
			t_ChangeHeartTime()
            {
                para = PARA_Change_HeartTime;
                heartTime = 0;
            }
		};



        
	};

};

#pragma pack()

#endif

