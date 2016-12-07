/**
 * \file
 * \version  $Id: FLClientManager.h 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2005年04月01日 15时09分53秒 CST
 * \brief 定义统一用户平台客户端连接的管理容器
 */


#ifndef _FLClientManager_h_
#define _FLClientManager_h_

#include <unordered_map>

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "FLClient.h"
#include "zTime.h"
#include "zRWLock.h"

/**
 * \brief 统一用户平台登陆服务器的客户端连接类管理器
 */
class FLClientManager : public Singleton<FLClientManager>
{
	friend class Singleton<FLClientManager>;
	public:

		~FLClientManager();

		bool init();
		bool reload();
		void resetState();
		void timeAction(const zTime &ct);
		void add(FLClient *flClient);
		void remove(FLClient *flClient);
		void broadcast(const void *pstrCmd, int nCmdLen);
		void sendTo(const WORD tempid, const void *pstrCmd, int nCmdLen);
			
		void initRegRoleCount();
		void notifyRegRoleCount();
        void final();
        
	private:

		FLClientManager();
		static FLClientManager *instance;
		
		/**
		 * \brief 客户端连接管理池
		 */
		zTCPClientTaskPool *flClientPool;
		/**
		 * \brief 进行断线重连检测的时间记录
		 */
		zTime actionTimer;

		/**
		 * \brief 存放连接已经成功的连接容器类型
		 */
		typedef std::unordered_map<WORD, FLClient *> FLClientContainer;
		typedef FLClientContainer::iterator iter;
		typedef FLClientContainer::const_iterator const_iter;
		typedef FLClientContainer::value_type value_type;
		/**
		 * \brief 存放连接已经成功的连接容器
		 */
		FLClientContainer allClients;
		/**
		 * \brief 容器访问读写锁
		 */
		zRWLock rwlock;

};

#endif

