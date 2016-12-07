/*
 * =====================================================================================
 *
 *       Filename:  ActTimer.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年05月04日 23时34分49秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuxin (lx), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "ActTimer.h"
#include "TimeTick.h"

bool ActTimerItem::init(zXMLParser &xml, xmlNodePtr timerNode)
{
	if (!timerNode) return false;

	xml.getNodePropNum(timerNode, "last", &_last, sizeof(_last));
	xml.getNodePropNum(timerNode, "interval", &_interval, sizeof(_interval));
	xml.getNodePropNum(timerNode, "time", &_time, sizeof(_time));


	char tempChar[256];
	bzero(tempChar, sizeof(tempChar));
	xml.getNodePropStr(timerNode, "startTime", tempChar, sizeof(tempChar));

	struct tm tm;
	if (strptime(tempChar,"%Y%m%d %H:%M:%S", &tm)==NULL) return false;
	time_t t = timegm(&tm);
	if (t==(time_t)-1) return false;
	t -= 8*60*60;
	_start = t;

	if (!_interval) _interval = 86400;
	if (!_time) _time = (DWORD)-1;

	DWORD cur = SceneTimeTick::currentTime.sec();
	if ((_start+_last)<=cur)
	{
		DWORD t = (cur-_last-_start)/_interval+1;
		if (_time<t)
			_start = 0;
		else
		{
			_start += _interval*t;
			_time -= t;
		}
	}
	return _start;
}

void ActTimerItem::timer(DWORD cur)
{
	if (!_start) return;

	if (_start && (_start+_last)<=cur)
	{
		if (_inTime) _inTime = 0;

		while (_time && (_start+_last)<=cur)
		{
			_start += _interval;
			_time--;
		}
		if (!_time)
		{
			_start = 0;
		}
	}

	if (!_inTime && _time && _start<=cur)
		_inTime = 1;
}

bool ActTimer::initTimer(zXMLParser &xml, xmlNodePtr node)
{
	if (!node) return false;

	timerList.clear();
	_inTime = 0;

	ActTimerItem ati;
	if (ati.init(xml, node))
		timerList.push_back(ati);

	xmlNodePtr timerNode = xml.getChildNode(node, "timer");
	while (timerNode)
	{
		ActTimerItem i;
		if (i.init(xml, timerNode))
			timerList.push_back(i);

		timerNode = xml.getNextNode(timerNode, "timer");
	}
	return timerList.size();
}

void ActTimer::timer(DWORD cur)
{
	BYTE willStart = 0, willEnd = 0;
	for (std::list<ActTimerItem>::iterator it=timerList.begin(); it!=timerList.end(); it++)
	{
		BYTE oldIn = it->_inTime;

		it->timer(cur);

		if (!oldIn && it->_inTime) willStart = 1;
		if (oldIn && !it->_inTime) willEnd = 1;
	}

	if (willEnd) end();
	if (willStart) start();

	v_timer(cur);
}

void ActTimer::start()
{
	_inTime = 1;
	v_start();
}

void ActTimer::end()
{
	_inTime = 0;
	v_end();
}

DWORD ActTimer::startTime()
{
	DWORD willStart = 0;
	for (std::list<ActTimerItem>::iterator it=timerList.begin(); it!=timerList.end(); it++)
		if (it->_start && (!willStart || it->_start<willStart))
			willStart = it->_start;
	return willStart;
}

DWORD ActTimer::endTime()
{
	DWORD willEnd = 0;
	for (std::list<ActTimerItem>::iterator it=timerList.begin(); it!=timerList.end(); it++)
		if (it->_inTime && (!willEnd || (it->_start+it->_last)<willEnd))
			willEnd = (it->_start+it->_last);
	return willEnd;
}

DWORD ActTimer::finishTime()
{
	DWORD willStart = 0;
	for (std::list<ActTimerItem>::iterator it=timerList.begin(); it!=timerList.end(); it++)
		if (it->_start && (!willStart || it->_start<willStart))
			willStart = it->_start + it->_last;
	return willStart;
}

bool ActTimer::canRandomToday()
{
	if(timerList.size()>0)
	{
		std::list<ActTimerItem>::iterator it=timerList.begin();
		for(;it!=timerList.end();it++)
		{
			DWORD start=it->_start;
			struct tm tv;
			zRTime::getLocalTime(tv,start);
			zTime now;
			if((now.getYear()==(tv.tm_year+1900))&&(now.getYDay()==tv.tm_yday))
			{
				return false;
			}
		}
	}
	return true;
}

void ActTimer::setStartTime(DWORD start)
{
	struct tm tv;
	zRTime::getLocalTime(tv,start);
	int hour=tv.tm_hour;
	int min=tv.tm_min;
	if(timerList.size()>0)
	{
		std::list<ActTimerItem>::iterator it=timerList.begin();
		for(;it!=timerList.end();it++)
		{
			it->_start=start;
			if(it->_time!=(DWORD)-1)
				it->_time++;
			if(hour<=23)
				it->_last=(24-hour-1)*3600+(60-min)*60;
			else
				it->_last=(60-min)*60;
			it->_interval=86400;
		}
	}
	else
	{
		ActTimerItem ati;
		ati._start=start;
		ati._time=1;
		if(hour<=23)
			ati._last=(24-hour-1)*3600+(60-min)*60;
		else
			ati._last=(60-min)*60;
		ati._interval=86400;
		timerList.push_back(ati);
	}
}
