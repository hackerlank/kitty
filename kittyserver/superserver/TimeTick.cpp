/**
 * \file
 * \version  $Id: TimeTick.cpp 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
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
#include "SuperServer.h"
#include "Fir.h"
#include "zDBConnPool.h"
#include "SuperCommand.h"
#include "ServerManager.h"
#include "FLClient.h"
#include "FLClientManager.h"
#include "TimerMgr.h"
#include "extractProtoMsg.h"
#include "GmToolClientManager.h"
#include "ResourceClientManager.h"

zRTime SuperTimeTick::currentTime;
SuperTimeTick *SuperTimeTick::instance = NULL;

void SuperTimeTick::run()
{
#if 0
	readTime();
#endif
	const int timeout_value = 500;

	while(!isFinal())
	{
		zThread::msleep(50);
		currentTime.now();
        ServerManager::getMe().execEvery();
		if(one_sec(currentTime))
		{
            qwStartGameTime = currentTime.sec();
            Fir::qwGameTime = qwStartGameTime; 
            Fir::qwStartTime = Fir::qwGameTime;
#if 0
			Fir::qwGameTime = qwStartGameTime + startTime.elapse(currentTime) / 1000;
			if (0 != Fir::qwGameTime && 0 == Fir::qwGameTime % 600)
			{
				saveTime();
			}
#endif
#if 0
			TimerMgr::getMe().Update();
#endif
			zTime ct;
			FLClientManager::getMe().timeAction(ct);
            GmToolClientManager::getMe().timeAction(ct);
            ResourceClientManager::getMe().timeAction(ct);
			ServerManager::getMe().checkDeadLock(currentTime.sec());
#if 0
            CMD::SUPER::t_GameTime tCmd;
			tCmd.qwGameTime = Fir::qwGameTime;
			tCmd.qwStartTime = Fir::qwStartTime;
            std::string ret;
            encodeMessage(&tCmd,sizeof(tCmd),ret);
			ServerManager::getMe().broadcast(ret.c_str(),ret.size());
#endif
		}
        //统计在线人数
        if(five_Min(currentTime))
        {
            ServerManager::getMe().caculateOnlineNum();
        }

		zRTime e;
		int t = currentTime.elapse(e);
		if (t > timeout_value)
		{
			Fir::logger->debug("[SUPRE TIMETICK超时],0,0,0,处理时间过长 %u",t);
		}
		
	}
#if 0
	saveTime();
#endif
}

static const dbCol gametime_define[] =
{
	{"`GAMETIME`",zDBConnPool::DB_QWORD,sizeof(QWORD)},
	{NULL, 0, 0}
};

bool SuperTimeTick::readTime()
{
	connHandleID handle = SuperService::dbConnPool->getHandle();
	if ((connHandleID)-1 == handle)
	{
		Fir::logger->error("不能从数据库连接池获取连接句柄");
		return false;
	}

	unsigned int retcode = SuperService::dbConnPool->exeSelectLimit(handle, "`GAMETIME`", gametime_define, NULL, NULL, 1, (unsigned char *)(&qwStartGameTime));
	if ((unsigned int)-1 == retcode)
	{
		SuperService::dbConnPool->putHandle(handle);
		Fir::logger->error("读取游戏时间错误");
		return false;
	}

	if (0 == retcode)
	{
		qwStartGameTime = 0;
		retcode = SuperService::dbConnPool->exeInsert(handle, "`GAMETIME`", gametime_define, (const unsigned char *)(&qwStartGameTime));
		if ((unsigned int)-1 == retcode)
		{
			SuperService::dbConnPool->putHandle(handle);
			Fir::logger->error("初始化游戏时间错误");
			return false;
		}
	}
	SuperService::dbConnPool->putHandle(handle);

	Fir::logger->debug("读取游戏时间成功");
	Fir::qwGameTime = qwStartGameTime;
	startTime.now();

	return true;
}

bool SuperTimeTick::saveTime()
{
	connHandleID handle = SuperService::dbConnPool->getHandle();
	if ((connHandleID)-1 == handle)
	{
		Fir::logger->error("不能从数据库连接池获取连接句柄");
		return false;
	}

	unsigned int retcode = SuperService::dbConnPool->exeUpdate(handle, "`GAMETIME`", gametime_define, (BYTE *)(&Fir::qwGameTime), NULL);
	SuperService::dbConnPool->putHandle(handle);

	if (1 == retcode)
	{
		Fir::logger->debug("保存游戏时间成功");
	}
	else
	{
		Fir::logger->error("保存游戏时间失败");
	}

	return true;
}

