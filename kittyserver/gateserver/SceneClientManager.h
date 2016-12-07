/**
 * \file
 * \version  $Id: SceneClientManager.h 42 2013-04-10 07:33:59Z  $
 * \author  王海军, wanghaijun@ztgame.com 
 * \date 2006年01月04日 16时55分37秒 CST
 * \brief 网关到场景数据缓冲发送
 *
 * 
 */


#ifndef _SCENECLIENTMANAGER_H_
#define _SCENECLIENTMANAGER_H_

#include <map>
#include <set>

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "SceneClient.h"
#include "zTime.h"
#include "zRWLock.h"

/**
 ** \brief 定义服务器信息采集连接的客户端管理容器
 **/
class SceneClientManager : public Singleton<SceneClientManager>
{
	friend class Singleton<SceneClientManager>;
	public:

		~SceneClientManager();

		bool init();
		void timeAction(const zTime &ct);
		void add(SceneClient *sceneClient);
		void remove(SceneClient *sceneClient);
		bool broadcastOne(const void *pstrCmd, int nCmdLen);
		bool sendTo(const DWORD tempid, const void *pstrCmd, int nCmdLen);
		void setUsleepTime(int time)
		{
			sceneClientPool->setUsleepTime(time);
		}
		bool reConnectScene(const CMD::SUPER::ServerEntry *serverEntry);
		void setTaskReconnect(const std::string& ip, unsigned short port, bool reconn);
		SceneClient* getSceneByID(DWORD id);
        SceneClient* getMinScene();
	
		bool isAllStartOK();
        void final();
	private:

		SceneClientManager();
		static SceneClientManager *instance;

		/**
		 ** \brief 客户端连接管理池
		 **/
		zTCPClientTaskPool *sceneClientPool;
		/**
		 ** \brief 进行断线重连检测的时间记录
		 **/
		zTime actionTimer;

		/**
		 ** \brief 存放连接已经成功的连接容器类型
		 **/
		typedef std::map<const DWORD, SceneClient *> SceneClient_map;
		typedef SceneClient_map::iterator iter;
		typedef SceneClient_map::const_iterator const_iter;
		typedef SceneClient_map::value_type value_type;
		/**
		 ** \brief 存放连接已经成功的连接容器
		 **/
		SceneClient_map allClients;


		/**
		 ** \brief 容器访问读写锁
		 **/
		zRWLock rwlock;

};

#endif

