#ifndef _zActTimer_H_
#define _zActTimer_H_

#include "zXMLParser.h"
#include "zSingleton.h"

#include <time.h>

template <int dummy>
class zActTimer 
{
public:
	typedef zActTimer<dummy> self_t;
	static self_t& getMe()
	{
		if(!instance)
			instance = FIR_NEW self_t;
		return *instance;
	}
	{
	}
	/**
	 * \brief 析构函数
	 *
	 */
	~zActTimer()
	{
	}

	/**
	 * \brief 从配置文件进行初始化
	 *
	 */
	bool init(zXMLParser &xml, xmlNodePtr timerNode)
	{
		if(!timerNode)
			return false;

		inited = false;
		startTime = 0;
		lastTime = 0;
		interval = 0;
		lastTime1 = 0;
		
        	xml.getNodePropNum(timerNode, "last", &lastTime, sizeof(lastTime));
        	xml.getNodePropNum(timerNode, "interval", &interval, sizeof(interval));
        	xml.getNodePropNum(timerNode, "last1", &lastTime1, sizeof(lastTime1));

        	char tempChar[256];
        	bzero(tempChar, sizeof(tempChar));
        	xml.getNodePropStr(timerNode, "startTime", tempChar, sizeof(tempChar));

        	struct tm tm;
        	if (strptime(tempChar,"%Y%m%d %H:%M:%S", &tm)==NULL) return false;
        	time_t t = timegm(&tm);
        	if (t==(time_t)-1) return false;
        	t -= 8*60*60;//UTC时间
        	startTime = t;

		if(!interval) interval = 10*365*24*60*60;/*10年,保证无效的时间间隔*/
		inited = true;

		return true;
	}

	/**
	 * \brief 是否在活动期间
	 *
	 */
	bool inTime(time_t _cur) const
	{
		if(!inited)
			return false;
		if(_cur < startTime)
			return false;
		else
		{
			return (_cur-startTime)%(interval) < lastTime;
		}
	}

	/**
	 * \brief 活动是否持续了_long秒(误差为两分钟)
	 * \param cur: 当前时间
	 * \param _long: 活动持续的时间
 	 *
	 */
	bool lasttime(time_t cur, time_t _long)
	{
		if(!inited)
			return false;
		if(cur < startTime)
			return false;
		time_t remain = (cur-startTime)%(interval);
		if(remain >= _long && remain <= _long+120)
			return true;
		else
			return false;
	}

	/**
	 * \brief 当前时间是否是在lasttime1时间点附近
	 *
	 */
	bool lasttime1(time_t cur)
	{
		return lasttime(cur, lastTime1);
	}

	/**
	 * \brief 访问成员变量
	 *
	 */
	time_t getStartTime() const {return startTime; }
	time_t getLastTime() const {return lastTime; }
	time_t getInterval() const {return interval; }
	time_t getLastTime1() const {return lastTime1; }
private:
	/**
	 *a \brief 默认构造
	 *
	 */
	zActTimer() :
		startTime(0), 
		lastTime(0), 
		interval(0), 
		lastTime1(0)
	{
		inited = false;
	}

	/**
	 * \brief 构造函数
	 *
	 */
	zActTimer(time_t startTime , 
			time_t lastTime , 
			time_t interval , 
			time_t lastTime1=0) :
		startTime(startTime), 
		lastTime(lastTime), 
		interval(interval), 
		lastTime1(lastTime1)
	{
		inited = false;
	}


	/**
	 * \brief 第一次活动的开始时间
	 *
 	 */
	time_t startTime;	

	/**
	 * \brief 活动的持续时间
	 * 单位秒
	 *
	 */
	time_t lastTime;	

	/**
	 * \brief 活动的时间间隔
	 *
	 * 两次活动开始的时间差(单位: 秒)
	 */
	time_t interval;

	/**
	 * \brief  活动开始后,lastTime1秒，可能要触发其他的活动
	 *
	 */
	time_t lastTime1;

	/**
	 * \brief 成员变量是否初始化成功
	 *
	 */
	bool inited;

	static self_t* instance;
};
template <int dummy>
zActTimer<dummy>* zActTimer<dummy>::instance = NULL;

#endif
