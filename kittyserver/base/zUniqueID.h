/**
 * \file
 * \version  $Id: zUniqueID.h 13 2013-03-20 02:35:18Z  $
 * \author  ,okyhc@263.sina.com
 * \date 2004年11月03日 16时39分42秒 CST
 * \brief 唯一编号生成器模板定义
 *
 * 
 */

#ifndef _ZUNIQEID_
#define _ZUNIQEID_

#include <pthread.h>
#include <list>
#include <set>
#include <ext/pool_allocator.h>

#include "zType.h"
#include "zMutex.h"
#include "zNoncopyable.h"
/**
 * \brief zUniqueID模板
 * 本模板实现了唯一ID生成器，并保证线程安全。
 * 可以用各种长度的无符号整数作为ID。
 */
template <class T>
class zUniqueID:private zNoncopyable
{
	private:
		/**
		 * \brief 锁
		 *
		 */
		zMutex mutex;
		/**
		 * \brief ID生成器可用列表唯一性保证类型
		 *
		 */
		typedef std::set<T> zUniqueSet;
		/**
		 * \brief ID生成器可用列表唯一性保证类型迭代
		 *
		 */
		//typedef typename zUniqueSet::iterator zUniqueSet_iter;
		/**
		 * \brief 可用的id列表唯一性控制
		 *
		 */
		zUniqueSet idset;
		/**
		 * \brief ID生成器可用列表类型
		 *
		 */
		typedef std::list<T, __gnu_cxx::__pool_alloc<T> > zUniqueList;
		/**
		 * \brief ID生成器可用列表类型迭代
		 *
		 */
		typedef typename zUniqueList::iterator zUniqueList_iter;
		/**
		 * \brief 可用的id列表
		 *
		 */
		zUniqueList idlist;
		/**
		 * \brief 分配器中最大的id
		 *
		 */
		T maxID;
		/**
		 * \brief 分配器中最小的id
		 *
		 */
		T minID;
		/**
		 * \brief 分配器中下一个可用id
		 *
		 */
		T curMaxID;
		/**
		 * \brief 初始化分配器
		 *
		 *
		 * \param min	最小值
		 * \param max	最大值
		 * \return 
		 */
		void init(T min,T max)
		{
			minID=min;
			maxID=max;
			curMaxID=minID;
		}

	public:
		/**
		 * \brief 默认构造函数 
		 * 开始ID为1，最大有效ID为(T)-2,无效ID为(T)-1
		 */
		zUniqueID()
		{
			init(1,(T)-1);
		}

		/**
		 * \brief 构造函数 
		 * 用户自定义起始ID，最大有效ID为(T)-2,无效ID为(T)-1
		 * \param startID 用户自定义的起始ID
		 */
		zUniqueID(T startID)
		{
			init(startID,(T)-1);
		}

		/**
		 * \brief 构造函数 
		 * 用户自定义起始ID，及最大无效ID,最大有效ID为最大无效ID-1
		 * \param startID 用户自定义的起始ID
		 * \param endID 用户自定义的最大无效ID
		 */
		zUniqueID(T startID,T endID)
		{
			init(startID,endID);
		}

		/**
		 * \brief 析构函数 
		 * 回收已分配的ID内存。
		 */
		~zUniqueID()
		{
			mutex.lock();
			idlist.clear();
			mutex.unlock();
		}

		/**
		 * \brief 得到最大无效ID 
		 * \return 返回最大无效ID
		 */
		T invalid()
		{
			return maxID;
		}

		/**
		 * \brief 测试这个ID是否被分配出去
		 * \return 被分配出去返回true,无效ID和未分配ID返回false
		 */
		bool hasAssigned(T testid)
		{
			mutex.lock();
			if(testid<maxID && testid>=minID)
			{
				if(idset.find(testid) != idset.end())
				{
						mutex.unlock();
						return false;
				}
				mutex.unlock();
				return true;
			}
			mutex.unlock();
			return false;
		}

		/**
		 * \brief 得到一个唯一ID 
		 * \return 返回一个唯一ID，如果返回最大无效ID，比表示所有ID都已被用，无可用ID。
		 */
		T get()
		{
			T ret;
			mutex.lock();
			if(maxID>curMaxID)
			{
				ret=curMaxID;
				curMaxID++;
			}
			else
				ret=maxID;
			if(ret == maxID && !idlist.empty())
			{
				ret=idlist.back();
				idlist.pop_back();
				idset.erase(ret);
			}
			mutex.unlock();
			return ret;
		}

		/**
		 * \brief 一次得到多个ID，这些ID都是相邻的,并且不回被放回去 
		 * \param size 要分配的ID个数
		 * \param count 实际分配ID的个数
		 * \return 返回第一个ID，如果返回最大无效ID，比表示所有ID都已被用，无可用ID。
		 */
		T get(int size,int & count)
		{
			T ret;
			mutex.lock();
			if(maxID>curMaxID)
			{
				count=(maxID-curMaxID)>size?size:(maxID-curMaxID);
				ret=curMaxID;
				curMaxID+=count;
			}
			else
			{
				count=0;
				ret=maxID;
			}
			mutex.unlock();
			return ret;
		}

		/**
		 * \brief 将ID放回ID池，以便下次使用。 
		 * 
		 * 放回的ID必须是由get函数得到的。并且不能保证放回的ID,没有被其他线程使用。
		 * 所以用户要自己保证还在使用的ID不会被放回去。以免出现ID重复现象。
		 * \param id 由get得到的ID.
		 */
		void put(T id)
		{
			mutex.lock();
			if(id<maxID && id>=minID)
			{
				if(idset.insert(id).second)
				{
					idlist.push_front(id);
				}
			}
			mutex.unlock();
		}
};

/**
 * \brief DWORD的id分配器
 *
 */
typedef zUniqueID<DWORD> zUniqueDWORDID;
/**
 * \brief WORD的id分配器
 *
 */
typedef zUniqueID<WORD> zUniqueWORDID;
#endif
