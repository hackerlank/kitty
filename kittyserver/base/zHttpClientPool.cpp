/**
 * @file zHttpClientPool.cpp
 * $Id$
 * @brief	实现http client连接池
 * @author 
 * @date 2013-07-22
 */

#include <vector>
#include <queue>
#include <set>
#include "zHttpClientPool.h"
#include "zHttpClient.h"
#include "zThread.h"
#include "zTime.h"


/**
 * \brief http链接池
 */
class zHttpClientThread : public zThread
{
	private:
		zHttpClientPool *pool;		/**< 所属的池 */
		zRTime currentTime;			/**< 当前时间 */
		zMutex mutex;				/**< 互斥变量 */
		zHttpClient client;		/**< http工作客户端 */
		std::queue<zHttpReq*> tasks;
		std::string ret_data; //请求后返回的数据，用于回调
	
	public:
		std::string ret;

		/**
		 * \brief 构造函数
		 * \param pool 所属的连接池
		 * \param name 线程名称
		 */
		zHttpClientThread(
				zHttpClientPool *pool,
				const std::string &name = std::string("zHttpClientThread"))
			: zThread(name), pool(pool), currentTime()
			{
				client.init_curl();
			}

		/**
		 * \brief 析构函数
		 */
		~zHttpClientThread()
		{
		}

		void run();

		/**
		 * \brief 添加一个连接任务
		 * \param task 连接任务
		 * \return 添加是否成功
		 */
		bool add(zHttpReq *task)
		{
			bool retval = false;

			mutex.lock();
			tasks.push(task);
			retval = true;
			mutex.unlock();

			return retval;
		}

		void remove()
		{
		}

		DWORD size()
		{
			return  tasks.size();
		}
};

/**
 * \brief 线程主回调函数
 */
void zHttpClientThread::run()
{
	while(!isFinal())
	{
		if (tasks.size()>=10)
		{
			DWORD process_num = tasks.size()/5;
			if (process_num==0) continue;

			std::set<zHttpReq*> req_list;
			for (DWORD i=0; i<process_num; i++)
			{
				if (!tasks.empty())
				{
					req_list.insert(tasks.front());
					tasks.pop();
				}
			}

			for (auto it=req_list.begin(); it!=req_list.end(); it++)
			{
				zHttpReq* h = (*it);
				if (!h) continue;

				client.init_requrl(h->url);

				if (h->getType() == HttpReqType_Get)
				{
					if (h->callback)
					{
						if (client.httpGet(ret_data, h->postdata))
						{
							h->callback(h->gatewayid, h->tempid, ret_data);
						}
					}
					else
					{
						if (client.httpGetNoReturn(ret_data, h->postdata))
						{
#ifdef _DEBUG_WCX
							Fir::logger->debug("发送成功");
#endif
						}
						else
						{
#ifdef _DEBUG_WCX
							Fir::logger->debug("发送失败");
#endif
						}
					}
				}
				else if (h->getType() == HttpReqType_Post)
				{
					if (h->callback)
					{
						if (client.httpPost(ret_data, h->postdata))
						{
							h->callback(h->gatewayid, h->tempid, ret_data);
						}
					}
					else
					{
						if (client.httpPostNoReturn(ret_data, h->postdata))
						{                                                  
#ifdef _DEBUG_WCX                                                          
							Fir::logger->debug("发送成功");                
#endif                                                                     
						}                                                  
						else                                               
						{                                                  
#ifdef _DEBUG_WCX                                                          
							Fir::logger->debug("发送失败");                
#endif                                                                     
						}                                                  
					}
				}

				SAFE_DELETE(h);
			}
		}
		else
		{
			if (tasks.size()>0)
			{
				mutex.lock();
				zHttpReq* h = tasks.front();
				tasks.pop();
				mutex.unlock();

				if (h)
				{

					client.init_requrl(h->url);

					if (h->getType() == HttpReqType_Get)
					{
						if (h->callback)
						{
							if (client.httpGet(ret_data, h->postdata))
							{
								h->callback(h->gatewayid, h->tempid, ret_data);
							}
						}
						else
						{
							if (client.httpGetNoReturn(ret_data, h->postdata))
							{
#ifdef _DEBUG_WCX
								Fir::logger->debug("发送成功");
#endif
							}
							else
							{
#ifdef _DEBUG_WCX
								Fir::logger->debug("发送失败");
#endif
							}
						}
					}
					else if (h->getType() == HttpReqType_Post)
					{
						if (h->callback)
						{
							if (client.httpPost(ret_data, h->postdata))
							{
								h->callback(h->gatewayid, h->tempid, ret_data);
							}
						}
						else
						{
							if (client.httpPostNoReturn(ret_data, h->postdata))
							{                                                  
#ifdef _DEBUG_WCX                                                          
								Fir::logger->debug("发送成功");                
#endif                                                                     
							}                                                  
							else                                               
							{                                                  
#ifdef _DEBUG_WCX                                                          
								Fir::logger->debug("发送失败");                
#endif                                                                     
							}                                                  
						}
					}

					SAFE_DELETE(h);
				}
			}
		}

		zThread::msleep(50);
	}

	//把所有等待验证队列中的连接加入到回收队列中，回收这些连接
	remove();
}

/**
 * \brief 把一个TCP连接添加到验证队列中，因为存在多个验证队列，需要按照一定的算法添加到不同的验证处理队列中
 * \param task 一个连接任务
 */
bool zHttpClientPool::addHttp(zHttpReq* task)
{
	//因为存在多个线程，需要按照一定的算法添加到不同的线程中
	static unsigned int hashcode = 0;
	bool retval = false;
	zHttpClientThread *p = (zHttpClientThread *)httpThreads.getByIndex(hashcode++ % maxHttpThreads);
	if (p)
		retval = p->add(task);
	return retval;
}

/**
 * \brief 初始化线程池，预先创建各种线程
 * \return 初始化是否成功
 */
bool zHttpClientPool::init()
{
	CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
	if ( res != CURLE_OK)
	{
		Fir::logger->error("curl_easy_init failed:%d", res);
		return false;
	}
	//创建初始化验证线程
	for(int i = 0; i < maxHttpThreads; i++)
	{
		std::ostringstream name;
		name << "zHttpClientThread[" << i << "]";
		zHttpClientThread *p = new zHttpClientThread(this, name.str());
		if (NULL == p)
			return false;
		if (!p->start())
			return false;
		httpThreads.add(p);
	}

	return true;
}

/**
 * \brief 返回连接池中子连接个数
 *
 */
const int zHttpClientPool::getSize()
{
	struct MyCallback : zThreadGroup::Callback
	{
		int size;
		MyCallback() : size(0) {}
		void exec(zThread *e)
		{
			zHttpClientThread *p = (zHttpClientThread *)e;
			size += p->size();
		}
	};

	MyCallback mcb;
	httpThreads.execAll(mcb);
	return mcb.size;
}

/**
 * \brief 释放线程池，释放各种资源，等待各种线程退出
 */
void zHttpClientPool::final()
{
	curl_global_cleanup();

	httpThreads.joinAll();
}

