/**
 * \file
 * \version  $Id: TimeTick.cpp 1071 2005-05-02 10:34:54Z song $
 * \author  ,
 * \date 2005年01月05日 21时03分02秒 CST
 * \brief 时间回调函数
 *
 * 
 */


#include <iostream>
#include <string>
#include <sys/timeb.h>

#include "Fir.h"
#include "zThread.h"
#include "TimeTick.h"
#include "GmToolServer.h"
#include "GmToolManager.h"
#include "ServerManager.h"

struct GmToolTimeout : public GmToolTaskManager::GmToolTaskCallback
{
	const zTime &ct;
	GmToolTimeout(const zTime &ct) : ct(ct) {}
	void exec(GmToolTask *lt)
	{
        //超时就注销
        if (lt->timeout(ct))
        {
            lt->Terminate();
        }
	}
};

zRTime GmToolTimeTick::currentTime;
void GmToolTimeTick::run()
{
    while(!isFinal())
	{
        zThread::msleep(100);

#if 0 
		currentTime.now();
		zTime ct;
		if (ct.sec() % 2 == 0)
		{
            GmToolTimeout cb(ct);
			GmToolTaskManager::getMe().execAll(cb);
		}
        //每隔15秒存一下档
        if(ct.sec() % 15 == 0)
        {
            GmToolTaskManager::getMe().update();
        }
#endif 
	}
}

