/**
 * \file
 * \version  $Id: GatewayTaskManager.cpp 24 2013-03-30 08:04:25Z  $
 * \author  ,@163.com
 * \date 2004年12月20日 17时28分53秒 CST
 * \brief 管理子连接的容器
 *
 * 
 */


#include <iostream>

#include "GatewayTask.h"
#include "GatewayTaskManager.h"
#include "zXMLParser.h"
#include "TimeTick.h"

///网关连接管理器实例
GatewayTaskManager *GatewayTaskManager::instance = NULL;

GatewayTaskManager::GatewayTaskManager()
{
}

GatewayTaskManager::~GatewayTaskManager()
{
}

/**
 * \brief 向容器中添加一个子连接
 *
 * \param task 子连接任务
 * \return 添加是否成功
 */
bool GatewayTaskManager::uniqueAdd(GatewayTask *task)
{

    //先断开旧的连接
    std::pair<DWORD, std::string> key(task->acctype, task->account);
    auto iter = account_map.find(key);
    if(iter != account_map.end())
    {
        GatewayTask *oldTask = const_cast<GatewayTask*>(iter->second);
        if(oldTask && oldTask->m_pUser)
        {
            oldTask->m_pUser->Terminate();
        }
    }
	rwlock.wrlock();
	if (!account_map.insert(std::make_pair(std::pair<DWORD, std::string>(task->acctype, task->account),task)).second)
	{
		rwlock.unlock();
		return false;
	}
	rwlock.unlock();

	return true;
}

/**
 * \brief 从容器中删除一个子连接
 *
 * \param task 子连接任务
 * \return 删除是否成功
 */
bool GatewayTaskManager::uniqueRemove(GatewayTask *task)
{
	rwlock.wrlock();

	auto it = account_map.find(std::pair<DWORD,std::string>(task->acctype, task->account));
	if (it != account_map.end())
	{
		account_map.erase(it);
	}
	else
	{
		Fir::logger->warn(__PRETTY_FUNCTION__);
	}

	rwlock.unlock();

	return true;
}


/**
 * \brief 遍历容器中的所有元素，执行某一个操作
 * \param callback 待执行的回调函数
 */
void GatewayTaskManager::execAll(GatewayTaskCallback &callback)
{
	rwlock.rdlock();
	for(GatewayTaskHashmap_iterator it = gatewayTaskSet.begin(); it != gatewayTaskSet.end(); ++it)
	{
		if (!callback.exec(it->second))
			break;
	}

	for(uuid_hashmap_iterator it = gatewaytask_uuidset.begin(); it != gatewaytask_uuidset.end(); ++it)
	{
		if (!callback.exec(it->second))
			break;
	}

	for (auto it = account_map.begin(); it != account_map.end(); ++it)
	{
		if (!callback.exec(it->second))
			break;
	}
	rwlock.unlock();
}

void GatewayTaskManager::execAllCheckTime()
{
	std::list<GatewayTask*> _remove;

	rwlock.rdlock();

	for(GatewayTaskHashmap_iterator it = gatewayTaskSet.begin(); it != gatewayTaskSet.end(); ++it)
	{
		GatewayTask* gt=it->second;
		gt->checkTime(GatewayTimeTick::currentTime);

		if (gt->checkTime(GatewayTimeTick::currentTime))
		{
			_remove.push_back(gt);
		}
	}

	for(uuid_hashmap_iterator it = gatewaytask_uuidset.begin(); it != gatewaytask_uuidset.end(); ++it)
	{
		GatewayTask* gt=it->second;
		gt->checkTime(GatewayTimeTick::currentTime);

		if (gt->checkTime(GatewayTimeTick::currentTime))
		{
			_remove.push_back(gt);
		}
	}

	for (auto it = account_map.begin(); it != account_map.end(); ++it)
	{
		GatewayTask* gt=it->second;
		gt->checkTime(GatewayTimeTick::currentTime);

		if (gt->checkTime(GatewayTimeTick::currentTime))
		{
			_remove.push_back(gt);
		}
	}

	rwlock.unlock();

	std::list<GatewayTask*>::iterator iter = _remove.begin();
	for (; iter != _remove.end(); ++iter)
	{
		(*iter)->TerminateWait();
	}
}

