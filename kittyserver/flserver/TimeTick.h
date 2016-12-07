/**
 * \file
 * \version  $Id: TimeTick.h 855 2005-04-04 13:53:18Z song $
 * \author  ,
 * \date 2005年01月05日 21时03分02秒 CST
 * \brief 时间回调函数
 *
 * 
 */


#ifndef _TimeTick_h_
#define _TimeTick_h_

#include <iostream>
#include <string>
#include <sys/timeb.h>

#include "Fir.h"
#include "zThread.h"
#include "zTime.h"
#include "zMisc.h"

class FLTimeTick : public Singleton<FLTimeTick>, public zThread
{

	public:

	void run();

	static zRTime currentTime;
	private:

		friend class Singleton<FLTimeTick>;
		FLTimeTick() : zThread("TimeTick") {};
		~FLTimeTick() {};

};

#endif

