#include <iostream>
#include <string>

#include "Fir.h"
#include "zThread.h"
#include "zTime.h"
#include "TimeTick.h"
#include "GatewayServer.h"
#include "GatewayTaskManager.h"
#include "SceneClientManager.h"

zRTime GatewayTimeTick::currentTime;

struct GatewayTaskCheckTime : public GatewayTaskManager::GatewayTaskCallback
{
	bool exec(GatewayTask *gt)
	{
		return gt->checkTime(GatewayTimeTick::currentTime);
	}
};

/**
 * \brief 线程主函数
 *
 */
void GatewayTimeTick::run()
{
	while(!isFinal())
	{
		zThread::sleep(1);

		//获取当前时间
		currentTime.now();

		if (one_second(currentTime) ) 
		{
			
			if (five_second(currentTime))
			{
			}
            GateUserManager::getMe().syncGameTime();
			GatewayTaskManager::getMe().execAllCheckTime();
		}
        
        zTime ct;
        SceneClientManager::getMe().timeAction(ct);
	}
}

