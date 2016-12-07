/**
 * \file
 * \version  $Id: ServerManager.h 855 2005-04-04 13:53:18Z song $
 * \author  Songsiliang,
 * \date 2004年12月13日 18时44分39秒 CST
 * \brief 定义服务器管理容器
 *
 * 这个容器包括全局容器和唯一性验证容器
 * 
 */


#ifndef _ServerManager_h_
#define _ServerManager_h_

#include <iostream>
#include <map>

#include "serverTask.h"
#include "zRWLock.h"
#include "zNoncopyable.h"
#include "NetType.h"

/**
 * \brief 服务器管理容器类
 *
 * 这个容器包括全局容器和唯一性验证容器
 *
 */
class ServerManager : public Singleton<ServerManager>
{

	public:
		void add(ServerTask *task);
		void remove(ServerTask *task);
        bool sendCmd(const void *pstrCmd, const DWORD nCmdLen);
	private:

		friend class Singleton<ServerManager>;
		ServerManager() {};
		~ServerManager() {};
		std::map<WORD,ServerTask*> serverTaskMap;
		zRWLock rwlock;
};

#endif

