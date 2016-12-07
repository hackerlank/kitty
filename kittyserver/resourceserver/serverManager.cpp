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

#include "serverTask.h"
#include "zRWLock.h"
#include "zNoncopyable.h"
#include "serverManager.h"
#include "Fir.h"

void ServerManager::add(ServerTask *task)
{
	zRWLock_scope_wrlock scope_wrlock(rwlock);
    serverTaskMap.insert(std::pair<WORD,ServerTask*>(task->getZoneID(), task));
}

void ServerManager::remove(ServerTask *task)
{
	zRWLock_scope_wrlock scope_wrlock(rwlock);
    serverTaskMap.erase(task->getZoneID());
}

bool ServerManager::sendCmd(const void *pstrCmd, const DWORD nCmdLen)
{
    bool ret = false;
	zRWLock_scope_wrlock scope_wrlock(rwlock);
    for(auto iter = serverTaskMap.begin();iter != serverTaskMap.end();++iter)
    {
        ServerTask *task = iter->second;
        if(task)
        {
            ret = task->sendCmd(pstrCmd,nCmdLen);
        }
    }
    return true;
}
