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
#include "robotServer.h"
#include "Fir.h"
#include "LoadClientManager.h"
zRTime RobotTimeTick::currentTime;

void RobotTimeTick::run()
{
	while(!isFinal())
	{
		zThread::sleep(1);
	
		currentTime.now();

		zTime ct;
		if (ct.sec() % 10 == 0)
		{
			LoadClientManager::getMe().timeAction();
		}
	}
}

