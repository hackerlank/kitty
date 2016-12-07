/**
 * \file
 * \version  $Id: ServerManager.h 855 2005-04-04 13:53:18Z song $
 * \author  Songsiliang,
 * \date 2004年12月13日 18时44分39秒 CST
 * \brief 定义服务器管理容器
 *
 * 这个容器包括全局容器和唯一性验证容器
 * 
 */


#ifndef _ServerManager_h_
#define _ServerManager_h_

#include <iostream>
#include <map>

#include "ServerTask.h"
#include "zRWLock.h"
#include "zNoncopyable.h"
#include "ServerACL.h"
#include "NetType.h"

/**
 * \brief 服务器管理容器类
 *
 * 这个容器包括全局容器和唯一性验证容器
 *
 */
class ServerManager : public Singleton<ServerManager>
{

	public:

		void add(ServerTask *task);
		void remove(ServerTask *task);
		bool sendCmdToZone(const DWORD zoneid, const void *pstrCmd, int nCmdLen);
		
		// 获得注册人数最少的区
		DWORD getRegRoleMinZone();
	private:

		friend class Singleton<ServerManager>;
		ServerManager() {};
		~ServerManager() {};

		typedef std::multimap<const NetType, ServerTask *> NetServer_multimap;
		typedef NetServer_multimap::iterator iter;
		typedef NetServer_multimap::const_iterator const_iter;
		typedef NetServer_multimap::value_type value_type;
		typedef std::map<const DWORD, NetServer_multimap> ZoneServer_map;
		ZoneServer_map mapper;
		zRWLock rwlock;

		bool _sendCmdToZone(ZoneServer_map::iterator &map_it, const void *pstrCmd, int nCmdLen)
		{
			NetType netType = NetType_near;//ServerACL::getMe().getNetType(map_it->first);
			NetServer_multimap &mtp = map_it->second;
			std::pair<iter, iter> eqp = mtp.equal_range(netType);
			for(iter it = eqp.first; it != eqp.second; ++it)
			{
				ServerTask *task = it->second;
				if (task
						&& task->sendCmd(pstrCmd, nCmdLen))
					return true;
			}
			eqp = mtp.equal_range(NetType_near);
			for(iter it = eqp.first; it != eqp.second; ++it)
			{
				ServerTask *task = it->second;
				if (task
						&& task->sendCmd(pstrCmd, nCmdLen))
					return true;
			}
			return false;
		}

};

#endif

