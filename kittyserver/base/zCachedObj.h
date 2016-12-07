/**
 * \file	zCachedObj.h
 * \version  	$Id: zCachedObj.h 13 2013-03-20 02:35:18Z  $
 * \author  	huanglie,huanglie1986@gmail.com
 * \date 	2009年10月17日 20时05分56秒 CST
 * \brief	对象内存空间缓存机制。	
 *			使用时在类声明中用宏定义FIR_CACHEDOBJ_DEC,如:
 *			class SessionChild{
 *				FIR_CACHEDOBJ_DEC(SessionChild,100)
 *			};
 *			多线程环境使用FIR_SAFECACHEDOBJ_DEC
 */

#ifndef _CACHEDOBJ_H
#define _CACHEDOBJ_H

#include <stdexcept>
#include "zFirNew.h"
#include "zMutex.h"

#define MIN_INCREASIZE 30

class NullMutex
{
public:
	void lock(){}
	void unlock(){}
};

template<typename ObjT,size_t Increa = 100,typename MutexT = NullMutex>
class zCachedObj
{
	struct ListEntry
	{
		ListEntry* next;
		ObjT  obj;
	};
	
	friend void* ObjT::operator new(size_t);
	friend void ObjT::operator delete(void*,size_t);
	
	static void *alloc()
	{
		m_mutex.lock();
		if(NULL == m_objList) try{ increase(); }
		catch( ... ){ m_mutex.unlock(); throw; }
		ListEntry* ret = m_objList;
		m_objList = ret->next;
		m_mutex.unlock();
		return &ret->obj;
	}

	static void free(void* ptr)
	{
		if(NULL == ptr) return;
		static size_t offset = (size_t)&(((ListEntry*)1)->obj)-1;
		ListEntry* entry = (ListEntry*)((char*)ptr - offset);
		m_mutex.lock();
		entry->next = m_objList;
		m_objList = entry;
		m_mutex.unlock();
	}

	static void increase();
	
	static ListEntry* m_objList;
	static MutexT m_mutex;
};

template<typename ObjT,size_t Increa,typename MutexT>
typename zCachedObj<ObjT,Increa,MutexT>::ListEntry* zCachedObj<ObjT,Increa,MutexT>::m_objList = NULL;
template<typename ObjT,size_t Increa,typename MutexT>
MutexT zCachedObj<ObjT,Increa,MutexT>::m_mutex;

template<typename ObjT,size_t Increa,typename MutexT>
void zCachedObj<ObjT,Increa,MutexT>::increase()
{
	static size_t size = Increa < MIN_INCREASIZE ? MIN_INCREASIZE : Increa;
	while(1)
	{
		ListEntry *ptr = 
			static_cast<ListEntry*>(malloc(sizeof(ListEntry) * size));
		if(ptr){
			for(ListEntry *end  = ptr + size;ptr != end;++ptr)
			{
				ptr->next = m_objList;
				m_objList = ptr;
			}
			return;
		}

		std::new_handler handler = std::set_new_handler(0);
		std::set_new_handler(handler);
		if(handler) (*handler)();
		else throw std::bad_alloc();
	}
}

/*
 *\brief 在类声明中使用以下宏定义支持对象内存缓存
 */
#ifdef _DEBUG_CHECK_LEAKS

#define FIR_CACHEDOBJ_DEC(classT,_increa) \
	public: \
		static void* operator new(size_t size) { \
			if(size != sizeof(classT)) return ::operator new(size); \
			else return zCachedObj<classT,_increa>::alloc(); \
		} \
		static void* operator new(size_t size,const char* file,int line) { \
			void* ret = operator new(size);\
			addNewAddr(ret,file,line,size); \
			return ret; \
		} \
		static void operator delete(void* ptr,size_t size) { \
			removeNewAddr(ptr); \
			if(size != sizeof(classT)) ::operator delete(ptr); \
			else zCachedObj<classT,_increa>::free(ptr); \
		} \

#define FIR_SAFECACHEDOBJ_DEC(classT,_increa) \
	public: \
		static void* operator new(size_t size) { \
			if(size != sizeof(classT)) return ::operator new(size); \
			else return zCachedObj<classT,_increa,zMutex>::alloc(); \
		} \
		static void* operator new(size_t size,const char* file,int line) { \
			void* ret = operator new(size);\
			addNewAddr(ret,file,line,size); \
			return ret; \
		} \
		static void operator delete(void* ptr,size_t size) { \
			removeNewAddr(ptr); \
			if(size != sizeof(classT)) ::operator delete(ptr); \
			else zCachedObj<classT,_increa,zMutex>::free(ptr); \
		} \

#else

#define FIR_CACHEDOBJ_DEC(classT,_increa) \
	public: \
		static void* operator new(size_t size) { \
			if(size != sizeof(classT)) return ::operator new(size); \
			else return zCachedObj<classT,_increa>::alloc(); \
		} \
		static void operator delete(void* ptr,size_t size) { \
			if(size != sizeof(classT)) ::operator delete(ptr); \
			else zCachedObj<classT,_increa>::free(ptr); \
		} \

#define FIR_SAFECACHEDOBJ_DEC(classT,_increa) \
	public: \
		static void* operator new(size_t size) { \
			if(size != sizeof(classT)) return ::operator new(size); \
			else return zCachedObj<classT,_increa,zMutex>::alloc(); \
		} \
		static void operator delete(void* ptr,size_t size) { \
			if(size != sizeof(classT)) ::operator delete(ptr); \
			else zCachedObj<classT,_increa,zMutex>::free(ptr); \
		} \

#endif

#endif
