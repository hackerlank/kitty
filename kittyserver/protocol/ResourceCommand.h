#ifndef RESOURCE_COMMAND_H
#define RESOURCE_COMMAND_H 

#include "zType.h"
#include "zNullCmd.h"
#include "EncDec.h"
#include "messageType.h"
#pragma pack(1)

namespace CMD
{
	namespace RES
	{
        struct ResNullCmd : t_NullCmd
        {
            ResNullCmd() : t_NullCmd(RESCMD,PARA_NULL)
            {

            }
        };

		//////////////////////////////////////////////////////////////
		/// 登陆GMTool服务器指令
		//////////////////////////////////////////////////////////////
		const BYTE PARA_LOGIN = 1;
		struct t_LoginRes : ResNullCmd 
		{
			char strIP[MAX_IP_LENGTH];
			unsigned short port;
			t_LoginRes()
            {
                para = PARA_LOGIN;
                bzero(strIP,sizeof(strIP));
                port = 0;
            }

		};

		const BYTE PARA_LOGIN_OK = 2;
		struct t_LoginRes_OK : ResNullCmd 
		{
			GameZone_t gameZone;
			char name[MAX_NAMESIZE];
			t_LoginRes_OK() 
			{
                para = PARA_LOGIN_OK;
				bzero(name, sizeof(name));
			};
		};

        const BYTE PARA_ADD_RES = 3;
		struct t_AddRes : ResNullCmd 
		{
            QWORD charID;
            DWORD resType;
            DWORD resID;
            DWORD key;
            DWORD time;
			t_AddRes() 
			{
                para = PARA_ADD_RES;
                charID = 0;
                resType = 0;
                resID = 0;
                key = 0;
                time = 0;
			};
		};

        const BYTE PARA_RSP_ADD_RES = 4;
		struct t_RspAddRes : ResNullCmd 
		{
            QWORD charID;
            DWORD resType;
            DWORD resID;
            DWORD key;
            DWORD time;
            int commit;
            char url[200];
			t_RspAddRes() 
			{
                para = PARA_RSP_ADD_RES;
                charID = 0;
                resType = 0;
                resID = 0;
                key = 0;
                time = 0;
                commit = 0;
                bzero(url,sizeof(url));
			};
		};




    };
};

#pragma pack()

#endif

