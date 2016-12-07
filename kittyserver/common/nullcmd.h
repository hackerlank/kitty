/**
 * \file
 * \version  $Id: nullcmd.h 12 2013-04-02 13:15:24Z chengxie $
 * \author  ,@163.com
 * \date 2004年12月11日 14时36分43秒 CST
 * \brief 定义空的指令
 *
 * 负责服务器内部交换使用，和客户端交互的指令需要另外定义
 * 
 */


#ifndef __NULLCMD_H__
#define __NULLCMD_H__

#include "zType.h"
#include "cmd_id.h"

#pragma pack(1)

namespace CMD {

	/**
	 * \brief 空指令，所有指令的基础结构
	 */
	struct NullCmd : public _null_cmd_ 
    {
        union 
        {
            unsigned int	_timestamp;
			unsigned int 	dwTimestamp;
		};

		NullCmd(const unsigned short id) : _null_cmd_(id), _timestamp(0)
		{
		}
	};

	template <unsigned short cmdid>
    struct CmdTemplate : public NullCmd 
    {
        enum 
        {
            ID = cmdid
        };
        CmdTemplate() : NullCmd(ID) {}
    };

#define	ELEGANT_DECLARE_CMD(name)	 name : public CmdTemplate<CMD::ID::name>
}

#pragma pack()

#endif

