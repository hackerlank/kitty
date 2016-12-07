#ifndef RESOURCE_CLENT_MANAGER_H
#define RESOURCE_CLENT_MANAGER_H 

#include <unordered_map>
#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "ResourceClient.h"
#include "zTime.h"
#include "zRWLock.h"

/**
 * \brief 统一用户平台登陆服务器的客户端连接类管理器
 */
class ResourceClientManager : public Singleton<ResourceClientManager>
{
	friend class Singleton<ResourceClientManager>;
	public:

		~ResourceClientManager();

		bool init();
		bool reload();
		void resetState();
		void timeAction(const zTime &ct);
		void add(ResourceClient *resourceClient);
		void remove(ResourceClient *resourceClient);
		void broadcast(const void *pstrCmd, int nCmdLen);
		void sendTo(const WORD tempid, const void *pstrCmd, int nCmdLen);
			
		void initRegRoleCount();
		void notifyRegRoleCount();
        void final();
        
	private:

		ResourceClientManager();
		static ResourceClientManager *instance;
		
		/**
		 * \brief 客户端连接管理池
		 */
		zTCPClientTaskPool *resourceClientPool;
		/**
		 * \brief 进行断线重连检测的时间记录
		 */
		zTime actionTimer;

		/**
		 * \brief 存放连接已经成功的连接容器类型
		 */
		typedef std::unordered_map<WORD, ResourceClient*> ResourceClientContainer;
		typedef ResourceClientContainer::iterator iter;
		typedef ResourceClientContainer::const_iterator const_iter;
		typedef ResourceClientContainer::value_type value_type;
		/**
		 * \brief 存放连接已经成功的连接容器
		 */
		ResourceClientContainer allClients;
		/**
		 * \brief 容器访问读写锁
		 */
		zRWLock rwlock;

};

#endif

