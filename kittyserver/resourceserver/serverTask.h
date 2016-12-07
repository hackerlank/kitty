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
#include "ResourceCommand.h"

/**
 * \brief 服务器连接任务
 */
class ServerTask : public zTCPTask
{

	public:
		ServerTask(zTCPTaskPool *pool,const int sock) : zTCPTask(pool, sock, NULL, true, true) {};
		~ServerTask() {};
		int verifyConn();
		int waitSync();
		void addToContainer();
		void removeFromContainer();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
		inline const DWORD getZoneID() const
		{
			return gameZone.zone;
		}
		inline const char *getLocalIP() const
		{
			return mSocket.getLocalIP();
		}
    private:
        bool check(const char *strIP, const unsigned short port);
        bool msgParseResourceCmd(const CMD::RES::ResNullCmd *resNullCmd, const DWORD nCmdLen);
	private:
		GameZone_t gameZone;
		std::string name;
};

#endif

