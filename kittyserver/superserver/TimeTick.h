/**
 * \file
 * \version  $Id: TimeTick.h 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
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

class SuperTimeTick : public zThread, public Singleton<SuperTimeTick>
{
	friend class Singleton<SuperTimeTick>;
	public:

		~SuperTimeTick(){};

		void run();

		static zRTime currentTime;
	private:

		static SuperTimeTick *instance;

		zRTime startTime;
		Timer one_sec;
        Timer five_Min;
		QWORD qwStartGameTime;
		
		SuperTimeTick() : zThread("TimeTick"), startTime(),one_sec(1),five_Min(5*60)
		{
			qwStartGameTime = 0;
		}

		bool readTime();
		bool saveTime();

};

#endif

