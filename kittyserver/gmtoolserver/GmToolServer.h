#ifndef GM_TOOL_SERVER_H
#define GM_TOOL_SERVER_H 

#include "Fir.h"
#include "zMisc.h"
#include "zMNetService.h"
#include "zTCPTaskPool.h"
#include "zDBConnPool.h"
#include "zMetaData.h"
#include "zNewHttpTaskPool.h"
#include "zCmdHandle.h"
#include "GmToolCommand.h"

/**
 * \brief 定义登陆服务类
 *
 * 登陆服务，负责登陆，建立帐号、档案等功能<br>
 * 这个类使用了Singleton设计模式，保证了一个进程中只有一个类的实例
 *
 */
class GmToolService : public Singleton<GmToolService>, public zMNetService
{

	public:

		/**
		 * \brief 获取连接池中的连接数
		 * \return 连接数
		 */
		const int getPoolSize() const
		{
			return gmTaskPool->getSize();
		}

		const int getMaxPoolSize() const
		{
			return gmTaskPool->getMaxConns();
		}

		/**
		 * \brief 获取服务器类型
		 * \return 服务器类型
		 */
		const WORD getType() const
		{
			return GMTOOLSERVER;
		}

		void reloadconfig();
		static zDBConnPool *dbConnPool;        //数据库连接池 
        static MetaData* metaData; 
        zCmdHandleManager m_cmdHandleManager;
    private:
        bool initConfig();
	private:
		friend class Singleton<GmToolService>;
		GmToolService();
		~GmToolService();

		bool init();
		void newTCPTask(const int sock, const unsigned short srcPort);
		void final();

		unsigned short login_port;
		unsigned short inside_port;
		unsigned short php_port;
        
		zTCPTaskPool* gmTaskPool;
		zTCPTaskPool* serverTaskPool;
		zTCPTaskPool* phpTaskPool;
};
#endif

