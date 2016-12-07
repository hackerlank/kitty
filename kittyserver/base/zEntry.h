/**
 * \file
 * \version  $Id: zEntry.h 67 2013-04-23 09:44:20Z  $
 * \author  ,okyhc@263.sina.com
 * \date 2004年12月15日 11时20分52秒 CST
 * \brief Entry基类定义，用于数据管理
 */

#ifndef _ZENTRY_H_
#define _ZENTRY_H_
#include "zMutex.h"
#include "zRWLock.h"
#include "zType.h"
#include "zNoncopyable.h"

/**
 * \brief Entry基类
 */

#pragma pack(1)
struct zEntryC
{
	/**
	 * \brief entry的数据ID，不同类型的Entry可能会重复,此时不能实现从ID查找entry
	 */
	QWORD id;
	/**
	 * \brief entry的临时id,建议在实现EntryManager时，保证分配唯一
	 */
	DWORD tempid;
	/**
	 * \brief entry的名字，不同类型的Entry可能会重复,此时不能实现从名字查找entry
	 */
	char name[MAX_NAMESIZE+1];
	zEntryC()
	{
		id=0;
		tempid=0;
		bzero(name,sizeof(name));
	};
};

/**
 * \brief 回调函数类模板
 */
template <typename T, typename RTValue = bool>
struct zEntryCallback
{
	virtual RTValue exec(T *e)=0;
	virtual ~zEntryCallback(){};
};

struct zEntry:public zEntryC,private zNoncopyable
{
	virtual ~zEntry(){};
	zEntry():zEntryC()
	{
	};
};
#pragma pack()

#endif
