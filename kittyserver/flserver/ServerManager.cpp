/**
 * \file
 * \version  $Id: ServerManager.cpp 855 2005-04-04 13:53:18Z song $
 * \author  Songsiliang,
 * \date 2004年12月13日 18时44分39秒 CST
 * \brief 实现服务器管理容器
 *
 * 这个容器包括全局容器和唯一性验证容器
 * 
 */


#include <iostream>

#include "ServerTask.h"
#include "zRWLock.h"
#include "zNoncopyable.h"
#include "ServerManager.h"
#include "Fir.h"
#include "GYListManager.h"
#include "SlaveCommand.h"

void ServerManager::add(ServerTask *task)
{
	zRWLock_scope_wrlock scope_wrlock(rwlock);
	mapper[task->getZoneid()].insert(value_type(NetType_near, task));
}

void ServerManager::remove(ServerTask *task)
{
	zRWLock_scope_wrlock scope_wrlock(rwlock);
	NetServer_multimap &mtp = mapper[task->getZoneid()];
	for(iter it = mtp.begin(); it != mtp.end(); ++it)
	{
		if (task == it->second)
		{
			mtp.erase(it);
			break;
		}
	}
	if (mtp.empty())
	{
		//没有这个区的连接，把这个区设置为维护状态
		GYListManager::getMe().disableAll(task->getZoneid());
	}
}

bool ServerManager::sendCmdToZone(const DWORD zoneid, const void *pstrCmd, int nCmdLen)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	ZoneServer_map::iterator it = mapper.find(zoneid);
	if (it == mapper.end())
		return false;
	else
		return _sendCmdToZone(it, pstrCmd, nCmdLen);
}

// 获得注册人数最少的区
DWORD ServerManager::getRegRoleMinZone()
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	
	DWORD role_count = 0;
	DWORD zoneid = 0;
	for(ZoneServer_map::iterator itr=mapper.begin();itr!=mapper.end(); itr++)
    {
        NetServer_multimap &mtp = itr->second;
        for(NetServer_multimap::iterator itr2=mtp.begin(); itr2!=mtp.end(); itr2++)
        {
            ServerTask *task = itr2->second;
            if(task != NULL)
            {
                if(zoneid == 0 || task->getRegRoleCount() < role_count)
                {
					zoneid = itr->first;
                    role_count = task->getRegRoleCount();
				}
            }
        }
    }

	return zoneid;
}

