/**
 * \file
 * \version  $Id$
 * \author  吴志勇,wuzhiyong@ztgame.com 
 * \date 定义实现轻量级(lightweight)的http服务框架(可以保持连接)
 * \brief 
 *
 */


#ifndef _zNewHttpTaskPool_h_
#define _zNewHttpTaskPool_h_

#include <string>
#include <vector>
#include <queue>
#include <list>
#include <unistd.h>
#include <sys/timeb.h>

#include "zSocket.h"
#include "zThread.h"
#include "zNewHttpTask.h"
#include "Fir.h"
#include "zString.h"

/**
 * \brief 定义实现轻量级(lightweight)的http服务框架类
 */
class zNewHttpTaskPool : private zNoncopyable
{

	public:

		/**
		 * \brief 构造函数
		 */
		zNewHttpTaskPool(const int maxHttpConns) : maxHttpConns(maxHttpConns)
		{
		};

		/**
		 * \brief 析构函数，销毁一个线程池对象
		 *
		 */
		~zNewHttpTaskPool()
		{
			final();
		}

		bool addHttp(zNewHttpTask *task);
		const int getSize();
		inline const int getMaxConns() const { return maxHttpConns; }
		bool init();
		void final();

	private:

		static const int maxHttpThreads = 8;					/**< 最大验证线程数量 */
		zThreadGroup httpThreads;								/**< http服务处理线程组 */
		const int maxHttpConns;								/**< 线程池并行处理连接的最大数量 */
		
};

#endif
