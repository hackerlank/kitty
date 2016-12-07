/**
 * \file
 * \version  $Id: zTCPClient.cpp 42 2013-04-10 07:33:59Z  $
 * \author  ,@163.com
 * \date 2004年11月04日 17时25分13秒 CST
 * \brief 实现类zTCPClient，TCP连接客户端。
 *
 * 
 */


#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>

#include "zTCPClient.h"
#include "Fir.h"
#include "extractProtoMsg.h"

//CmdAnalysis zTCPClient::analysis("Client指令发送统计",600);
/**
 * \brief 建立一个到服务器的TCP连接
 *
 *
 * \return 连接是否成功
 */
bool zTCPClient::connect()
{
	int retcode;
	int nSocket;
	struct sockaddr_in addr;

	nSocket = ::socket(PF_INET, SOCK_STREAM, 0);
	if (-1 == nSocket)
	{
		Fir::logger->error("创建套接口失败: %s", strerror(errno));
		return false;
	}

	//设置套接口发送接收缓冲，并且客户端的必须在connect之前设置
	socklen_t window_size = 128 * 1024;
	retcode = ::setsockopt(nSocket, SOL_SOCKET, SO_RCVBUF, &window_size, sizeof(window_size));
	if (-1 == retcode)
	{
		TEMP_FAILURE_RETRY(::close(nSocket));
		return false;
	}
	retcode = ::setsockopt(nSocket, SOL_SOCKET, SO_SNDBUF, &window_size, sizeof(window_size));
	if (-1 == retcode)
	{
		TEMP_FAILURE_RETRY(::close(nSocket));
		return false;
	}

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	retcode = TEMP_FAILURE_RETRY(::connect(nSocket, (struct sockaddr *) &addr, sizeof(addr)));
	if (-1 == retcode)
	{
		Fir::logger->error("创建到服务器 %s:%u 的连接失败", ip.c_str(), port);
		TEMP_FAILURE_RETRY(::close(nSocket));
		return false;
	}

	pSocket = FIR_NEW zSocket(nSocket, &addr, compress);
	if (NULL == pSocket)
	{
		Fir::logger->fatal("没有足够的内存，不能创建zSocket实例");
		TEMP_FAILURE_RETRY(::close(nSocket));
		return false;
	}

	Fir::logger->info("创建到服务器 %s:%u 的连接成功", ip.c_str(), port);

	return true;
}

/**
 * \brief 向套接口发送指令
 *
 *
 * \param pstrCmd 待发送的指令
 * \param nCmdLen 待发送指令的大小
 * \return 发送是否成功
 */
bool zTCPClient::sendCmd(const void *pstrCmd, const int nCmdLen)
{
	if (NULL == pSocket) 
		return false;
	else
	{
		/*
		CMD::t_NullCmd *ptNullCmd = (CMD::t_NullCmd *)pstrCmd;
		analysis.add(ptNullCmd->cmd,ptNullCmd->para,nCmdLen);
		// */
		return pSocket->sendCmd(pstrCmd, nCmdLen);
	}
}
/**
 * \brief 重载zThread中的纯虚函数，是线程的主回调函数，用于处理接收到的指令
 *
 */
void zTCPClient::run()
{
	//Fir::logger->debug("%s", __PRETTY_FUNCTION__);
	while(!isFinal())
	{
		BYTE acceptCmd[zSocket::MAX_DATASIZE];
		int nCmdLen;

		nCmdLen = pSocket->recvToCmd(acceptCmd, zSocket::MAX_DATASIZE, false);
		if (nCmdLen > 0) 
		{
            msgParse(acceptCmd,nCmdLen); 
		}
		else if (-1 == nCmdLen)
		{
			//接收指令失败，退出循环，结束线程
			Fir::logger->error("接收指令失败，关闭 %s", getThreadName().c_str());
			break;
		}
	}
}

bool zTCPBufferClient::sendCmd(const void *pstrCmd, const DWORD nCmdLen)
{
	if (pSocket)
		return pSocket->sendCmd(pstrCmd, nCmdLen, _buffered);
	else
		return false;
}

bool zTCPBufferClient::ListeningRecv()
{
	int retcode = pSocket->recvToBuf_NoPoll();
	if (-1 == retcode)
	{
		Fir::logger->error("%s", __PRETTY_FUNCTION__);
		return false;
	}
	else
	{
		do
		{
			//这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
			BYTE acceptCmd[zSocket::MAX_DATASIZE];
            int nCmdLen = pSocket->recvToCmd_NoPoll(acceptCmd, sizeof(acceptCmd));
			if (nCmdLen <= 0)
				break;
			else
			{
                if(!msgParse(acceptCmd,nCmdLen))
                {
                    return false;
                }
			}
		}
		while(true);
	}
	return true;
}

bool zTCPBufferClient::ListeningSend()
{
	if (pSocket)
		return pSocket->sync();
	else
		return false;
}

void zTCPBufferClient::sync()
{
	if (pSocket)
		pSocket->force_sync();
}

