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
#include "resourceTaskManager.h"
#include "extractProtoMsg.h"

/**
 * \brief 向容器中添加一个连接
 *
 * \param task 一个连接任务
 * \return 添加是否成功
 */
bool ResourceTaskManager::add(ResourceTask *task)
{
	if (task)
	{
		zRWLock_scope_wrlock scope_wrlock(rwlock);
		task->genTempID();
		DWORD taskID = task->getTempID();
        m_resourceIDMap.insert(std::pair<DWORD,ResourceTask*>(task->getTempID(),task));
		Fir::logger->debug("向容器中添加一个连接,ID:%d", taskID);
        return true;
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
void ResourceTaskManager::remove(ResourceTask *task)
{
	if (task)
	{
		zRWLock_scope_wrlock scope_wrlock(rwlock);
        m_resourceIDMap.erase(task->getTempID());
	}
}

/**
 * \brief 对容器中的所有元素调用回调函数
 * \param cb 回调函数实例
 */
void ResourceTaskManager::execAll(ResourceTaskCallback &cb)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	for(auto it = m_resourceIDMap.begin(); it != m_resourceIDMap.end(); ++it)
	{
		cb.exec(it->second);
	}
}

ResourceTask* ResourceTaskManager::getTask(const DWORD id)
{
    ResourceTask *ret = NULL;
	zRWLock_scope_rdlock scope_rdlock(rwlock);
    auto iter = m_resourceIDMap.find(id);
    if(iter != m_resourceIDMap.end())
    {
        ret = const_cast<ResourceTask*>(iter->second);
    }
    return ret;
}

ResourceTask* ResourceTaskManager::getTaskByMod(const QWORD charID)
{
    DWORD mod = charID % m_resourceIDMap.size() + 1;
    return getTask(mod);
}
