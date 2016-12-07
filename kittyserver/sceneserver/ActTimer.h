/*
 * =====================================================================================
 *
 *       Filename:  ActTimer.h
 *
 *    Description:  活动时间管理
 *
 *        Version:  1.0
 *        Created:  2013年05月04日 23时33分31秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuxin (lx), 
 *        Company:  
 *
 * =====================================================================================
 */


#ifndef _ACT_TIMER_H
#define _ACT_TIMER_H

#include "zType.h"
#include "zXMLParser.h"
#include <list>

struct ActTimerItem
{
	ActTimerItem():_start(0),_last(0),_interval(0),_time(0),_inTime(0){}
	bool init(zXMLParser &xml, xmlNodePtr node);
	void timer(DWORD cur);

	DWORD _start, _last, _interval, _time;
	BYTE _inTime;
};

class ActTimer
{
	public:
		ActTimer():_inTime(0){};
		virtual ~ActTimer(){}

		bool initTimer(zXMLParser &xml, xmlNodePtr node);

		void timer(DWORD cur);
		BYTE inTime() const {return _inTime;}
		bool canRandomToday();
		void setStartTime(DWORD start);
		DWORD startTime();
		DWORD endTime();
		DWORD finishTime();
	protected:
		void start();
		void end();
		virtual bool v_initTimer(zXMLParser &xml, xmlNodePtr node){return true;}
		virtual void v_start(){}
		virtual void v_end(){}
		virtual void v_timer(DWORD cur){}

		std::list<ActTimerItem> timerList;
		BYTE _inTime;
};

#endif
