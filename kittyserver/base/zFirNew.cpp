/**
 * \file zDebugNew.cpp
 * \version  $Id: zFirNew.cpp 13 2013-03-20 02:35:18Z  $
 * \author  , 
 * \date 2009年01月19日 14时05分13秒 CST
 * \brief 内存泄漏检查工具， 重载new操作
 *
 * 
 */


#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <unordered_map>
#include "Fir.h"
#include "zMutex.h"
#include <string.h>

#ifndef DEBUG_NEW_TABLESIZE
#define DEBUG_NEW_TABLESIZE (16384*100)
#endif

#ifndef DEBUG_NEW_HASH
#define DEBUG_NEW_HASH(p) ((reinterpret_cast<unsigned long>(p)>>8) % DEBUG_NEW_TABLESIZE)
#endif


#ifndef DEBUG_NEW_FILENAME_SIZE
#define DEBUG_NEW_FILENAME_SIZE 32
#endif


#ifdef _DEBUG_CHECK_LEAKS


struct NewAddr
{
	const char* file;
	int line;
	size_t size;
	
	NewAddr(const char* _file,int _line,size_t _size):file(_file),line(_line),size(_size){}
};

std::unordered_map<QWORD,NewAddr> newAddrMap;

static zMutex mem_mutex;
static zMutex mutex;

void addNewAddr(void* p,const char* file,int line,size_t size)
{
	mutex.lock();
	newAddrMap.insert(std::make_pair((QWORD)p,NewAddr(file,line,size)));
	mutex.unlock();
}

void removeNewAddr(void* p)
{
	mutex.lock();
	newAddrMap.erase((QWORD)p);
	mutex.unlock();
}



struct new_ptr_list_t
{
	new_ptr_list_t* next;
	char fn[DEBUG_NEW_FILENAME_SIZE];
	int line;
	size_t size;
	new_ptr_list_t()
	{
		next=NULL;
		bzero(fn, DEBUG_NEW_FILENAME_SIZE);
		line = 0;
		size = 0;
	}
};

static new_ptr_list_t* new_ptr_list[DEBUG_NEW_TABLESIZE];
size_t list_size = 0;

bool check_leaks()
{
	struct ListEntry{
		char name[DEBUG_NEW_FILENAME_SIZE + 8];
		int size;
	};
	
	bool fLeaked = false;
	size_t index = 0;
	ListEntry* tempList = NULL;
	
	mem_mutex.lock();
	tempList = (ListEntry*)malloc(sizeof(ListEntry) * list_size);
	if(!tempList)
	{
		mem_mutex.unlock();
		return false;
	}
	
	for (int i=0; i<DEBUG_NEW_TABLESIZE; ++i)
	{
		new_ptr_list_t* ptr = new_ptr_list[i];
		if (ptr==NULL)
			continue;
		fLeaked = true;
		while (ptr)
		{
//			if (Fir::logger)
//				Fir::logger->trace("[内存泄漏]：%p (size %u, %s:%d)", (char*)ptr+sizeof(new_ptr_list), ptr->size, ptr->fn, ptr->line);
			if(index < list_size)
			{
				snprintf(tempList[index].name, DEBUG_NEW_FILENAME_SIZE + 8, "%s:%d", ptr->fn, ptr->line);
				tempList[index].size = ptr->size;
				++index;
			}

			ptr=ptr->next;
		}
	}
	mem_mutex.unlock();

	std::map<std::string, int> leak_count;
	for(size_t i = 0;i < index;++i)
		leak_count[tempList[i].name] += tempList[i].size;
	free(tempList);		

	for (std::map<std::string,int>::iterator it=leak_count.begin(); it!=leak_count.end(); it++)
	{
		Fir::logger->trace("[内存泄漏分类统计]: %s  size:%d", it->first.c_str(), it->second);
	}

	mutex.lock();
	for (std::unordered_map<QWORD,NewAddr>::iterator iter = newAddrMap.begin();iter != newAddrMap.end();++iter)
	{
		fLeaked = true;
		Fir::logger->trace("[内存泄漏分类统计]: %s:%u  size:%lu",iter->second.file,iter->second.line,iter->second.size);
	}
	mutex.unlock();
	
	if (!fLeaked)
	{
		Fir::logger->trace("[内存泄漏]：该程序没有任何内存泄漏");
	}
	
	return fLeaked;
}

void* operator new(size_t size, const char* file, int line)
{
	size_t s = size+sizeof(new_ptr_list_t);
	new_ptr_list_t* ptr = (new_ptr_list_t*)malloc(s);
	if (ptr == NULL)
	{
		abort();
	}

	void* p = (char*)ptr+sizeof(new_ptr_list_t);
	
		
	size_t hash_index = DEBUG_NEW_HASH(p);
	strncpy(ptr->fn, file, DEBUG_NEW_FILENAME_SIZE-1);
	ptr->line = line;
	ptr->size = size;
	
	mem_mutex.lock();
	ptr->next = new_ptr_list[hash_index];
	new_ptr_list[hash_index] = ptr;
	++list_size;
	mem_mutex.unlock();

	//	if (file && line)	
//			fprintf(stdout, "内存分配: %p(size %u, %s:%d)\n", p, ptr->size, ptr->fn, ptr->line);

	return p;
}

void* operator new[](size_t size, const char* file, int line)
{
	return operator new(size, file, line);
}


void* operator new(size_t size)
{
		return ::malloc(size);
//	return operator new(size, "<Unknown>", 0);
}

void* operator new[](size_t size)
{
	return ::malloc(size);
//	return operator new(size);
}



void* operator new(size_t size, const std::nothrow_t&) throw()
{
	return operator new(size);
}

void* operator new[](size_t size, const std::nothrow_t&) throw()
{
	return operator new[](size);
}


void operator delete(void* pointer)
{
	if (pointer == NULL)
		return;

	size_t hash_index = DEBUG_NEW_HASH(pointer);
	new_ptr_list_t* ptr_pre = NULL;
	
	mem_mutex.lock();
	new_ptr_list_t* ptr = new_ptr_list[hash_index];
	while (ptr)
	{
		if ((char*)ptr+sizeof(new_ptr_list_t) == pointer)
		{

//			if (ptr->fn && ptr->line)	
//				fprintf(stdout, "内存释放: %p(size %u)\n", pointer, ptr->size);
			
			if (ptr_pre == NULL)
				new_ptr_list[hash_index] = ptr->next;
			else
				ptr_pre->next = ptr->next;

			--list_size;
			mem_mutex.unlock();
			free(ptr);
			return;
		}

		ptr_pre = ptr;
		ptr = ptr->next;
	}

	mem_mutex.unlock();
	free(pointer);

//	if (Fir::logger)
//		Fir::logger->trace("[内存释放]：无效地址 %p", pointer);
//	abort();
}


void operator delete[](void* pointer)
{
	operator delete(pointer);
}

void  operator delete(void* pointer, const char* file, int line)
{
	operator delete(pointer);
}

void  operator delete[](void* pointer, const char* file, int line)
{
	operator delete(pointer);
}

void  operator delete(void* pointer, const std::nothrow_t&)
{
	operator delete(pointer, "<Unknown>", 0);
}

void  operator delete[](void* pointer, const std::nothrow_t&)
{
	operator delete(pointer, "<Unknown>", 0);
}


#endif
