/**
 * \file
 * \version  $Id: zMTCPServer.cpp 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2004年11月02日 17时31分02秒 CST
 * \brief 实现类zMTCPServer
 *
 * 
 */

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#include "zSocket.h"
#include "zMTCPServer.h"
#include "Fir.h"

/**
 * \brief 构造函数，用于构造一个服务器zMTCPServer对象
 * \param name 服务器名称
 */
zMTCPServer::zMTCPServer(const std::string &name) : name(name)
{
	kdpfd = epoll_create(1);
	assert(-1 != kdpfd);
	epfds.resize(8);
}

/**
 * \brief 析构函数，用于销毁一个zMTCPServer对象
 */
zMTCPServer::~zMTCPServer() 
{
	TEMP_FAILURE_RETRY(::close(kdpfd));

	for(Sock2Port_const_iterator it = mapper.begin(); it != mapper.end(); ++it)
	{
		if (-1 != it->first)
		{
			::shutdown(it->first, SHUT_RD);
			TEMP_FAILURE_RETRY(::close(it->first));
		}
	}
	mapper.clear();
}

/**
 * \brief 绑定监听服务到某一个端口
 * \param name 绑定端口名称
 * \param port 具体绑定的端口
 * \return 绑定是否成功
 */
bool zMTCPServer::bind(const std::string &name, const unsigned short port) 
{
	zMutex_scope_lock scope_lock(mlock);
	struct sockaddr_in addr;
	int sock;

	for(Sock2Port_const_iterator it = mapper.begin(); it != mapper.end(); ++it)
	{
		if (it->second == port)
		{
			Fir::logger->warn("端口 %u 已经绑定服务",port);
			return false;
		}
	}

	sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock) 
	{
		Fir::logger->error("创建套接口失败");
		return false;
	}

	//设置套接口为可重用状态
	int reuse = 1;
	if (-1 == ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) 
	{
		Fir::logger->error("不能设置套接口为可重用状态");
		TEMP_FAILURE_RETRY(::close(sock));
		return false;
	}

	//设置套接口发送接收缓冲，并且服务器的必须在accept之前设置
	socklen_t window_size = 128 * 1024;
	if (-1 == ::setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &window_size, sizeof(window_size)))
	{
		TEMP_FAILURE_RETRY(::close(sock));
		return false;
	}
	if (-1 == ::setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &window_size, sizeof(window_size)))
	{
		TEMP_FAILURE_RETRY(::close(sock));
		return false;
	}

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	int retcode = ::bind(sock, (struct sockaddr *) &addr, sizeof(addr));
	if (-1 == retcode) 
	{
		Fir::logger->error("不能绑定服务器端口%u",port);
		TEMP_FAILURE_RETRY(::close(sock));
		return false;
	}

	retcode = ::listen(sock, MAX_WAITQUEUE);
	if (-1 == retcode) 
	{
		Fir::logger->error("监听套接口失败");
		TEMP_FAILURE_RETRY(::close(sock));
		return false;
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sock;
	assert(0 == epoll_ctl(kdpfd, EPOLL_CTL_ADD, sock, &ev));

	mapper.insert(Sock2Port_value_type(sock, port));
	if (mapper.size() > epfds.size())
	{
		epfds.resize(mapper.size() + 8);
	}

	Fir::logger->info("服务器 %s:%u 端口初始化绑定成功", name.c_str(), port);

	return true;
}

/**
 * \brief 接受客户端的连接
 * \param res 返回的连接集合
 * \return 接收到的连接个数
 */
int zMTCPServer::accept(Sock2Port &res)
{
	zMutex_scope_lock scope_lock(mlock);
	int retval = 0;

	int rc = epoll_wait(kdpfd, &epfds[0], mapper.size(), T_MSEC);
	if (rc > 0)
	{
		for(int i = 0; i < rc; i++)
		{
			if (epfds[i].events & EPOLLIN)
			{
				res.insert(Sock2Port_value_type(TEMP_FAILURE_RETRY(::accept(epfds[i].data.fd, NULL, NULL)), mapper[epfds[i].data.fd]));
				retval++;
			}
		}
	}

	return retval;
}

