/**
 * \file
 * \version  $Id: RecordTaskManager.h 36 2013-04-07 11:42:48Z  $
 * \author  ,
 * \date 2013年04月07日 15时28分53秒 CST
 * \brief 管理记录连接的容器
 *
 * 
 */


#ifndef _RecordTaskMANAGER_H_
#define _RecordTaskMANAGER_H_

#include <iostream>
#include <unordered_map>

#include "RecordTask.h"
#include "zRWLock.h"

/**
 * \brief 计费服务器子连接管理器
 *
 */
class RecordTaskManager : public Singleton<RecordTaskManager>
{	
	friend class Singleton<RecordTaskManager>;
	public:

		/**
		 * \brief 析构函数
		 *
		 */
		~RecordTaskManager() {};

		bool uniqueAdd(RecordTask *task);
		bool uniqueRemove(RecordTask *task);
		//bool broadcastByID(const WORD wdServerID, const void *pstrCmd, int nCmdLen);
		RecordTask *getTaskByID(const WORD wdServerID);
		bool sceneTaskEmpty();
        RecordTask *getFirstScen();
        bool broadcastByType(ServerType type,const void *pstrCmd, int nCmdLen);
        bool randByType(ServerType type,const void *pstrCmd, int nCmdLen);
	private:

		/**
		 * \brief 类的唯一实例指针
		 *
		 */
		static RecordTaskManager *instance;

		/**
		 * \brief 构造函数
		 *
		 */
		RecordTaskManager() {};

		/**
		 * \brief 定义容器类型
		 *
		 */
		typedef std::unordered_map<WORD, RecordTask *> RecordTaskHashmap;
		/**
		 * \brief 定义容器迭代器类型
		 *
		 */
		typedef RecordTaskHashmap::iterator RecordTaskHashmap_iterator;
		/**
		 * \brief 定义容器常量迭代器类型
		 *
		 */
		typedef RecordTaskHashmap::const_iterator RecordTaskHashmap_const_iterator;
		/**
		 * \brief 定义容器键值对类型
		 *
		 */
		typedef RecordTaskHashmap::value_type RecordTaskHashmap_pair;
		/**
		 * \brief 容器访问互斥变量
		 *
		 */
		zRWLock rwlock;
		/**
		 * \brief 声明一个容器，存放所有的子连接
		 *
		 */
		RecordTaskHashmap RecordTaskSet;

};

#endif

