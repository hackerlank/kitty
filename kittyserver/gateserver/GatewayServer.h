/**
 * \file
 * \version  $Id: GatewayServer.h 36 2013-04-07 11:42:48Z  $
 * \author  ,okyhc@263.sina.com
 * \date 2004年12月10日 10时55分53秒 CST
 * \brief zebra项目Gateway服务器,负责用户指令检查转发、加密解密等
 */

#ifndef _GatewayServer_h_
#define _GatewayServer_h_
#include <vector>
#include <string>

#include "zSubNetService.h"
#include "Fir.h"
#include "zMisc.h"
#include "SuperCommand.h"
#include "LoginUserCommand.h"
#include "zMemDBPool.h"
#include "zCmdHandle.h"
#include "FLCommand.h"

/**
 * \brief 定义网关服务类
 *
 * 这个类使用了Singleton设计模式，保证了一个进程中只有一个类的实例
 *
 */
class GatewayService : public zSubNetService, public Singleton<GatewayService>
{
	friend class Singleton<GatewayService>;

    public:

		bool msgParse_SuperService(const CMD::t_NullCmd *ptNullCmd, const unsigned int nCmdLen);

		const int getPoolSize() const
		{
			if(taskPool)
			{
			return taskPool->getSize();
			}
			else
			{
				return 0;
			}
		}

		const int getPoolState() const
		{
			return taskPool->getState();
		}

		bool notifyLoginServer();


		/**
		 * \brief 虚析构函数
		 *
		 */
		~GatewayService()
		{
			
		}

		void reloadconfig();
		bool isSequeueTerminate() 
		{
			return taskPool == NULL;
		}

		static void loadNewZoneLimit(); //新区3天5天限制
		void initGatewayconfig();

    public:
        
        /**
		 * \brief 校验客户端版本号
		 */
		float verify_client_version;


        static std::map<DWORD, time_t>  questLimit;
		zCmdHandleManager cmd_handle_manager;

		//推广员_logger
		static zLogger* promoter_logger;

		static zLogger* bill_logger;		//支付日志

		static zLogger* login_logger; // 登录日志

		volatile unsigned int limit_level;	//登陆等级限制

		GameZone_t zoneID;


		static bool service_gold;
		static bool service_stock;
		DWORD  acu_baseexp;
		DWORD  acu_switch;

		/**
		* \brief 是否为新世界
		*
		*/
		bool newworld;
    
    private:
       
        /**
		 * \brief 构造函数
		 *
		 */
		GatewayService() : zSubNetService("网关服务器", GATEWAYSERVER)
		{
			limit_level = 0;
			taskPool = NULL;
			newworld = false;
		}

		bool init();
		void newTCPTask(const int sock, const struct sockaddr_in *addr);
		void final();

		/**
		 * \brief 确认服务器初始化成功，即将进入主回调函数
		 *
		 * 向服务器发送t_Startup_OK指令来确认服务器启动成功
		 * 并且通知所有登陆服务器，这台网关服务器准备好了
		 *
		 * \return 确认是否成功
		 */
		virtual bool validate()
		{
			zSubNetService::validate();

			return notifyLoginServer();
		}

		bool initLogger();
		bool initLoginLogger();
        virtual void getnewServerEntry(const CMD::SUPER::ServerEntry &entry);

    private:
        //处理superCmd消息
        bool msgParseSuperCmd(const CMD::SUPER::SuperServerNull *superNull,const DWORD nCmdLen);
        //处理flCmd消息
        bool msgParseFlCmd(const CMD::FL::FLNullCmd *flNull,const DWORD nCmdLen);
    private:

		/**
		 * \brief 类的唯一实例指针
		 *
		 */
		static GatewayService *instance;

		static zTCPTaskPool *taskPool;				/**< TCP连接池的指针 */


};

#endif

