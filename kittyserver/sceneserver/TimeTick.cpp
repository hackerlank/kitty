/**
 * \file
 * \version  $Id: TimeTick.cpp 67 2013-04-23 09:44:20Z  $
 * \author  ,
 * \date 2013年04月05日 21时03分02秒 CST
 * \brief 时间回调函数
 *
 * 
 */


#include <iostream>
#include <string>

#include "zThread.h"
#include "TimeTick.h"
#include "SceneServer.h"
#include "SceneUserManager.h"
#include "SceneTaskManager.h"
#include "RecordClient.h"
#include "TimerMgr.h"
#include "zTime.h"
#include "SceneToOtherManager.h"
#include "RecordClient.h"
#include "PlayerActiveConfig.h"

SceneTimeTick *SceneTimeTick::instance = NULL;
zRTime SceneTimeTick::currentTime;

WORD SceneTimeTick::secCountNum = 0;
SceneTimeTick::SceneTimeTick() :
	zThread("TimeTick")
	,_1_min(60)
	,_1_sec(1)
	,_1_card_sec(1,1)
	,_1_hour(3600,3600)
{
}
void SceneTimeTick::run()
{
	const int timeout_value = 500;
	int t = 0;
	while(!isFinal())
	{
		zThread::msleep((10-t)>0?(10-t):1);
		currentTime.now();
        
        //这个要在场景消息解析前
		if (_1_card_sec(currentTime))
		{
			secCountNum = 0;
		}

        SceneService::getMe().superCmd.doCmd();
        SceneTaskManager::getMe().execEvery();
        SceneClientToOtherManager::getMe().execEvery();
        MgrrecordClient.execEvery();

		if(_1_sec(currentTime))
		{
            SceneUserManager::getMe().loop();
            SceneUserManager::getMe().update();
			TimerMgr::getMe().Update();
            
            //判断是否0点
            struct tm tm_now;
            zRTime::getLocalTime(tm_now,currentTime.sec());
            if(tm_now.tm_hour == 0 && tm_now.tm_min == 0 && tm_now.tm_sec == 0)
            {
                SceneUserManager::getMe().oneDay();
            }
		}
        if(_1_min(currentTime))
        {
            PlayerActiveConfig::getMe().timerCheck();
        }
        if(_1_hour(currentTime))
        {
            SceneUserManager::getMe().oneHour();
        }
		zRTime e;
		t = currentTime.elapse(e);
		if (t > timeout_value)
		{
			Fir::logger->debug("[场景TIMETICK超时],0,0,0,处理时间过长 %u",t);
		}
        zTime ct;
        SceneClientToOtherManager::getMe().timeAction(ct);

	}
}


void My_Address_wrapper::showLog()
{
}

#if 0

#if defined (__MY_FUNCTIONTIME_WRAPPER__)
My_FunctionTime My_FunctionTime_wrapper::my_func(600);
#endif

#endif
