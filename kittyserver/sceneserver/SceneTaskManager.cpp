/**
 * \file
 * \version  $Id: SceneTaskManager.cpp 36 2013-04-07 11:42:48Z  $
 * \author  ,
 * \date 2013年04月07日 15时28分53秒 CST
 * \brief 管理场景连接的容器
 *
 * 
 */


#include <iostream>

#include "SceneTaskManager.h"

SceneTaskManager *SceneTaskManager::instance = NULL;

/**
 * \brief 向唯一性验证容器中添加一个子连接任务
 *
 * \param task 子连接任务
 * \return 添加连接是否成功
 */
bool SceneTaskManager::uniqueAdd(SceneTask *task)
{
	SceneTaskHashmap_const_iterator it;
	rwlock.wrlock();
	it = sceneTaskSet.find(task->getID());
	if (it != sceneTaskSet.end())
	{
		Fir::logger->error(__PRETTY_FUNCTION__);
		rwlock.unlock();
		return false;
	}
	sceneTaskSet.insert(SceneTaskHashmap_pair(task->getID(), task));
	rwlock.unlock();
	return true;
}

/**
 * \brief 从唯一性容器中移除一个子连接任务
 *
 * \param task 子连接任务
 * \return 移除是否成功
 */
bool SceneTaskManager::uniqueRemove(SceneTask *task)
{
	SceneTaskHashmap_iterator it;
	rwlock.wrlock();
	it = sceneTaskSet.find(task->getID());
	if (it != sceneTaskSet.end())
	{
		sceneTaskSet.erase(it);
	}
	else
		Fir::logger->warn(__PRETTY_FUNCTION__);
	rwlock.unlock();
	return true;
}

/**
 * \brief 根据服务器编号广播指令
 *
 * \param wdServerID 待广播指令的服务器编号
 * \param pstrCmd 待广播的指令
 * \param nCmdLen 指令长度
 * \return 广播是否成功
 */
bool SceneTaskManager::broadcastByID(const WORD wdServerID, const void *pstrCmd, const DWORD nCmdLen)
{
	bool retval = true;
	SceneTaskHashmap_iterator it;
	rwlock.rdlock();
	it = sceneTaskSet.find(wdServerID);
	if (it != sceneTaskSet.end())
	{
		retval = it->second->sendCmd(pstrCmd, nCmdLen);
	}
	rwlock.unlock();
	return retval;
}

/**
 * \brief 根据服务器编号查找task
 *
 * \param wdServerID 待查找的服务器编号
 * \return 广播是否成功
 */
SceneTask *SceneTaskManager::getTaskByID(const WORD wdServerID)
{
	SceneTask *ret=NULL;
	SceneTaskHashmap_iterator it;
	rwlock.rdlock();
	it = sceneTaskSet.find(wdServerID);
	if (it != sceneTaskSet.end())
	{
		ret = it->second;
	}
	rwlock.unlock();
	return ret;
}

void SceneTaskManager::execEvery()
{
	SceneTaskHashmap_iterator it;
	SceneTask *task=NULL;
	rwlock.rdlock();
	it = sceneTaskSet.begin();
	for (; it != sceneTaskSet.end() ; it ++)
	{
		task=it->second;
		if(!task->checkRecycle())
		{
			task->doCmd();
		}
	}
	rwlock.unlock();
}

bool SceneTaskManager::broadcastUserCmdToGateway(const QWORD charid,const void *pstrCmd, const DWORD nCmdLen)
{
	bool retval = true;
	SceneTaskHashmap_iterator it;
	rwlock.rdlock();
    for(SceneTaskHashmap_iterator it = sceneTaskSet.begin();it != sceneTaskSet.end();++it)
    {
        SceneTask *task = it->second;
        if(task && task->getType() == GATEWAYSERVER)
        {
            task->sendCmdToUser(charid,pstrCmd, nCmdLen);
        }
    }
	rwlock.unlock();
	return retval;
}

bool SceneTaskManager::broadcastUserCmdToGateway(const DWORD serverID,const QWORD charid,const void *pstrCmd, const DWORD nCmdLen)
{
	bool retval = false;
    SceneTask *task = getTaskByID(serverID);
	rwlock.rdlock();
    if(task && task->getType() == GATEWAYSERVER)
    {
        task->sendCmdToUser(charid,pstrCmd, nCmdLen);
        retval = true;
    }
	rwlock.unlock();
	return retval;
}

bool SceneTaskManager::broadcastUserCmdToGateway(const void *pstrCmd, const DWORD nCmdLen)
{
	bool retval = false;
    SceneTaskHashmap_iterator it;
	rwlock.rdlock();
    for(SceneTaskHashmap_iterator iter = sceneTaskSet.begin();iter != sceneTaskSet.end();++iter)
    {
        SceneTask *task = iter->second;
        if(task && task->getType() == GATEWAYSERVER)
        {
            task->sendCmd(pstrCmd, nCmdLen);
            retval = true;
        }
    }
	rwlock.unlock();
	return retval;
}
