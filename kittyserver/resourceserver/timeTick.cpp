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
#include "timeTick.h"
#include "resourceServer.h"
#include "resourceTaskManager.h"
#include "serverManager.h"

struct ResourceTimeout : public ResourceTaskManager::ResourceTaskCallback
{
	const zTime &ct;
	ResourceTimeout(const zTime &ct) : ct(ct) {}
	void exec(ResourceTask *lt)
	{
        //超时就注销
        if (lt->timeout(ct))
        {
            lt->Terminate();
        }
	}
};

zRTime ResourceTimeTick::currentTime;
void ResourceTimeTick::run()
{
    while(!isFinal()) 
    {
        zThread::msleep(100);

        //TODO:
    }
}

