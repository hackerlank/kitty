/**
 * \file
 * \version  $Id: zNullCmd.h 11 2013-04-01 10:17:41Z  $
 * \author  ,@163.com
 * \date 2004年12月11日 14时36分43秒 CST
 * \brief 定义空的指令
 *
 * 负责服务器内部交换使用，和客户端交互的指令需要另外定义
 * 
 */


#ifndef _zNullCmd_h_
#define _zNullCmd_h_

#include "zType.h"

#pragma pack(1)


namespace CMD
{
	const BYTE CMD_NULL = 0;		/**< 空的指令 */
	const BYTE PARA_NULL = 0;		/**< 空的指令参数 */

	/**
	 * \brief 空操作指令，测试信号和对时间指令
	 *
	 */
	struct t_NullCmd : public _null_cmd_
	{
		/**
		 * \brief 构造函数
		 *
		 */
		t_NullCmd(const BYTE _cmd = CMD_NULL, const BYTE _para = PARA_NULL) : _null_cmd_(_cmd,_para)
		{
			//cmd = _cmd;
			//para = _para;
		};
	};

	struct t_newNullCmd : public _null_cmd_
	{
		/**
		 * \brief 构造函数
		 *
		 */
		explicit t_newNullCmd(const unsigned short id) : _null_cmd_(id)
		{
		}
	};

	template <unsigned short cmdid>
		struct ServerCmdTemplate : public t_newNullCmd 
	{
		enum 
		{ 
			ID = cmdid 
		};

		ServerCmdTemplate() : t_newNullCmd(ID) {}
	};

#define	DECLARE_CMD(name,NAMESPACE)	\
name : public ServerCmdTemplate<CMD::NAMESPACE::name>

	/**
	 * \brief socket服务端空指令子编号
	 *
	 */
	const BYTE SERVER_PARA_NULL = 0;
	/**
	 * \brief socket服务端空操作指令，测试信号和对时间指令
	 *
	 */
	struct t_ServerNullCmd:public t_NullCmd
	{
		/**
		 * \brief 构造函数
		 */
		t_ServerNullCmd():t_NullCmd(CMD_NULL,SERVER_PARA_NULL)
		{
		}
	};
	/**
	 * \brief socket客户端空指令子编号
	 *
	 */
	const BYTE CLIENT_PARA_NULL = 1;
	/**
	 * \brief socket客户端空操作指令，测试信号和对时间指令
	 *
	 */
	struct t_ClientNullCmd:public t_NullCmd
	{
		/**
		 * \brief 构造函数
		 */
		t_ClientNullCmd():t_NullCmd(CMD_NULL,CLIENT_PARA_NULL)
		{
		}
	};

	/**
	 * \brief 序列化空指令 用来装载消息转发
	 *
	 */
	struct t_SeriallizeDataNullCmd : public t_NullCmd
	{
		t_SeriallizeDataNullCmd() : t_NullCmd(CMD_NULL,CLIENT_PARA_NULL),size(0)
		{

		}
		QWORD size;     //data的大小            
		BYTE data[0];   //发送的数据            
	};

	const BYTE NULL_USERCMD_PARA = 0;
	struct stNullUserCmd : public t_NullCmd {
		stNullUserCmd()
		{
			cmd = 0;
			para = NULL_USERCMD_PARA;
			dwTimestamp = 0;
		}

		DWORD   dwTimestamp;
	};

};

#pragma pack()

#endif

