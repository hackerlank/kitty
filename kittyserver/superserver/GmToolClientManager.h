#ifndef GM_TOOL_CLENT_MANAGER_H
#define GM_TOOL_CLENT_MANAGER_H 

#include <unordered_map>
#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "GmToolClient.h"
#include "zTime.h"
#include "zRWLock.h"

/**
 * \brief 统一用户平台登陆服务器的客户端连接类管理器
 */
class GmToolClientManager : public Singleton<GmToolClientManager>
{
	friend class Singleton<GmToolClientManager>;
	public:

		~GmToolClientManager();

		bool init();
		bool reload();
		void resetState();
		void timeAction(const zTime &ct);
		void add(GmToolClient *gmToolClient);
		void remove(GmToolClient *gmToolClient);
		void broadcast(const void *pstrCmd, int nCmdLen);
		void sendTo(const WORD tempid, const void *pstrCmd, int nCmdLen);
			
		void initRegRoleCount();
		void notifyRegRoleCount();
        void final();
        
	private:

		GmToolClientManager();
		static GmToolClientManager*instance;
		
		/**
		 * \brief 客户端连接管理池
		 */
		zTCPClientTaskPool *gmToolClientPool;
		/**
		 * \brief 进行断线重连检测的时间记录
		 */
		zTime actionTimer;

		/**
		 * \brief 存放连接已经成功的连接容器类型
		 */
		typedef std::unordered_map<WORD, GmToolClient*> GmToolClientContainer;
		typedef GmToolClientContainer::iterator iter;
		typedef GmToolClientContainer::const_iterator const_iter;
		typedef GmToolClientContainer::value_type value_type;
		/**
		 * \brief 存放连接已经成功的连接容器
		 */
		GmToolClientContainer allClients;
		/**
		 * \brief 容器访问读写锁
		 */
		zRWLock rwlock;

};

#endif

