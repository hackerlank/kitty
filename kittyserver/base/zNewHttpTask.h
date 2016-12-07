/**
 * \file
 * \version  $Id$
 * \author  吴志勇,wuzhiyong@ztgame.com 
 * \date 定义实现轻量级(lightweight)的http服务框架(可以保持连接)
 * \brief 
 *
 */


#ifndef _zNewHttpTask_h_
#define _zNewHttpTask_h_

#include <string>
#include <vector>
#include <queue>
#include <list>
#include <unistd.h>
#include <sys/timeb.h>

#include "zSocket.h"
#include "zNoncopyable.h"
#include "zTime.h"

class zNewHttpTaskPool;

/**
 * \brief 定义轻量级http任务类，封装一些低层接口
 */
class zNewHttpTask : private zNoncopyable
{

	public:
		/**
		 * \brief 连接断开方式
		 *
		 */
		enum TerminateMethod
		{
			terminate_no,								/**< 没有结束任务 */
			terminate_active,							/**< 客户端主动断开连接，主要是由于服务器端检测到套接口关闭或者套接口异常 */
			terminate_passive							/**< 服务器端主动断开连接 */
		};
		
		/**
		 * \brief 构造函数，用于创建一个对象
		 * \param pool 所属连接池指针
		 * \param sock 套接口
		 * \param addr 地址
		 */
		zNewHttpTask(
				zNewHttpTaskPool *pool,
				const int sock,
				const struct sockaddr_in *addr = NULL) : pool(pool), lifeTime()
		{
			terminate = terminate_no;
			pSocket = new zSocket(sock, addr, false);
			state = verify;
		}

		/**
		 * \brief 析构函数，用于销毁一个对象
		 */
		virtual ~zNewHttpTask()
		{
			SAFE_DELETE(pSocket);
		}

#ifdef _USE_EPOLL_
		/**
		 * \brief 添加检测事件到epoll描述符
		 * \param kdpfd epoll描述符
		 * \param events 待添加的事件
		 * \param ptr 额外参数
		 * \return 操作是否成功
		 */
		bool addEpoll(int kdpfd, __uint32_t events, void *ptr)
		{
			if (pSocket)
				return pSocket->addEpoll(kdpfd, events, ptr);
			else
				return false;
		}
		/**
		 * \brief 从epoll描述符中删除检测事件
		 * \param kdpfd epoll描述符
		 * \param events 待添加的事件
		 * \return 操作是否成功
		 */
		bool delEpoll(int kdpfd, __uint32_t events)
		{
			if (pSocket)
				return pSocket->delEpoll(kdpfd, events);
			else
				return false;
		}
#else
		/**
		 * \brief 填充pollfd结构
		 * \param pfd 待填充的结构
		 * \param events 等待的事件参数
		 * \return 填充是否成功
		 */
		bool fillPollFD(struct pollfd &pfd, short events)
		{
			if (pSocket)
				{pSocket->fillPollFD(pfd, events);return true;}
			else
				return false;
		}
#endif

		/**
		 * \brief 检测是否验证超时
		 * \param ct 当前系统时间
		 * \param interval 超时时间，毫秒
		 * \return 检测是否成功
		 */
		bool checkHttpTimeout(const zRTime &ct, const unsigned long long interval = 2000) const
		{
			return (lifeTime.elapse(ct) > interval);
		}

		/**
		 * \brief http任务主处理函数
		 * \return 是否成功，1表示还要需要其它服务器的处理，0，表示还要继续等待，-1，表示失败(处理结束)
		 */
		virtual int httpCore()
		{
			return -1;
		}

		bool sendCmd(const void *pstrCmd, int nCmdLen);

		/**
		 * \brief 判断是否需要关闭连接
		 *
		 * \return true or false
		 */
		bool isTerminate() const
		{
			return terminate_no != terminate;
		}

		/**
		 * \brief 需要主动断开客户端的连接
		 *
		 * \param method 连接断开方式
		 */
		void Terminate(const TerminateMethod method = terminate_passive)
		{
			terminate = method;
		}

		/**
		 * \brief 连接任务状态
		 */
		enum zNewHTTPTask_State
		{
			verify		=	0,							/**< 只在本服务器处理验证的状态 */
			sync		=	1,							/**< 需要等待来自其它服务器的验证信息同步 状态*/
		};

		/**
		 * \brief 获取连接任务当前状态
		 * \return 状态
		 */
		const zNewHTTPTask_State getState() const
		{
			return state;
		}

		/**
		 * \brief 设置连接任务状态
		 * \param state 需要设置的状态
		 */
		void setState(const zNewHTTPTask_State state)
		{
			this->state = state;
		}

		/**
		 * \brief 一个连接任务验证等步骤完成以后，需要添加到全局容器中
		 *
		 * 这个全局容器是外部容器
		 *
		 */
		virtual bool addToContainer() { return false; }

		/**
		 * \brief 连接任务退出的时候，需要从全局容器中删除
		 *
		 * 这个全局容器是外部容器
		 *
		 */
		virtual void removeFromContainer() {}
		
	protected:

		zSocket *pSocket;								/**< 底层套接口 */
		zNewHTTPTask_State state;							/**< 连接状态 */
		
		/**
		 * \brief  返回池内的连接数
		 *
		 */
		virtual const int getPoolSize();

		/**
		 * \brief  返回池最大连接数
		 *
		 */
		virtual const int getPoolMaxSize();
	private:

		zNewHttpTaskPool *pool;							/**< 任务所属的池 */
		zRTime lifeTime;								/**< 连接创建时间记录 */
		TerminateMethod terminate;						/**< 是否结束任务 */

};

#endif

