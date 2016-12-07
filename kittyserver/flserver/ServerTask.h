/**
 * \file
 * \version  $Id: ServerTask.h 964 2005-04-20 14:01:41Z song $
 * \author  Songsiliang,
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 定义服务器连接任务
 */

#ifndef _ServerTask_h_
#define _ServerTask_h_

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zMisc.h"
#include "LoginCommand.h"
#include "FLCommand.h"

/**
 * \brief 服务器连接任务
 */
class ServerTask : public zTCPTask
{

	public:

		/**
		 * \brief 构造函数
		 * 用于创建一个服务器连接任务
		 * \param pool 所属的连接池
		 * \param sock TCP/IP套接口
		 */
		ServerTask(zTCPTaskPool *pool,const int sock) : zTCPTask(pool, sock, NULL, true, true)
		{
			_reg_role_count = 0;
		}

		/**
		 * \brief 虚析构函数
		 */
		~ServerTask() {};

		int verifyConn();
		int waitSync();
		void addToContainer();
		void removeFromContainer();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
		const DWORD getZoneid() const
		{
			return gameZone.zone;
		}

		const char *getLocalIP() const
		{
			return mSocket.getLocalIP();
		}

		inline DWORD getRegRoleCount(){ return _reg_role_count; }
    
    private:

        //处理FLCMD 消息
        bool msgParseFlCmd(const CMD::FL::FLNullCmd *flNullCmd,const DWORD nCmdLen);
        //处理t_GYList_FL
        bool msgParseGyList(const CMD::FL::t_GYList_FL *ptCmd, const DWORD nCmdLen);
	private:

		GameZone_t gameZone;
		std::string name;
		DWORD _reg_role_count; // 注册人数
};

#endif

