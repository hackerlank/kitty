/**
 * \file
 * \version  $Id: GateUserManager.h 53 2013-04-17 05:03:41Z  $
 * \author  ,
 * \date 2013年04月11日 09时55分24秒 CST
 * \brief 定义网关用户管理类
 */

#ifndef _GATEUSERMANAGER_H_
#define _GATEUSERMANAGER_H_
#include "GateUser.h"
#include "zUniqueID.h"
#include <unordered_map>
#include <functional>

class GateUserManager : public Singleton<GateUserManager>
{
	friend class Singleton<GateUserManager>;
	private:
		/**
		 * \brief 用户索引读写锁
		 */
		zRWLock charid_lock;

		GateUserManager();
		~GateUserManager();
	public:
		// accid map
		GateUser * getUserAccid(DWORD accid);
		void removeUserAccid(DWORD accid);
		bool addUserAccid(GateUser *user);

		void removeUserBySceneClient(SceneClient *scene);
		void removeAllUser();

		// charid map
		GateUser * getUserCharid(QWORD charid);
		bool addUserCharid(GateUser *user);
		void removeUserCharid(QWORD charid);

		DWORD getRoleCount();

		//plat_account map
		GateUser * getUserAccount(DWORD platform, const std::string& account);
		bool addUserAccount(GateUser* user);
		void removeUserAccount(DWORD platform, const std::string& account);

		typedef std::function<void (GateUser*)> UserFunction;
		void execAll(UserFunction func)
		{
			charid_lock.rdlock();

			for(auto itr=m_mapUsers.begin();itr!=m_mapUsers.end();++itr)
			{
				func(itr->second);
			}

			charid_lock.unlock();
		}
        void sendCmd(const void *pstrCmd, const DWORD nCmdLen);
		void sendCmdToWorld(const void *pstrCmd, const unsigned int nCmdLen,bool worldChat = false);
        void sendChatToworld(const void *pstrCmd, const unsigned int nCmdLen, BYTE lang);
        void sendChatToCity(DWORD cityid,const void *pstrCmd,const unsigned int nCmdLen);
        //给客户端同步时间
        bool syncGameTime();
	private:
		typedef std::map<DWORD,GateUser*> ACCID_MAP;
		ACCID_MAP m_mapAccids;

		std::map<QWORD, GateUser*> m_mapUsers; // 所有gateuser

		std::map<std::pair<DWORD,std::string>, GateUser*> account_map;	//所有帐号登陆的玩家

		/**
		 * \brief accid索引读写锁
		 */
		zRWLock arwlock;

		/**
		 * \brief uuid索引读写锁
		 */
		zRWLock urwlock;

		/**
		 * \brief 平台数字帐号读写锁
		 */
		zRWLock plat_account_rwlock;
};
#endif
