/**
 * \file
 * \version  $Id: zHttpTask.cpp 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2004年11月18日 14时19分29秒 CST
 * \brief 定义实现轻量级(lightweight)的http服务框架
 */


#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>

#include "zSocket.h"
#include "zThread.h"
#include "zHttpTask.h"
#include "zHttpTaskPool.h"
#include "Fir.h"

/**
 * \brief 向套接口发送指令
 * \param pstrCmd 待发送的指令
 * \param nCmdLen 待发送指令的大小
 * \return 发送是否成功
 */
bool zHttpTask::sendCmd(const void *pstrCmd, int nCmdLen)
{
	if (pSocket) 
		return pSocket->sendCmdNoPack(pstrCmd, nCmdLen);
	else
		return false;
}

