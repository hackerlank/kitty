#include "ActTimer.h"
#include "TimeTick.h"

/******************************************************************************************/

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
	//t -= 8*60*60;//UTC时间
	_start = t;

	if (!_interval) _interval = 86400;
	if (!_time) _time = (DWORD)-1;

	DWORD cur = SuperTimeTick::currentTime.sec();
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
#ifdef _XWL_DEBUG
	//Fir::logger->debug("ActTimerItem _inTime=%u _start=%u _last=%u _interval=%u _time=%u cur=%u", _inTime, _start, _last, _interval, _time, cur);
#endif
}

/******************************************************************************************/

bool ActTimer::initTimer(zXMLParser &xml, xmlNodePtr node)
{
	if (!node) return false;

	timerList.clear();

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
