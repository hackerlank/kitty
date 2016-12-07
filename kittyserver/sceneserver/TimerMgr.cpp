#include "TimerMgr.h"

#include <list>
#include "zTime.h"
#include "TimeTick.h"

TimerMgr::TimerMgr() 
: m_dwCurSeq(1)
{
    m_tCurTime = SceneTimeTick::currentTime.local_sec();
}

void TimerMgr::Update()
{
	m_tCurTime = SceneTimeTick::currentTime.local_sec();

    std::list<int> lstDelTimers;
    std::list<int> lstTimers;

	for(auto itr=m_mapTimerItems.begin();itr!=m_mapTimerItems.end();itr++)
    {
        lstTimers.push_back(itr->first);
    }

    for(auto itr=lstTimers.begin(); itr!=lstTimers.end();itr++)
    {
    	const int nTimerId = *itr;
        TimerItemMap::iterator itrFind = m_mapTimerItems.find(nTimerId);
        if (itrFind != m_mapTimerItems.end())
        {
            S_TimerItem& sTimerItem = itrFind->second;

            //std::string strfunc = sTimerItem.m_strFunc;

            if( m_tCurTime >= sTimerItem.m_timer)
            {
                if (sTimerItem.m_bCircle)
                {
                    sTimerItem.m_timer += sTimerItem.m_dwInterval;
                    if(sTimerItem.m_timer > sTimerItem.m_end_time)
					{
						lstDelTimers.push_back(itrFind->first);
					}
                }
                else
                {
                    lstDelTimers.push_back(itrFind->first);
                }

                sTimerItem.m_func();
            }
        }
    }

    for(auto itr=lstDelTimers.begin();itr!=lstDelTimers.end();itr++)
    {
    	const int nTimerId = *itr;
        m_mapTimerItems.erase(nTimerId);
    }
}

void TimerMgr::CancelTimer(DWORD iTimer)
{
    m_mapTimerItems.erase(iTimer);
}

/*
DWORD TimerMgr::AddinDayStaticTimer(std::function<void (time_t)> triggerFunc, DWORD tTrigger, std::string strfunc)
{
    time_t tStart = DayStartOfTime(m_tCurTime) + tTrigger;

    if (tStart < m_tCurTime)
    {
        tStart += DAY;
    }

    return AddCircleTimer(tStart, triggerFunc, DAY, strfunc);
}

DWORD TimerMgr::AddHourStaticTimer(std::function<void (time_t)> triggerFunc, DWORD tTrigger, std::string strfunc)
{
    time_t tStart = HourStartOfTime(m_tCurTime) + tTrigger;

    if (tStart < m_tCurTime)
    {
        tStart += HOUR;
    }

    return AddCircleTimer(tStart, triggerFunc, HOUR, strfunc);
}
*/
time_t TimerMgr::default_end_time()
{
	return SceneTimeTick::currentTime.local_sec() + 15 * 365 * 24 * 3600; 
}

DWORD TimerMgr::AddOnceTimer(time_t tTrigger,TimerFunction func)
{
    m_tCurTime = SceneTimeTick::currentTime.local_sec();

    if(m_tCurTime > tTrigger)
	{
		return 0;
	}
    
    S_TimerItem timer;
    timer.m_func = func;
    timer.m_timer = tTrigger;
    timer.m_bCircle = false;
    timer.m_dwInterval = 0;
    timer.m_dwSeq = m_dwCurSeq;
	timer.m_end_time = default_end_time();

    m_mapTimerItems[m_dwCurSeq] = timer;

    return m_dwCurSeq++;
}

DWORD TimerMgr::AddOnceTimerFromNow(time_t tTrigger, TimerFunction func)
{
    tTrigger += m_tCurTime;
    return AddOnceTimer(tTrigger, func);
}

DWORD TimerMgr::AddCircleTimer(time_t tStart,TimerFunction func, DWORD dwInterval)
{
	m_tCurTime = SceneTimeTick::currentTime.local_sec();

	while(true)
	{
		if(tStart > m_tCurTime)
			break;

		tStart += dwInterval;
	}

    S_TimerItem timer;
    timer.m_timer = tStart;
    timer.m_bCircle = true;
    timer.m_dwInterval = dwInterval;
    timer.m_dwSeq = m_dwCurSeq;
    timer.m_end_time = default_end_time();
	timer.m_func = func;

    m_mapTimerItems[m_dwCurSeq] = timer;

    return m_dwCurSeq++;
}

DWORD TimerMgr::AddCircleTimer(time_t tStart, time_t tEnd, TimerFunction func, DWORD dwInterval)
{
	m_tCurTime = SceneTimeTick::currentTime.local_sec();

	while(true)
	{   
		if(tStart > m_tCurTime)
			break;

		tStart += dwInterval;
	}   

	if(tStart > tEnd)
	{
		return 0;
	}

	S_TimerItem timer;
	timer.m_timer = tStart;
	timer.m_bCircle = true;
	timer.m_dwInterval = dwInterval;
	timer.m_dwSeq = m_dwCurSeq;
	timer.m_end_time = tEnd;
	timer.m_func = func;

	m_mapTimerItems[m_dwCurSeq] = timer;

	return m_dwCurSeq++;
}

