/**
 * \file
 * \version  $Id: FLServer.h 2877 2005-09-12 12:16:19Z whj $
 * \author  ,okyhc@263.sina.com
 * \date 2004年12月10日 10时55分53秒 CST
 * \brief zebra项目登陆服务器，负责登陆，建立帐号、档案等功能
 *
 */

#ifndef _FLServer_h_
#define _FLServer_h_

#include "Fir.h"
#include "zMisc.h"
#include "zMNetService.h"
#include "zTCPTaskPool.h"
#include "zDBConnPool.h"
#include "zMetaData.h"
#include "zNewHttpTaskPool.h"
#include "zCmdHandle.h"
#include "zHttpClientPool.h"

/**
 * \brief 定义登陆服务类
 *
 * 登陆服务，负责登陆，建立帐号、档案等功能<br>
 * 这个类使用了Singleton设计模式，保证了一个进程中只有一个类的实例
 *
 */
class FLService : public Singleton<FLService>, public zMNetService
{

	public:

		/**
		 * \brief 获取连接池中的连接数
		 * \return 连接数
		 */
		const int getPoolSize() const
		{
			return loginTaskPool->getSize();
		}

		const int getMaxPoolSize() const
		{
			return loginTaskPool->getMaxConns();
		}

		/**
		 * \brief 获取服务器类型
		 * \return 服务器类型
		 */
		const WORD getType() const
		{
			return LOGINSERVER;
		}

		void reloadconfig();

		static zDBConnPool *dbConnPool;        //数据库连接池 

        // @brief brief 消息解析者管理器
        zCmdHandleManager cmd_handle_manager;
        zHttpClientPool *gethttppool(){return httptaskPool;}

	private:

		friend class Singleton<FLService>;
		FLService();
		~FLService();

		bool init();
		void newTCPTask(const int sock, const unsigned short srcPort);
		void final();

		unsigned short login_port;
		unsigned short inside_port;
		unsigned short php_port;

		zTCPTaskPool* loginTaskPool;
		zTCPTaskPool* serverTaskPool;
		zTCPTaskPool* phpTaskPool;
        zHttpClientPool * httptaskPool;

};

class TimeTick : public Singleton<TimeTick>, public zThread
{
public:
	void run()
	{
		while(!isFinal())
		{
			zThread::sleep(60);
			if(ifNewHour())
			{
				Fir::logger->info("切日志");
			}
		}
	}

private:
	friend class Singleton<TimeTick>;
	TimeTick() : zThread("TimeTick") {};
	~TimeTick() {};

	bool ifNewHour()
	{
		std::string str;
		zTime zt;

		DWORD iMin = zt.getMin();
		//Fir::logger->debug("TimeTick: iMin=%d", iMin);

		if(iMin < 5)
			return true;

		return false;
	}
};

#endif

