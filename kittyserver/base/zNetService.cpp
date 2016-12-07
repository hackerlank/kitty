/**
 * \file
 * \version  $Id: zNetService.cpp 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2004年11月29日 17时19分12秒 CST
 * \brief 实现网络服务器
 *
 * 
 */

#include <iostream>
#include <string>
#include <ext/numeric>

#include "zService.h"
#include "zThread.h"
#include "zSocket.h"
#include "zTCPServer.h"
#include "zTCPTaskPool.h"
#include "zNetService.h"
#include "Fir.h"

zNetService *zNetService::instance = NULL;

/**
 * \brief 初始化服务器程序
 *
 * 实现<code>zService::init</code>的虚函数
 *
 * \param port 端口
 * \return 是否成功
 */

bool zNetService::init(unsigned short port)
{
	if (!zService::init())
		return false;

	//Fir::logger->debug("%s", __PRETTY_FUNCTION__);

	//初始化服务器
	tcpServer = FIR_NEW zTCPServer(serviceName);
	if (NULL == tcpServer)
		return false;
	if (!tcpServer->bind(serviceName, port))
		return false;

	return true;
}

/**
 * \brief 网络服务程序的主回调函数
 *
 * 实现虚函数<code>zService::serviceCallback</code>，主要用于监听服务端口，如果返回false将结束程序，返回true继续执行服务
 *
 * \return 回调是否成功
 */
bool zNetService::serviceCallback()
{
	//Fir::logger->debug("%s", __PRETTY_FUNCTION__);

	struct sockaddr_in addr;
	int retcode = tcpServer->accept(&addr);
	if (retcode >= 0) 
	{
		//接收连接成功，处理连接
		newTCPTask(retcode, &addr);
	}

	return true;
}

/**
 * \brief 结束网络服务器程序
 *
 * 实现纯虚函数<code>zService::final</code>，回收资源
 *
 */
void zNetService::final()
{
	Fir::logger->debug("%s", __PRETTY_FUNCTION__);
	SAFE_DELETE(tcpServer);
}

