/**
 * \file
 * \version  $Id: zHttpTaskPool.cpp 13 2013-03-20 02:35:18Z  $
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
#include "zHttpTaskPool.h"
#include "Fir.h"
#include "zTime.h"

/**
 * \brief 轻量级http服务的主处理线程
 */
class zHttpThread : public zThread
{

	private:

		/**
		 * \brief http连接任务链表类型
		 */
//		typedef std::list<zHttpTask *, __gnu_cxx::__pool_alloc<zHttpTask *> > zHttpTaskContainer;
		typedef std::list<zHttpTask * > zHttpTaskContainer;

		/**
		 * \brief epoll事件结构向量类型
		 */
		typedef std::vector<struct epoll_event> epollfdContainer;

		zHttpTaskPool *pool;		/**< 所属的池 */
		zRTime currentTime;			/**< 当前时间 */
		zMutex mutex;				/**< 互斥变量 */
		zHttpTaskContainer tasks;	/**< 任务列表 */

		int kdpfd;
		epollfdContainer epfds;
		epollfdContainer::size_type fds_count;

	public:

		/**
		 * \brief 构造函数
		 * \param pool 所属的连接池
		 * \param name 线程名称
		 */
		zHttpThread(
				zHttpTaskPool *pool,
				const std::string &name = std::string("zHttpThread"))
			: zThread(name), pool(pool), currentTime()
			{
				kdpfd = epoll_create(256);
				assert(-1 != kdpfd);
				epfds.resize(256);
				fds_count = 0;
			}

		/**
		 * \brief 析构函数
		 */
		~zHttpThread()
		{
			TEMP_FAILURE_RETRY(::close(kdpfd));
		}

		void run();

		/**
		 * \brief 添加一个连接任务
		 * \param task 连接任务
		 */
		void add(zHttpTask *task)
		{
			mutex.lock();
			task->addEpoll(kdpfd, EPOLLIN | EPOLLERR | EPOLLPRI, (void *)task);
			tasks.push_back(task);
			++fds_count;
			if (fds_count > epfds.size())
			{
				epfds.resize(fds_count + 16);
			}
			mutex.unlock();
		}

		typedef zHttpTask* zHttpTaskP;
		void remove(zHttpTaskP &task)
		{
			task->delEpoll(kdpfd, EPOLLIN | EPOLLERR | EPOLLPRI);
			tasks.remove(task);
			SAFE_DELETE(task);
			fds_count--;
		}
		void remove(zHttpTaskContainer::iterator &it)
		{
			zHttpTask *task = *it;
			task->delEpoll(kdpfd, EPOLLIN | EPOLLERR | EPOLLPRI);
			tasks.erase(it);
			SAFE_DELETE(task);
			fds_count--;
		}

};

/**
 * \brief 线程主回调函数
 */
void zHttpThread::run()
{
	zHttpTaskContainer::iterator it, next;

	while(!isFinal())
	{
		mutex.lock();
		if (!tasks.empty())
		{
			int retcode = epoll_wait(kdpfd, &epfds[0], fds_count, 0);
			if (retcode > 0)
			{
				for(int i = 0; i < retcode; ++i)
				{
					zHttpTask *task = (zHttpTask *)epfds[i].data.ptr;
					if (epfds[i].events & (EPOLLERR | EPOLLPRI))
					{
						//套接口出现错误
						remove(task);
					}
					else if (epfds[i].events & EPOLLIN)
					{
						switch(task->httpCore())
						{
							case 1:		//接收成功
							case -1:	//接收失败
								remove(task);
								break;
							case 0:		//接收超时，
								break;
						}
					}
				}
			}

			currentTime.now();
			for(it = tasks.begin(), next = it, ++next; it != tasks.end(); it = next, ++next)
			{
				zHttpTask *task = *it;
				if (task->checkHttpTimeout(currentTime))
				{
					//超过指定时间验证还没有通过，需要回收连接
					remove(it);
				}
			}
		}
		mutex.unlock();

		zThread::msleep(50);
	}

	//把所有等待验证队列中的连接加入到回收队列中，回收这些连接
	for(it = tasks.begin(), next = it, ++next; it != tasks.end(); it = next, ++next)
	{
		remove(it);
	}
}

/**
 * \brief 把一个TCP连接添加到验证队列中，因为存在多个验证队列，需要按照一定的算法添加到不同的验证处理队列中
 * \param task 一个连接任务
 */
bool zHttpTaskPool::addHttp(zHttpTask *task)
{
	//因为存在多个验证队列，需要按照一定的算法添加到不同的验证处理队列中
	static unsigned int hashcode = 0;
	zHttpThread *pHttpThread = (zHttpThread *)httpThreads.getByIndex(hashcode++ % maxHttpThreads);
	if (pHttpThread)
		pHttpThread->add(task);
	return true;
}

/**
 * \brief 初始化线程池，预先创建各种线程
 * \return 初始化是否成功
 */
bool zHttpTaskPool::init()
{
	//创建初始化验证线程
	for(int i = 0; i < maxHttpThreads; ++i)
	{
		std::ostringstream name;
		name << "zHttpThread[" << i << "]";
		zHttpThread *pHttpThread = FIR_NEW zHttpThread(this, name.str());
		if (NULL == pHttpThread)
			return false;
		if (!pHttpThread->start())
			return false;
		httpThreads.add(pHttpThread);
	}

	return true;
}

/**
 * \brief 释放线程池，释放各种资源，等待各种线程退出
 */
void zHttpTaskPool::final()
{
	httpThreads.joinAll();
}

