/**
 * \file
 * \version  $Id$
 * \author  吴志勇,wuzhiyong@ztgame.com 
 * \date 定义实现轻量级(lightweight)的http服务框架(可以保持连接)
 * \brief 
 *
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>

#include "zSocket.h"
#include "zThread.h"
#include "zNewHttpTask.h"
#include "zNewHttpTaskPool.h"
#include "Fir.h"

/**
 * \brief 向套接口发送指令
 * \param pstrCmd 待发送的指令
 * \param nCmdLen 待发送指令的大小
 * \return 发送是否成功
 */
bool zNewHttpTask::sendCmd(const void *pstrCmd, int nCmdLen)
{
	if (pSocket) 
		return pSocket->sendRawDataIM(pstrCmd, nCmdLen);
	else
		return false;
}

const int zNewHttpTask::getPoolSize()
{
	return pool->getSize();
} 

const int zNewHttpTask::getPoolMaxSize()
{
	return pool->getMaxConns();
}
