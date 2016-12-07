/**
 * \file
 * \version  $Id: GatewayTaskManager.h 24 2013-03-30 08:04:25Z  $
 * \author  ,@163.com
 * \date 2004年12月20日 17时28分53秒 CST
 * \brief 管理子连接的容器
 *
 * 
 */


#ifndef _GatewayTaskManager_h_
#define _GatewayTaskManager_h_

#include <iostream>
#include <unordered_map>

#include "zEntry.h"
#include "GatewayTask.h"
#include "zRWLock.h"

/**
 * \brief 服务器子连接管理器
 *
 */
class GatewayTaskManager : public Singleton<GatewayTaskManager>
{
	friend class Singleton<GatewayTaskManager>;
	public:

		/**
		 * \brief 回调函数
		 *
		 */
		typedef zEntryCallback<GatewayTask> GatewayTaskCallback;

		/**
		 * \brief 析构函数
		 *
		 */
		~GatewayTaskManager();

		bool uniqueAdd(GatewayTask *task);
		bool uniqueRemove(GatewayTask *task);
		void execAll(GatewayTaskCallback &callback);
		void execAllCheckTime();

	private:

		/**
		 * \brief 类的唯一实例指针
		 *
		 */
		static GatewayTaskManager *instance;

		/**
		 * \brief 构造函数
		 *
		 */
		GatewayTaskManager();

		/**
		 * \brief 定义容器类型
		 *
		 */
		typedef std::unordered_map<DWORD, GatewayTask *> GatewayTaskHashmap;
		/**
		 * \brief 定义容器迭代器类型
		 *
		 */
		typedef GatewayTaskHashmap::iterator GatewayTaskHashmap_iterator;
		/**
		 * \brief 定义容器常量迭代器类型
		 *
		 */
		typedef GatewayTaskHashmap::const_iterator GatewayTaskHashmap_const_iterator;
		/**
		 * \brief 定义容器键值对类型
		 *
		 */
		typedef GatewayTaskHashmap::value_type GatewayTaskHashmap_pair;
		/**
		 * \brief 子连接管理容器类型
		 *
		 */
		GatewayTaskHashmap gatewayTaskSet;

		typedef std::unordered_map<std::string, GatewayTask*> gatewaytask_uuid_hashmap;
		typedef gatewaytask_uuid_hashmap::iterator uuid_hashmap_iterator;
		typedef gatewaytask_uuid_hashmap::value_type uuid_hashmap_pair;
		typedef gatewaytask_uuid_hashmap::const_iterator uuid_hashmap_const_iterator;
		gatewaytask_uuid_hashmap gatewaytask_uuidset;

		std::map<std::pair<DWORD, std::string>, GatewayTask*> account_map;//帐号登陆连接

		/**
		 * \brief 容器访问互斥变量
		 *
		 */
		zRWLock rwlock;
};

#endif

