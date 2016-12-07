/**
 * \file
 * \version  $Id: LoginManager.cpp 2699 2005-08-30 13:53:37Z yhc $
 * \author  Songsiliang,
 * \date 2004年12月17日 13时17分28秒 CST
 * \brief 登陆连接管理容器
 *
 * 
 */

#include "zMisc.h"
#include "GmToolTask.h"
#include "GmToolManager.h"
#include "extractProtoMsg.h"

/**
 * \brief 向容器中添加一个连接
 *
 * \param task 一个连接任务
 * \return 添加是否成功
 */
bool GmToolTaskManager::add(GmToolTask *task)
{
	if (task)
	{
		zRWLock_scope_wrlock scope_wrlock(rwlock);
		task->genTempID();
		DWORD taskID = task->getTempID();
#if 0
		GmToolTaskHashmap_const_iterator it = gmToolTaskSet.find(task->getAccount());
		if (it != gmToolTaskSet.end())
		{
			Fir::logger->error("向容器中添加一个连接error(%s)",task->getAccount());
			return false;
		}
		std::pair<GmToolTaskHashmap_iterator, bool> p = gmToolTaskSet.insert(GmToolTaskHashmap_pair(task->getAccount(), task));
#endif
        m_gmToolIDMap.insert(std::pair<DWORD,GmToolTask*>(task->getTempID(),task));
		Fir::logger->debug("向容器中添加一个连接,ID:%d,account:%s", taskID,task->getAccount());
        return true;
		//return p.second;
	}
	else
	{
		Fir::logger->error("连接任务error");
		return false;
	}
}

/**
 * \brief 从一个容器中移除一个连接
 *
 * \param task 一个连接任务
 */
void GmToolTaskManager::remove(GmToolTask *task)
{
	if (task)
	{
		zRWLock_scope_wrlock scope_wrlock(rwlock);
		gmToolTaskSet.erase(task->getAccount());
        m_gmToolIDMap.erase(task->getTempID());
	}
}

/**
 * \brief 广播指令到指定的登陆连接
 *
 * \param loginTempID 登陆连接的唯一编号
 * \param pstrCmd 待转发的指令
 * \param nCmdLen 待转发的指令长度
 * \return 转发是否成功
 */
bool GmToolTaskManager::broadcast(const char *account, const void *pstrCmd, int nCmdLen)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	GmToolTaskHashmap_iterator it = gmToolTaskSet.find(account);
	if (it != gmToolTaskSet.end())
    {
		return it->second->sendCmd(pstrCmd, nCmdLen);
    }
	else
	{
		Fir::logger->error("广播指令到指定的登陆连接error");
		return false;
	}
}


/**
 * \brief 对容器中的所有元素调用回调函数
 * \param cb 回调函数实例
 */
void GmToolTaskManager::execAll(GmToolTaskCallback &cb)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	for(GmToolTaskHashmap_iterator it = gmToolTaskSet.begin(); it != gmToolTaskSet.end(); ++it)
	{
		cb.exec(it->second);
	}
}

GmToolTask* GmToolTaskManager::getTask(const char *account)
{
    GmToolTask *ret = NULL;
	zRWLock_scope_rdlock scope_rdlock(rwlock);
    GmToolTaskHashmap_iterator it = gmToolTaskSet.find(account);
    if(it != gmToolTaskSet.end())
    {
        ret = it->second;
    }
    return ret;
}

GmToolTask* GmToolTaskManager::getTask(const DWORD id)
{
    GmToolTask *ret = NULL;
	zRWLock_scope_rdlock scope_rdlock(rwlock);
    auto iter = m_gmToolIDMap.find(id);
    if(iter != m_gmToolIDMap.end())
    {
        ret = const_cast<GmToolTask*>(iter->second);
    }
    return ret;
}


void GmToolTaskManager::update()
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
    for(GmToolTaskHashmap_iterator it = gmToolTaskSet.begin();it != gmToolTaskSet.end();++it)
    {
        GmToolTask *ret = it->second;
        if(ret)
        {
            ret->save();
        }
    }
}
