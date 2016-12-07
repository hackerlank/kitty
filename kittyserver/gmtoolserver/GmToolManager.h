/**
 * \file
 * \version  $Id: LoginManager.h 2536 2005-08-22 06:52:38Z whj $
 * \author  Songsiliang,
 * \date 2004年12月17日 13时17分28秒 CST
 * \brief 登陆连接管理容器
 *
 * 
 */


#ifndef GM_TOOL_TASK_MANAGER_H
#define GM_TOOL_TASK_MANAGER_H

#include <ext/hash_map>

#include "zEntry.h"
#include "GmToolTask.h"
#include "zMisc.h"
#include "zRWLock.h"

/**
 * \brief 登陆连接管理容器
 *
 * 管理所有的登陆连接的容器，方便查找连接
 *
 */
class GmToolTaskManager : public Singleton<GmToolTaskManager>
{

	public:

		/**
		 * \brief 定义回调函数类
		 *
		 */
		typedef zEntryCallback<GmToolTask, void> GmToolTaskCallback;
		bool add(GmToolTask *task);
		void remove(GmToolTask *task);
		bool broadcast(const char *account, const void *pstrCmd, int nCmdLen);
		void execAll(GmToolTaskCallback &cb);
        GmToolTask* getTask(const char *account);
        void update();
        //获得连接
        GmToolTask* getTask(const DWORD id);
	private:
		friend class Singleton<GmToolTaskManager>;
		GmToolTaskManager(){};
		~GmToolTaskManager() {};

		typedef __gnu_cxx::hash_map<const char*,GmToolTask *> GmToolTaskHashmap;
		typedef GmToolTaskHashmap::iterator GmToolTaskHashmap_iterator;
		typedef GmToolTaskHashmap::const_iterator GmToolTaskHashmap_const_iterator;
		typedef GmToolTaskHashmap::value_type GmToolTaskHashmap_pair;
		zRWLock rwlock;
		GmToolTaskHashmap gmToolTaskSet;
        std::map<DWORD,GmToolTask*> m_gmToolIDMap;
};

#endif

