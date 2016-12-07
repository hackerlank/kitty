/**
 * \file zFirNew.h
 * \version  $Id: zFirNew.h 92 2013-04-26 05:17:24Z  $
 * \author  , 
 * \date 2009年01月19日 14时05分13秒 CST
 * \brief 内存泄漏检查工具， 重载new操作
 *
 * 
 */

#ifndef _DEBUG_NEW
#define _DEBUG_NEW

#include <new>
#include <cstddef>

#ifdef _DEBUG_CHECK_LEAKS

#define CHECK_LEAKS check_leaks()
// 检查未释放内存
bool check_leaks();

//重载new和delete
extern void* operator new(size_t size, const char* file, int line);
extern void* operator new[](std::size_t size, const char* file, int line);

//void* operator new(size_t size, void* ptr);
//void* operator new[](size_t size, void* ptr);

extern void operator delete(void* p, const char* f, int line);
extern void operator delete[](void*p , const char* f, int line);


void addNewAddr(void* p,const char* file,int line,std::size_t size);
void removeNewAddr(void* p);


#define FIR_NEW new(__FILE__, __LINE__)

#else

#define CHECK_LEAKS
#define FIR_NEW new
#endif

#endif




