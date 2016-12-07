/**
 * \file: zRTime.h
 * \version  $Id: zNewTime.h 13 2013-03-20 02:35:18Z  $
 * \author  , @ztgame.com 
 * \date 2010年06月24日 03时36分17秒 CST
 * \brief 时间相关类
 *
 * 
 */


#pragma once 

#include <time.h>
#include <sys/time.h>
#include <sstream>
#include "Fir.h"
#include "zType.h"
#include "zFunctor.h"
#include "zTime.h"


 /// @brief	秒,分,时,日,周的毫秒数的宏.FOREVER代表不再执行,是DWORD的最大值
#define ONE_SEC (1000)
#define HALF_SEC (500)
#define ONE_MIN (60000)
#define HALF_MIN (30000)
#define ONE_HOUR (3600000)
#define HALF_HOUR (1800000)
#define ONE_DAY (86400000)
#define HALF_DAY (43200000)
#define ONE_WEEK (604800000)
#define FOREVER (4294967294)



typedef Fir::Functor<bool, Fir::NullType> TimerHandle;

 /// @brief	定时器
 // 			固定时间间隔的定时器，方便对于时间间隔的判断，精确到毫秒级
class FTimer
{
	public:

		 /// @brief	空构造
		FTimer() : interval(0),last_time() {}
		
		 /// @brief	构造函数
		 /// @param	how_long	定时器的时间，单位：毫秒
		 /// @param	first		有些定时器可能希望在启动时就可以执行一次,所以不能直接addDelay哦
		explicit FTimer(const QWORD how_long, bool first=false, const QWORD delay=0) : interval(how_long), last_time()
		{
			if(!first)
				last_time.addDelay(interval+delay);
		}

		 /// @brief	构造函数
		 /// @param	how_long	定时器的时间，单位：毫秒
		 /// @param	first		有些定时器可能希望在启动时就可以执行一次,所以不能直接addDelay哦
		 /// @param	ct		当前时间
		explicit FTimer(const QWORD how_long, bool first=false, zRTime &ct) : interval(how_long), last_time(ct)
		{
			if(!first)
			{
				zRTime ct;
				addDelay(ct);
			}
		}

		 /// @brief	重新设置定时器的间隔和开始计时时间
		 /// @param	how_long	定时器的时间，单位：毫秒
		 /// @param	ctv		当前时间
		void reset(const QWORD how_long, const zRTime &ct)
		{
			interval = how_long;
			last_time = ct;
			last_time.addDelay(interval);
		}

		 /// @brief	重新设置定时器的时间
		 /// @param	ct		指定定时器启动的时间
		void current(const zRTime &ct)
		{
			last_time = ct;
		} 

		 /// @brief	延时定时器时间
		 /// @param	ct	指定定时器启动的时间
		 /// @param	delay	延时时间
		void next(const zRTime &ct, const DWORD delay)
		{
			last_time = ct;
			last_time.addDelay(delay);
		} 

		 /// @brief	重新设置定时器的时间
		 /// @param	ct		指定定时器启动的时间
		void next(const zRTime &ct)
		{
			last_time = ct;
			last_time.addDelay(interval);
		} 

		 /// @brief	倒计时剩余秒数.不受时间调整影响.
		 /// @param	ct	当前时间	
		 /// @return	剩余描述
		inline DWORD leftSec(const zRTime &ct)
		{
			return (last_time.sec() > ct.sec()) ? (last_time.sec() - last_time.sec()) : 0;
		} 

		 /// @brief	倒计时剩余毫秒数.受时间调整影响
		 /// @param	cur	当前时间	
		 /// @return	剩余值
		inline unsigned long leftMSec(const zRTime &ct)
		{
			return (last_time.msecs() > ct.msecs()) ? (last_time.msecs() - ct.msecs()) : 0; 
		} 

		 /// @brief	定时器检查
		 /// @param	cur	检查定时器的时间
		 /// @return	是否到达指定时间
		inline bool operator() (const zRTime &ct)
		{
			if (last_time.msecs() > ct.msecs())
			{
				return false;
			}
			
			handle();
			addDelay(ct);
			
			return true;
		}

		 /// @brief	空定时器
		inline bool empty()
		{
			return interval == 0 ? true : false;
		}
		
		inline bool setHandle(TimerHandle h)
		{
			handle = h;
			return true;
		}
		
	private:

		/// @brief 定点执行的回调函数
		TimerHandle handle;

		 /// @brief	定时器时间间隔
		QWORD interval;

		 /// @brief	上次检查定时器的时间
		zRTime last_time;
		
	private:

		void addDelay(const zRTime& ct)
		{ 
			// 计算从最后一次定时到现在，经过了多少个周期。并按下个周期开始重新定时
			QWORD escape = (QWORD)(((double)((ct.sec() - last_time.sec())/(interval/1000))) + 1);
			last_time.addDelay((escape*interval));
		}
};