#if 0
void zTCPBufferClient::run()
{
	_buffered = true;
	struct pollfd pfds;
	struct pollfd pfds_r;
	pSocket->fillPollFD(pfds, POLLIN | POLLOUT | POLLERR | POLLPRI);
	pSocket->fillPollFD(pfds_r, POLLIN | POLLERR | POLLPRI);
	//long time=usleep_time;
	zRTime currentTime;
	zRTime _1_msec(currentTime), _50_msec(currentTime);
	while(!isFinal())
	{
		//struct timespec _tv_1;
		//struct timespec _tv_2;
		//clock_gettime(CLOCK_REALTIME, &_tv_1);
		//if (TEMP_FAILURE_RETRY(::poll(&pfds_r, 1, time/1000)) > 0)
		zThread::msleep(2);
		currentTime.now();
		if (_1_msec.elapse(currentTime) >= 2)
		{
			_1_msec = currentTime;
			if (TEMP_FAILURE_RETRY(::poll(&pfds_r, 1, 0)) > 0)
			{
				if (pfds_r.revents & (POLLERR | POLLPRI))
				{
					//套接口出现错误
					Fir::logger->fatal("%s: 套接口错误", __PRETTY_FUNCTION__);
					break;
				}
				else
				{
					if (pfds_r.revents & POLLIN)
					{
						//套接口准备好了读取操作
						if (!ListeningRecv())
						{
							Fir::logger->debug("%s: 套接口读操作错误", __PRETTY_FUNCTION__);
							break;
						}
					}
				}
			}
		}
		//clock_gettime(CLOCK_REALTIME, &_tv_2);
		//unsigned long end=_tv_2.tv_sec*1000000L + _tv_2.tv_nsec/1000L;
		//unsigned long begin= _tv_1.tv_sec*1000000L + _tv_1.tv_nsec/1000L;
		//time = time - (end - begin);
		//if(time <= 0)
		//
		if (_50_msec.elapse(currentTime) >= (usleep_time/1000))
		{
			_50_msec = currentTime;
			if (TEMP_FAILURE_RETRY(::poll(&pfds, 1, 0)) > 0)
			{
				if (pfds.revents & (POLLERR | POLLPRI))
				{
					//套接口出现错误
					Fir::logger->fatal("%s: 套接口错误", __PRETTY_FUNCTION__);
					break;
				}
				else
				{
					if (pfds.revents & POLLIN)
					{
						//套接口准备好了读取操作
						if (!ListeningRecv())
						{
							Fir::logger->debug("%s: 套接口读操作错误", __PRETTY_FUNCTION__);
							break;
						}
					}
					if (pfds.revents & POLLOUT)
					{
						//套接口准备好了写入操作
						if (!ListeningSend())
						{
							Fir::logger->debug("%s: 套接口写操作错误", __PRETTY_FUNCTION__);
							break;
						}
					}
				}
			}
			//time = usleep_time;
		}
	}

	//保证缓冲的数据发送完成
	sync();
	_buffered = false;
}
#endif

/**
 * \brief 采用EPOLL轮询
 *
 */
void zTCPBufferClient::run()
{
	_buffered = true;
	int epfd = epoll_create(256); assert(-1 != epfd);
	int epfd_r = epoll_create(256); assert(-1 != epfd_r);
	pSocket->addEpoll(epfd, EPOLLOUT | EPOLLIN | EPOLLERR | EPOLLPRI, NULL);
	pSocket->addEpoll(epfd_r, EPOLLIN | EPOLLERR | EPOLLPRI, NULL);
	struct epoll_event ep_event, ep_event_r;
	ep_event.events = 0, ep_event_r.events = 0;

	zRTime currentTime;
	zRTime _1_msec(currentTime), _50_msec(currentTime);

	while(!isFinal())
	{
		zThread::msleep(2);
		currentTime.now();
		if (_1_msec.elapse(currentTime) >= 2)
		{
			_1_msec = currentTime;
			if (TEMP_FAILURE_RETRY(::epoll_wait(epfd_r, &ep_event_r, 1, 0)) > 0)
			{
				if (ep_event_r.events & (EPOLLERR | EPOLLPRI))
				{
					//套接口出现错误
					Fir::logger->fatal("%s: 套接口错误", __PRETTY_FUNCTION__);
					break;
				}
				else
				{
					if (ep_event_r.events & EPOLLIN)
					{
						//套接口准备好了读取操作
						if (!ListeningRecv())
						{
							Fir::logger->debug("%s: 套接口读操作错误", __PRETTY_FUNCTION__);
							break;
						}
					}
				}
				ep_event_r.events = 0;
			}
		}
	
		if (_50_msec.elapse(currentTime) >= (usleep_time/1000))
		{
			_50_msec = currentTime;
			if (TEMP_FAILURE_RETRY(::epoll_wait(epfd, &ep_event, 1,0)) > 0)
			{
				if (ep_event.events & (EPOLLERR | EPOLLPRI))
				{
					//套接口出现错误
					Fir::logger->fatal("%s: 套接口错误", __PRETTY_FUNCTION__);
					break;
				}
				else
				{
					if (ep_event.events & EPOLLIN)
					{
						//套接口准备好了读取操作
						if (!ListeningRecv())
						{
							Fir::logger->debug("%s: 套接口读操作错误", __PRETTY_FUNCTION__);
							break;
						}
					}
					if (ep_event.events & EPOLLOUT)
					{
						//套接口准备好了写入操作
						if (!ListeningSend())
						{
							Fir::logger->debug("%s: 套接口写操作错误", __PRETTY_FUNCTION__);
							break;
						}
					}
				}
				ep_event.events = 0;
			}
			//time = usleep_time;
		}
	}
	

	//保证缓冲的数据发送完成
	sync();
	_buffered = false;
}



bool zTCPBufferClient::msgParseNullCmd(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if (CMD::CMD_NULL == ptNullCmd->cmd)
    {
        if (CMD::PARA_NULL == ptNullCmd->para)
        {
            Fir::logger->debug("客户端收到测试信号:(%s:%u)", ip.c_str(), port);
            std::string ret;
            if(encodeMessage(ptNullCmd,nCmdLen,ret))
            {
                sendCmd(ret.c_str(),ret.size());
            }
        }
        return true;
    }
    return false;
}
