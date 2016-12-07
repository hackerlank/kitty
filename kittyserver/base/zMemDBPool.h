#ifndef _ZMEMDBPOOL_H_
#define _ZMEMDBPOOL_H_

#include <string.h>
#include <set>
#include <hiredis.h>
#include <unordered_map>
#include "Fir.h"
#include "zType.h"
#include "zMemDB.h"
#include "zMutex.h"
#include "zSingleton.h"


/**
 * \brief 哈希代码函数类型定义
 * 
 * 用户可以根据自己的需要写自己的哈希函数，以便对相对应用户定义的数据库进行操作。
 */
typedef unsigned int(* hashHandleFunc)(const void *data);

/**
 * \brief 连接句柄,用户调用使用,只能从链接池中得到
 */
typedef unsigned int memdbHandleID;

struct MemConnInfo
{
	unsigned int hashcode;

	std::string host;//server ip
	DWORD port;//server port
	DWORD dbIndex;//database 1-15

	MemConnInfo(const unsigned int hashcode,const std::string &host,DWORD port,	DWORD dbIndex)
		: hashcode(hashcode),host(host),port(port),dbIndex(dbIndex)
	{
	}

	MemConnInfo(const MemConnInfo &ui)
		: hashcode(ui.hashcode),host(ui.host),port(ui.port),dbIndex(ui.dbIndex)
	{
	}
};

class zMemDBPool : public Fir::Singleton<zMemDBPool>
{
	public:
		bool init(hashHandleFunc hashfunc=NULL);

		zMemDBPool();

		/**
		 * \brief 接口析构虚函数
		 */
		~zMemDBPool();

		/**
		 * \brief 向连接池中添加数据库连接URL
		 *
		 * \param hashcode 此连接所对应的哈希代码，使用者需要指定。
		 * \param url 数据库联接的url
		 * \return 成功返回true，否则返回false
		 */
		bool putConnInfo(QWORD hashcode,const std::string& server, DWORD port, DWORD dbIndex=0);

		/**
		 * \brief 得到连接Handle,内部以当前线程ID对应一个memdb连接，
		 * 以后需要根据玩家ID散列到不同数据库时，在线程ID下面，
		 * 再建立一级管理器，目前就简单的按一级管理实现
		 *
		 * \param id 玩家ID，以后根据玩家ID进行二级散列时用
		 * \return 数据库联接句柄,-1表示无效句柄
		 */
		zMemDB* getMemDBHandle(DWORD id=0);

		/**
		 * \brief FLUSHALL
		 */
		void flushALLMemDB();
        bool loadLua(const std::string &filePath);

	private:

		/*
		typedef std::unordered_multimap<QWORD,zMemDB *> memDBPool;//hashcode,dbconn，二级，根据用户ID散列到多个memdb进程
		typedef std::unordered_map<QWORD,memDBPool>  handlesPool; // thread_t,memDBPool,一级，按线程ID散列到memdb连接管理器
		*/

		//typedef std::unordered_map<QWORD,zMemDB *> handlesPool;//thread_t,根据所在线程，散列到不同memdb,如果该线程还没有对应的memdb连接，则新创建，如果创建失败，则返回NULL
		
		typedef std::vector<MemConnInfo> connInfoPool;//memdb进程连接, 根据配置的编号填写,putConnInfo

		typedef std::unordered_map<QWORD, zMemDB*> hashHandlesMap;
		typedef std::unordered_map<QWORD, hashHandlesMap> handlesPool;//thread_t,根据所在线程，散列到不同memdb,如果该线程还没有对应的memdb连接，则新创建，如果创建失败，则返回NULL

		typedef handlesPool::iterator handlesPool_iterator;
		typedef handlesPool::value_type handlesPool_pair;

		typedef hashHandlesMap::iterator hashHandlesMap_iterator;
		typedef hashHandlesMap::value_type hashHandlesMap_pair;


		handlesPool handles;//一级 线程id 二级 hashcode,redis对

		connInfoPool infoPool;
		std::set<QWORD> idset;//已分配ID
		hashHandleFunc hashCode;

		zMutex mlock;
};
#endif

