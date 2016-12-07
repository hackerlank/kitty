#pragma once

#include <time.h>
#include <utility>
#include <map>
#include <functional>
#include <unordered_map>
#include "Fir.h"
#include "zSingleton.h"

struct StaticTimer
{
	DWORD					m_timer;
	bool					m_bPass;
   	std::function<void ()>	m_func;	
};

#define MINUTE	60
#define HOUR	(60*60)
#define	DAY		(24*HOUR)
#define TEN_SECOND 10
#define TWENTY_SECOND (2 * TEN_SECOND)

// 整点触发器实现
class TimerMgr : public Singleton<TimerMgr>
{
	friend class Singleton<TimerMgr>;
public:
	virtual ~TimerMgr(){};

	TimerMgr();

public:
    void Update();
    void CancelTimer(DWORD iTimer);
	void CancelAllTimers();

protected:
    DWORD		m_dwCurSeq;
	time_t  m_tCurTime;

protected:
    typedef std::function<void ()> TimerFunction;

    struct S_TimerItem
    {
        DWORD         m_dwSeq;
        time_t	       m_timer;	
        TimerFunction  m_func;
        bool           m_bCircle;
        DWORD         m_dwInterval;
        time_t        m_end_time; // 截止时间

        S_TimerItem()
        {
            m_timer = 0;
            m_bCircle = false;
            m_dwInterval = 0;
            m_dwSeq = 0;
           	m_end_time = 0;
        }
    };

    typedef std::map<int, S_TimerItem> TimerItemMap;

    TimerItemMap m_mapTimerItems;
    time_t default_end_time();
public:
    DWORD AddOnceTimer(time_t tTrigger,TimerFunction func);

    DWORD AddOnceTimerFromNow(time_t tTrigger, TimerFunction func);

    DWORD AddCircleTimer(time_t tStart,TimerFunction func, DWORD dwInterval);
    DWORD AddCircleTimer(time_t tStart, time_t tEnd, TimerFunction func, DWORD dwInterval);	
    DWORD AddCircleTimerFromNow(TimerFunction func, DWORD dwInterval);
};
