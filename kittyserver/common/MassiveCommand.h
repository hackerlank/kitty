#ifndef _MASSIVE_COMMAND_H_
#define _MASSIVE_COMMAND_H_

#include "zType.h"
#include "Command.h"
#include "nullcmd.h"
#include "common.h"
#pragma pack(1)

namespace CMD
{ 
	//返回错误码
	struct ELEGANT_DECLARE_CMD(stRetErrorOperationUserCmd_SC)
	{
		stRetErrorOperationUserCmd_SC()
		{
			cmd_id = 0;;	
		}
		DWORD cmd_id;
	};
}
#pragma pack()

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif

