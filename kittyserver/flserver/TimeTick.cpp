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
#include "FLServer.h"
#include "Fir.h"
#include "LoginManager.h"

struct LoginTimeout : public LoginManager::LoginTaskCallback
{
	const zTime &ct;
	LoginTimeout(const zTime &ct) : ct(ct) {}
	void exec(LoginTask *lt)
	{
        //超时就注销
        if (lt->timeout(ct))
        {
            lt->Terminate();
        }
	}
};

zRTime FLTimeTick::currentTime;

void FLTimeTick::run()
{
	while(!isFinal())
	{
		zThread::sleep(1);
	
		currentTime.now();

		zTime ct;
		if (ct.sec() % 10 == 0)
		{
            LoginTimeout cb(ct);
			LoginManager::getMe().execAll(cb);
		}
	}
}

