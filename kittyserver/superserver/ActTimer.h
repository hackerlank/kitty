#pragma once
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
		ActTimer():_inTime(0){}
		virtual ~ActTimer(){}

		bool initTimer(zXMLParser &xml, xmlNodePtr node);

		void timer(DWORD cur);
		BYTE inTime() const {return _inTime;}
	protected:
		void start();
		void end();

		DWORD startTime();
		DWORD endTime();

		virtual bool v_initTimer(zXMLParser &xml, xmlNodePtr node){return true;}
		virtual void v_start(){}
		virtual void v_end(){}
		virtual void v_timer(DWORD cur){}

		std::list<ActTimerItem> timerList;
		BYTE _inTime;
};
