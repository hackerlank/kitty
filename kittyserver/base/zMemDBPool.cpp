
#include "zMemDBPool.h"
#include "xmlconfig.h"

/**
 * \brief MysqlClient默认HashCode函数,始终返回0
 * \param anyArg 任意参数
 * \return 始终返回0
 */
unsigned int defaultMemHashCode(const void *anyArg)
{
	QWORD id = *((QWORD*)anyArg);
	if (id==0) return 0;

	DWORD seed = config::redis_config().redismap().size()-1;
	if (0 == seed)
	{
		return 0;
	}
	return ((id % seed)+1);
}


zMemDBPool::zMemDBPool()
{
	hashCode=defaultMemHashCode;
	handles.clear();
	infoPool.clear();
	idset.clear();
}

bool zMemDBPool::init(hashHandleFunc hashfunc)
{
	if(hashfunc!=NULL)
		hashCode=hashfunc;
	else
		hashCode=defaultMemHashCode;

	auto redismap = config::redis_config().redismap();
	auto redismap_it = redismap.begin();
	for (;redismap_it!=redismap.end();redismap_it++)
	{
		Fir::logger->info("[memdb],hashcode:%d ip:%s port:%d", (DWORD)redismap_it->second.id(), 
				(const char*)redismap_it->second.ip(), (DWORD)redismap_it->second.port());

		this->putConnInfo(redismap_it->second.id(), redismap_it->second.ip(), redismap_it->second.port());
	}

	return true;
}

zMemDBPool::~zMemDBPool()
{
	mlock.lock();

	for(handlesPool_iterator it = handles.begin(); it != handles.end(); ++it)
	{
		for (hashHandlesMap_iterator iter = it->second.begin(); iter != it->second.end(); ++iter)
			SAFE_DELETE(iter->second);
	}

	handles.clear();
	infoPool.clear();
	hashCode=defaultMemHashCode;
	mlock.unlock();
}

bool zMemDBPool::putConnInfo(QWORD hashcode,const std::string& server, DWORD port, DWORD dbIndex)
{
	MemConnInfo mi(hashcode,server,port, dbIndex);

	mlock.lock();
	infoPool.push_back(mi);
	mlock.unlock();

	return true;
}

/*
zMemDB* zMemDBPool::getMemDBHandle(DWORD id)
{//id以后用来进行二级散列时用,例如CHARID,或ACCID，UUID等
	mlock.lock();
	pthread_t tid = ::pthread_self();
	handlesPool_iterator iter = handles.find(tid);
	if (iter!=handles.end())
	{
		mlock.unlock();
		
		if (iter->second && (zMemDB*)(iter->second)->is_valid())
			return (*iter).second;
		else
			return NULL;
	}
	mlock.unlock();

	//如果没有对应连接，则根据连接信息，创建新连接
	QWORD hashcode = hashCode(&id);//通过ID进行二级散列，找到二级散列对应的server:port所在进程连接,目前默认都是同一个URL
	connInfoPool_iterator infoIter = infoPool.find(hashcode);

	if (infoIter!=infoPool.end())
	{
		zMemDB* new_conn = FIR_NEW zMemDB(infoIter->second.host, infoIter->second.port, infoIter->second.dbIndex);
		if (!new_conn) return NULL;
				
		if (!new_conn->init()){SAFE_DELETE(new_conn); return NULL;}

		mlock.lock();
		handles.insert(handlesPool_pair(tid, new_conn));
		mlock.unlock();

		return new_conn;
	}

	return NULL;
}
*/

zMemDB* zMemDBPool::getMemDBHandle(DWORD key)
{
    DWORD id = key % MAX_MEM_DB + 1;
    //id以后用来进行二级散列时用,例如CHARID,或ACCID，UUID等,0为全局redis
	pthread_t tid = ::pthread_self();

	if (handles.count(tid)==0)//该线程无分配
	{
		hashHandlesMap hashHandles;
		for (unsigned int i = 0; i<infoPool.size(); i++)
		{
			zMemDB* new_conn = FIR_NEW zMemDB(infoPool[i].host, infoPool[i].port, infoPool[i].dbIndex,infoPool[i].hashcode);//创建redis实例
			if (!new_conn) return NULL;

			if (!new_conn->init()) {SAFE_DELETE(new_conn); return NULL;}

			hashHandles.insert(hashHandlesMap_pair(infoPool[i].hashcode, new_conn));//构成一对
		}

		mlock.lock();
		handles.insert(handlesPool_pair(tid, hashHandles));//改线程完成redis分配
		mlock.unlock();
	}

	QWORD hashcode = hashCode(&id);//通过ID进行二级散列，找到二级散列对应的server:port所在进程连接,目前默认都是同一个URL

#ifdef _ZJW_DEBUG	
	Fir::logger->debug("[memdb], tid:%lu hashcode:%d",tid,id);
#endif

	zMemDB* ret = NULL;

	mlock.lock();
	auto iter = handles.find(tid);//找到hash对
	if (iter!=handles.end())
	{
		auto handle_map = handles[tid];//hash对
	
		if (handle_map.count(hashcode) !=0)
			ret = handle_map[hashcode];//找到hash接口
	}
	mlock.unlock();

	return ret;
}

void zMemDBPool::flushALLMemDB()//清空redis
{
	mlock.lock();
	for (auto it = handles.begin(); it != handles.end(); ++it)
	{
		for (auto iter = it->second.begin(); iter != it->second.end(); ++iter)
		{
			if (iter->second)
			{
				iter->second->exec("FLUSHALL");
			}
		}
	}
	mlock.unlock();
}

bool zMemDBPool::loadLua(const std::string &filePath)//脚本加载
{
   zMemDB* handle = getMemDBHandle();
   if(handle ==NULL)
       return false;
   return handle->ReloadLuaScript(filePath);


}




