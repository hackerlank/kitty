#include "Counter.h"
#include <cstring>
#include "../base/zTime.h"

Counter::Counter()//默认为NORMAL
	:dataSize(0)
{
	memset(data, 0, sizeof(data));
//	load();
}

Counter::~Counter()
{
//	save();
}

void Counter::setType(const DWORD& item, CounterType type)
{
	data[item].type = type;
}

Counter::CounterType Counter::getType(const DWORD& item)
{
	return data[item].type;
}

QWORD Counter::getCounter(const DWORD& item)
{
	if( needClear(item) )
		clearCounter( item );
	return data[item].value;
}

void Counter::clearCounter(const DWORD& item)
{
	data[item].value = 0;
	data[item].date = TimeTool::getCurTimeSec();
}

void Counter::addCounter(const DWORD& item, const QWORD& num)
{
	if( needClear(item) )
		clearCounter(item);
	
	data[item].value += num;
	data[item].date = TimeTool::getCurTimeSec();
}

void Counter::setCounter(const DWORD& item, const QWORD& value)
{
	data[item].value = value;
	data[item].date = TimeTool::getCurTimeSec();
}

bool Counter::needClear(const DWORD& item)
{
	CounterType type = data[item].type;
	const DWORD date = data[item].date;
	const DWORD now = TimeTool::getCurTimeSec();

	bool retVal = false;
	if( type == YEAR )
	{
		retVal = !TimeTool::isSameYear(now, date);
	}
	if( type == MONTH )
	{
		retVal = !TimeTool::isSameMonth(now, date);
	}
	if( type == WEEK )
	{
		retVal = !TimeTool::isSameWeek(now, date);
	}
	if( type == DAY )
	{
		retVal = !TimeTool::isSameDay(now, date);
	}
	return retVal;
}



/************************************************************/
/***********************TimeTool开始*************************/
/************************************************************/

DWORD TimeTool::getCurTimeSec()
{
	return zRTime().sec();
}

DWORD TimeTool::getWeekDay(const DWORD& t/* = 0*/) //默认参数0表示当前时间 ，返回0表示周日
{
	DWORD timeSec = t ? t : TimeTool::getCurTimeSec();
	struct tm tm1;
	zRTime::getLocalTime(tm1, static_cast<time_t>(timeSec));
	return tm1.tm_wday;
}

bool TimeTool::isLeapYear(const DWORD& year)
{
	return (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0));
}

bool TimeTool::isSameYear(const DWORD& t1, const DWORD& t2)
{
	struct tm tm1,tm2;
	zRTime::getLocalTime(tm1, static_cast<time_t>(t1));
	zRTime::getLocalTime(tm2, static_cast<time_t>(t2));
	return tm1.tm_year == tm2.tm_year;
}

bool TimeTool::isSameMonth(const DWORD& t1, const DWORD& t2)
{
	struct tm tm1, tm2;
	zRTime::getLocalTime(tm1, static_cast<time_t>(t1));
	zRTime::getLocalTime(tm2, static_cast<time_t>(t2));
	return tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon;
}

bool TimeTool::isSameWeek(const DWORD& t1_, const DWORD& t2_)//周一为每周第一天
{
	DWORD t1 = t1_, t2 = t2_;
	if( t2 > t1 )//保证 t2 <= t1
	{
		DWORD t = t1;
		t1 = t2;
		t2 = t;
	}
	struct tm tm1, tm2;
	zRTime::getLocalTime(tm1, static_cast<time_t>(t1));
	zRTime::getLocalTime(tm2, static_cast<time_t>(t2));
	if( !tm1.tm_wday ) 
		tm1.tm_wday = 7;
	if( !tm2.tm_wday ) 
		tm2.tm_wday = 7;
	
	if( tm1.tm_year == tm2.tm_year )//同一年
	{
		return tm1.tm_yday - tm2.tm_yday < 7 && tm1.tm_wday >= tm2.tm_wday;
	}

	if( tm1.tm_year - tm2.tm_year == 1 )//相连的两年
	{
		if( isLeapYear(tm2.tm_year + 1900) )
		{
			DWORD ydayRemain = 366 - tm2.tm_yday;
			return ydayRemain + tm1.tm_yday < 7 && tm1.tm_wday >= tm2.tm_wday;
		}
		else
		{
			DWORD ydayRemain = 365 - tm2.tm_yday;
			return ydayRemain + tm1.tm_yday < 7 && tm1.tm_wday >= tm2.tm_wday;
		}
	}
	//相隔超过1年
	return false;
}

bool TimeTool::isSameDay(const DWORD& t1, const DWORD& t2)
{
	struct tm tm1, tm2;
	zRTime::getLocalTime(tm1, static_cast<time_t>(t1));
	zRTime::getLocalTime(tm2, static_cast<time_t>(t2));
	return tm1.tm_year == tm2.tm_year && tm1.tm_yday == tm2.tm_yday;
}

bool TimeTool::isSameHour(const DWORD& t1, const DWORD& t2)
{
	struct tm tm1, tm2;
	zRTime::getLocalTime(tm1, static_cast<time_t>(t1));
	zRTime::getLocalTime(tm2, static_cast<time_t>(t2));
	return tm1.tm_year == tm2.tm_year && tm1.tm_yday == tm2.tm_yday && tm1.tm_hour == tm2.tm_hour;
}
