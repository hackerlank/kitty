/**
 * \file
 * \version  $Id: LoginManager.h 2536 2005-08-22 06:52:38Z whj $
 * \author  Songsiliang,
 * \date 2004年12月17日 13时17分28秒 CST
 * \brief 登陆连接管理容器
 *
 * 
 */


#ifndef RESOURCE_TASK_MANAGER_H
#define RESOURCE_TASK_MANAGER_H 

#include <ext/hash_map>

#include "zEntry.h"
#include "resourceTask.h"
#include "zMisc.h"
#include "zRWLock.h"

/**
 * \brief 登陆连接管理容器
 *
 * 管理所有的登陆连接的容器，方便查找连接
 *
 */
class ResourceTaskManager : public Singleton<ResourceTaskManager>
{

	public:

		/**
		 * \brief 定义回调函数类
		 *
		 */
		typedef zEntryCallback<ResourceTask, void> ResourceTaskCallback;
		bool add(ResourceTask *task);
		void remove(ResourceTask *task);
		void execAll(ResourceTaskCallback &cb);
        //获得连接
        ResourceTask* getTask(const DWORD id);
        ResourceTask* getTaskByMod(const QWORD charID);
	private:
		friend class Singleton<ResourceTaskManager>;
		ResourceTaskManager(){};
		~ResourceTaskManager() {};

		typedef __gnu_cxx::hash_map<const char*,ResourceTask*> ResourceTaskHashmap;
		typedef ResourceTaskHashmap::iterator ResourceTaskHashmap_iterator;
		typedef ResourceTaskHashmap::const_iterator ResourceTaskHashmap_const_iterator;
		typedef ResourceTaskHashmap::value_type ResourceTaskHashmap_pair;
		zRWLock rwlock;
		ResourceTaskHashmap resourceTaskSet;
        std::map<DWORD,ResourceTask*> m_resourceIDMap;
};

#endif

