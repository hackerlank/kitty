/**
 * \file
 * \version  $Id: SuperServer.h 75 2013-04-25 05:10:39Z  $
 * \author  ,@163.com
 * \date 2004年11月29日 17时19分12秒 CST
 * \brief 实现服务器管理器
 *
 * 对一个区中的所有服务器进行管理
 * 
 */

#ifndef _SuperServer_h_
#define _SuperServer_h_

#include <iostream>
#include <string>
#include <ext/numeric>

#include "zService.h"
#include "zThread.h"
#include "zSocket.h"
#include "zTCPServer.h"
#include "zMNetService.h"
#include "ServerTask.h"
#include "zDBConnPool.h"
#include "zMisc.h"
#include "zMemDBPool.h"
#include "zTCPTaskPool.h"
#include "zNewHttpTaskPool.h"

class MetaData;
/**
 * \brief 服务器管理器类
 *
 * 派生了基类<code>zNetService</code>
 *
 */
class SuperService : public zMNetService, public Singleton<SuperService>
{
	friend class Singleton<SuperService>;
	public:

		/**
		 * \brief 析构函数
		 *
		 * 虚函数
		 *
		 */
		~SuperService()
		{
			instance = NULL;

			//关闭线程池
			if (taskPool)
			{
				taskPool->final();
				SAFE_DELETE(taskPool);
			}
		}

#ifdef _OP_TOOL_VERSION
		//运维属性察看工具
		bool reSetDBPool(std::string str);//重置数据库池
#endif
		const int getPoolSize() const
		{
			if (taskPool)
				return taskPool->getSize();
			else
				return 0;
		}

		void reloadconfig();

		/**
		 * \brief 获取游戏区编号
		 * \return 返回游戏区编号
		 */
		const GameZone_t &getZoneID() const
		{
			return gameZone;
		}

		/**
		 * \brief 设置游戏区编号
		 * \param gameZone 游戏区编号
		 */
		void setZoneID(const GameZone_t &gameZone)
		{
			this->gameZone = gameZone;
		}

		/**
		 * \brief 获取游戏区名称
		 * \return 返回游戏区名称
		 */
		const std::string &getZoneName() const
		{
			return zoneName;
		}

		/**
		 * \brief 设置游戏区名称
		 * \param zoneName 待设置的名称
		 */
		void setZoneName(const char *zoneName)
		{
			this->zoneName = zoneName;
		}

		/**
		 * \brief 获取服务器编号
		 * \return 服务器编号
		 */
		const WORD getID() const
		{
			return wdServerID;
		}

		/**
		 * \brief 获取服务器类型
		 * \return 服务器类型
		 */
		const WORD getType() const
		{
			return wdServerType;
		}

		/**
		 * \brief 获取ip地址
		 * \return 返回ip地址
		 */
		const char *getIP() const
		{
			return pstrIP;
		}

		/**
		 * \brief 获取端口
		 * \return 返回端口
		 */
		const WORD getPort() const
		{
			return wdPort;
		}

		/**
		 * \brief 获取网络类型
		 * \return 返回网络类型
		 */
		const WORD getNetType() const
		{
			return wdNetType;
		}

		/**
		 * \brief 指向数据库连接池实例的指针
		 *
		 */
		static zDBConnPool *dbConnPool;
		/**
		 * \brief 指向数据库表管理器的指针
		 *
		 */
		static MetaData* metaData;
		/**
		 *\brief 是否要转发gm工具指令给平台
		 *\param command 要转发的指令
		 *\return  true为可以转发，false为不可以转发
		 **/
		bool isTranferCmd(std::string command)
		{
			std::map<std::string,QWORD>::iterator gmtoolcommandsmap_it;
			typedef std::map<std::string,QWORD>::value_type gmtoolcommands_pair;
			gmtoolcommandsmap_it=gmtoolcommandsmap.find(command);
			zRTime now;
			if(gmtoolcommandsmap_it!=gmtoolcommandsmap.end())
			{//找到消息
				if((now.sec()-30)>gmtoolcommandsmap_it->second)
				{//转发两条指令间隔时间是否超过30s,超过则允许转发
					gmtoolcommandsmap.erase(command);
					gmtoolcommands_pair pair(command,now.sec());
					gmtoolcommandsmap.insert(pair);
					return true;
				}
				else
					return false;
			}
			else
			{//未找到可以转发
				gmtoolcommands_pair pair(command,now.sec());
				gmtoolcommandsmap.insert(pair);
				return true;
			}
		}         
	private:

		//记录执行过的gm工具指令
		std::map<std::string,QWORD> gmtoolcommandsmap;
		/**
		 * \brief 游戏区编号
		 */
		GameZone_t gameZone;
		/**
		 * \brief 游戏区名称
		 */
		std::string zoneName;

		WORD wdServerID;					/**< 服务器编号，一个区唯一的 */
		WORD wdServerType;					/**< 服务器类型，创建类实例的时候已经确定 */
		char pstrName[MAX_NAMESIZE];		/**< 服务器名称 */
		char pstrIP[MAX_IP_LENGTH];			/**< 服务器内网地址 */
		WORD wdPort;						/**< 服务器内网端口，也就是邦定端口 */
		char pstrExtIP[MAX_IP_LENGTH];		/**< 服务器外网地址，也就是防火墙地址 */
		WORD wdExtPort;						/**< 服务器外网端口，也就是映射到防火墙的端口 */
		WORD wdNetType;						/**< 网络类型 */

		WORD http_port;

		/**
		 * \brief 类的唯一实例指针
		 *
		 */
		static SuperService *instance;

		zTCPTaskPool *taskPool;				/**< TCP连接池的指针 */
		/**
		 * \brief 构造函数
		 *
		 */
		SuperService() : zMNetService("服务器管理器"),gameZone()
	{
		wdServerID = 1;
		wdServerType = SUPERSERVER;
		bzero(pstrName, sizeof(pstrName));
		bzero(pstrIP, sizeof(pstrIP));
		wdPort = 0;
		bzero(pstrExtIP, sizeof(pstrExtIP));
		wdExtPort = 0;
		wdNetType = 0;
		taskPool = NULL;
		serverSequence.clear();
	}

		bool init();

		void newTCPTask(const int sock, const unsigned short srcPort);
		//void newTCPTask(const int sock, const struct sockaddr_in *addr);
		void final();

		bool getServerInfo();

		void initLogger();

		void initServerSequence();

		std::unordered_map<int, std::vector<int> > serverSequence;
	public:
		BYTE yuyinFlag;		
		static zLogger* bill_logger;		//充值日志
		static zLogger* rechargelogger;
		std::vector<int> getServerSes(int wdServerType);
};


#endif

