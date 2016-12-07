/**
 * \file
 * \version  $Id: ServerManager.h 70 2013-04-23 13:26:44Z  $
 * \author  ,@163.com
 * \date 2004年12月13日 18时44分39秒 CST
 * \brief 定义服务器管理容器
 *
 * 这个容器包括全局容器和唯一性验证容器
 * 
 */


#ifndef _ServerManager_h_
#define _ServerManager_h_

#include <iostream>
#include <list>
#include <unordered_map>

#include "ServerTask.h"
#include "zMutex.h"
#include "zNoncopyable.h"
#include "zRWLock.h"

/**
 * \brief 服务器管理容器类
 *
 * 这个容器包括全局容器和唯一性验证容器
 *
 */
class ServerManager : public Singleton<ServerManager>
{
	friend class Singleton<ServerManager>;
	public:

		/**
		 * \brief 缺省析构函数
		 *
		 */
		~ServerManager() {};

		void addServer(ServerTask *task);
		void removeServer(ServerTask *task);
		ServerTask* getServer(WORD wdServerID);
		ServerTask* getServerByType(const WORD wdType);
		bool uniqueAdd(ServerTask *task);
		bool uniqueVerify(const WORD wdServerID);
		bool uniqueRemove(ServerTask *task);
		bool broadcast(const void *pstrCmd, int nCmdLen);
		bool broadcastIP(const void *pstrcmd, int nCmdLen);
		bool broadcastByID(const WORD wdServerID, const void *pstrCmd, int nCmdLen);
		bool broadcastByType(const WORD wdType, const void *pstrCmd, int nCmdLen);
		bool broadcastByOne(const WORD wdType, const void *pstrCmd, int nCmdLen);
		const DWORD caculateOnlineNum();
		void responseOther(const WORD srcID, const WORD wdServerID);

		void checkDeadLock(DWORD now);

		void notifyGatewayReconnectScene(ServerTask *task);
		void addSceneInfo(WORD id, const std::string& ip);
        void sendOtherSceneClose(ServerTask *task);
        void sendOtherMeStart(ServerTask *task);
        void sendOtherserverToMe(ServerTask *task,const std::set<WORD>& settype);
		void execEvery();
        ServerTask * randServerByType(WORD wdType);
	private:

		/**
		 * \brief 定义服务器容器类型
		 *
		 */
		typedef std::list<ServerTask *> Container;
		/**
		 * \brief 定义服务器容器类型的迭代器
		 *
		 */
		typedef Container::iterator Container_iterator;
		/**
		 * \brief 定义服务器容器类型的常量迭代器
		 *
		 */
		typedef Container::const_iterator Containter_const_iterator;
		/**
		 * \brief 定义了服务器的唯一性验证容器类型
		 * 
		 **/
		typedef std::unordered_map<WORD, ServerTask *> ServerTaskHashmap;
		/**
		 * \brief 定义容器的迭代器类型
		 *
		 */
		typedef ServerTaskHashmap::iterator ServerTaskHashmap_iterator;
		/**
		 * \brief 定义了容器的常量迭代器类型
		 *
		 */
		typedef ServerTaskHashmap::const_iterator ServerTaskHashmap_const_iterator;
		/**
		 * \brief 定义了容器的键值对类型
		 *
		 */
		typedef ServerTaskHashmap::value_type ServerTaskHashmap_pair;
		/**
		 * \brief 容器访问的互斥变量
		 *
		 */
		zMutex mutex;
		/**
		 * \brief 服务器全局容器的实例
		 *
		 */
		Container container;
		/**
		 * \brief 唯一性容器实例
		 *
		 */
		ServerTaskHashmap taskUniqueContainer;

		/**
		 * \brief 保存连接成功的场景<ID,IP> 类型容器
		 *
		 */
		typedef std::unordered_map<WORD, std::string> SceneInfoHashmap;
		SceneInfoHashmap sceneinfo;


		/**
		 * \brief 定义迭代器类型
		 */
		typedef SceneInfoHashmap::iterator SceneInfoHashmap_iterator;

		/**
		 * \brief 定义窗口的键值对类型
		 */
		typedef SceneInfoHashmap::value_type SceneInfohashmap_pair;


		/**
		 * \brief 类的唯一实例指针
		 *
		 */
		static ServerManager *instance;

		/**
		 * \brief 构造函数
		 *
		 */
		ServerManager() {};

		zRWLock rwlock;

};

#endif

