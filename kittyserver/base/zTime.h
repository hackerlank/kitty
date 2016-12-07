/**
 * \file
 * \version  $Id: zTime.h 13 2013-03-20 02:35:18Z  $
 * \author  ,okyhc@263.sina.com
 * \date 2004年11月23日 16时42分46秒 CST
 * \brief 时间定义
 *
 * 
 */

#ifndef _ZTIME_H_
#define _ZTIME_H_

#include <time.h>
#include <sys/time.h>
#include <sstream>
#include <map>
#include <assert.h>

#include "zType.h"
#include "zMutex.h"

#define DAY_SECS (24 * 60 * 60)

/**
 * \brief 真实时间类,对timespec结构简单封装,提供一些常用时间函数
 * 时间精度精确到毫秒，
 * 关于timespec请man clock_gettime
 */
class zRTime
{

	private:

		/**
		 * \brief 真实时间换算为毫秒
		 *
		 */
		unsigned long long _msecs;
        //时间差
        unsigned long long _grap;
		/**
		 * \brief 得到当前真实时间
		 *
		 * \return 真实时间，单位毫秒
		 */
		unsigned long long _now()
		{
			unsigned long long retval = 0LL;
			struct timespec tv;
			clock_gettime(CLOCK_REALTIME, &tv);
			retval = tv.tv_sec;
			retval *= 1000LL;
			retval += (tv.tv_nsec / 1000000L);
			return retval;
		}

		/**
		 * \brief 得到当前真实时间延迟后的时间
		 * \param delay 延迟，可以为负数，单位毫秒
		 */
		void nowByDelay(int delay)
		{
			_msecs = _now();
			addDelay(delay);
		}

	public:

		/**
		 * \brief 构造函数
		 *
		 * \param delay 相对于现在时间的延时，单位毫秒
		 */
		zRTime(const int delay = 0)
		{
            _grap = 0;
			nowByDelay(delay);
		}

		/**
		 * \brief 拷贝构造函数
		 *
		 * \param rt 拷贝的引用
		 */
		zRTime(const zRTime &rt)
		{
            _grap = 0;
			_msecs = rt._msecs;
		}

		explicit zRTime(const char* stime)//用字符串初始化一个时间，格式:19010214 00:00:00)
		{
            _grap = 0;
			struct tm tv;
  		    struct timespec ts;
			time_t theTime = 0;

           // strptime(stime, "%Y%m%d %H:%M:%S", &tv);
			strptime(stime, "%Y%m%d%H%M%S", &tv);

			theTime = timegm(&tv);
			ts.tv_nsec = (theTime+my_timezone) * 1000000000L; // 减去了8个小时
			ts.tv_sec = (ts.tv_nsec/1000000000L);
			ts.tv_nsec %=  1000000000L;

			_msecs = ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
		}

		/**
		 * \brief 获取当前时间
		 *
		 */
		void now()
		{
			_msecs = _now() + _grap;
		}

#define _check_ok_time(time) \
		unsigned long long t = time; \
		t -= zRTime::my_timezone*1000L; 

		/**
		 * \brief 返回秒数
		 *
		 * \return 秒数
		 */
		unsigned long sec() const
		{
//			_check_ok_time(_msecs);
			return _msecs / 1000L;
		}

		unsigned long local_sec()
		{
			_check_ok_time(_msecs);
			return t / 1000L;
		}
		
		std::string toString(const char* format)
		{
			time_t tnow = sec();
			tm tmNow;
			getLocalTime(tmNow,tnow);
			char szBuf[100];
			bzero(szBuf,sizeof(szBuf));
			strftime(szBuf,100,format,&tmNow);
			return std::string(szBuf);
		}

		std::string toString()
		{
			return toString("%Y-%m-%d %H:%M:%S");
		}

		static std::string sToString(time_t tnow)
		{
			tm tmNow;
			getLocalTime(tmNow,tnow);
			char szBuf[100];
			bzero(szBuf,sizeof(szBuf));
			strftime(szBuf,100,"%Y-%m-%d %H:%M:%S",&tmNow);
			return std::string(szBuf);
		}

		std::string toNumber()
		{
			return toString("%Y%m%d");
		}


		/**
		 * \brief 返回毫秒数
		 *
		 * \return 毫秒数
		 */
		unsigned long msec() const
		{
//			_check_ok_time(_msecs);
			return _msecs % 1000L;
		}

		/**
		 * \brief 返回总共的毫秒数
		 *
		 * \return 总共的毫秒数
		 */
		unsigned long long msecs() const
		{
//			_check_ok_time(_msecs);
			return _msecs;
		}

		unsigned long long local_msecs() const
		{
			_check_ok_time(_msecs);
			return t;
		}

		/**
		 * \brief 返回总共的毫秒数
		 *
		 * \return 总共的毫秒数
		 */
		void setmsecs(unsigned long long data)
		{
			_msecs = data;
		}
        
        //设置差距
        void setgrap(unsigned long long data)
		{
			_grap += data;
		}
		/**
		 * \brief 加延迟偏移量
		 *
		 * \param delay 延迟，可以为负数，单位毫秒
		 */
		void addDelay(int delay)
		{
			_msecs += delay;
		}

		/**
		 * \brief 重载=运算符号
		 *
		 * \param rt 拷贝的引用
		 * \return 自身引用
		 */
		zRTime & operator= (const zRTime &rt)
		{
			_msecs = rt._msecs;
			return *this;
		}

		/**
		 * \brief 重构+操作符
		 *
		 */
		const zRTime & operator+ (const zRTime &rt)
		{
			_msecs += rt._msecs;
			return *this;
		}

		/**
		 * \brief 重构-操作符
		 *
		 */
		const zRTime & operator- (const zRTime &rt)
		{
			_msecs -= rt._msecs;
			return *this;
		}

		/**
		 * \brief 重构>操作符，比较zRTime结构大小
		 *
		 */
		bool operator > (const zRTime &rt) const
		{
			return _msecs > rt._msecs;
		}

		/**
		 * \brief 重构>=操作符，比较zRTime结构大小
		 *
		 */
		bool operator >= (const zRTime &rt) const
		{
			return _msecs >= rt._msecs;
		}

		/**
		 * \brief 重构<操作符，比较zRTime结构大小
		 *
		 */
		bool operator < (const zRTime &rt) const
		{
			return _msecs < rt._msecs;
		}

		/**
		 * \brief 重构<=操作符，比较zRTime结构大小
		 *
		 */
		bool operator <= (const zRTime &rt) const
		{
			return _msecs <= rt._msecs;
		}

		/**
		 * \brief 重构==操作符，比较zRTime结构是否相等
		 *
		 */
		bool operator == (const zRTime &rt) const
		{
			return _msecs == rt._msecs;
		}

		/**
		 * \brief 计时器消逝的时间，单位毫秒
		 * \param rt 当前时间
		 * \return 计时器消逝的时间，单位毫秒
		 */
		unsigned long long elapse(const zRTime &rt) const
		{
			if (rt._msecs > _msecs)
				return (rt._msecs - _msecs);
			else
				return 0LL;
		}

		/**
		 * 保留的时区信息
		 * 程序初始化的时候设置，运行过程中一般不变
		 */
		static long my_timezone;
		static const std::string & getLocalTZ();
		static void save_timezone(std::string &tzstr);
		static void restore_timezone(const std::string &tzstr);
		static void getLocalTime(struct tm & tv1, time_t timValue)
		{
			timValue -=my_timezone;
			gmtime_r(&timValue, &tv1);
		}
		static DWORD getLocalDay(time_t timValue)
		{
			return (timValue - my_timezone)/DAY_SECS;
		}
		static bool isSameLocalDay(time_t oldTime,time_t newTime)
		{
			return (oldTime - my_timezone)/DAY_SECS == (newTime - my_timezone)/DAY_SECS;
		}
		static DWORD getSecsOfDay(time_t timValue)
		{
			return (timValue - my_timezone)%DAY_SECS;
		}
		
		static bool isInWeekDay(int _day)
		{       
			time_t now = time(NULL);
			struct tm tv;
			zRTime::getLocalTime(tv, now);
			return (1<<(tv.tm_wday%7))&_day;
		}   

};

/**
 * \brief 时间类,对struct tm结构简单封装
 */

class zTime
{

	public:

		/**
		 * \brief 构造函数
		 */
		zTime()
		{
			time(&secs);
			zRTime::getLocalTime(tv, secs);
		}
		
		/**
		 * \brief 拷贝构造函数
		 */
		zTime(const zTime &ct)
		{
			secs = ct.secs;
			zRTime::getLocalTime(tv, secs);
		}

		/**
		 *\brief 根据指定时间初始化
		 */
		zTime(time_t _secs)
		{
			secs = _secs;
			zRTime::getLocalTime(tv, secs);
		}

		/**
		 * \brief 获取当前时间
		 */
		void now()
		{
			time(&secs);
			zRTime::getLocalTime(tv, secs);
		}

		/**
		 * \brief 返回存储的时间
		 * \return 时间，秒
		 */
		time_t sec() const
		{
			return secs;
		}

		/**
		 * \brief 重载=运算符号
		 * \param rt 拷贝的引用
		 * \return 自身引用
		 */
		zTime & operator= (const zTime &rt)
		{
			secs = rt.secs;
			zRTime::getLocalTime(tv, secs);
			return *this;
		}

		/**
		 * \brief 重构+操作符
		 */
		const zTime & operator+ (const zTime &rt)
		{
			secs += rt.secs;
			return *this;
		}

		/**
		 * \brief 重构-操作符
		 */
		const zTime & operator- (const zTime &rt)
		{
			secs -= rt.secs;
			return *this;
		}

		/**
		 * \brief 重构-操作符
		 */
		const zTime & operator-= (const time_t s)
		{
			secs -= s;
			return *this;
		}

		/**
		 * \brief 重构>操作符，比较zTime结构大小
		 */
		bool operator > (const zTime &rt) const
		{
			return secs > rt.secs;
		}

		/**
		 * \brief 重构>=操作符，比较zTime结构大小
		 */
		bool operator >= (const zTime &rt) const
		{
			return secs >= rt.secs;
		}

		/**
		 * \brief 重构<操作符，比较zTime结构大小
		 */
		bool operator < (const zTime &rt) const
		{
			return secs < rt.secs;
		}

		/**
		 * \brief 重构<=操作符，比较zTime结构大小
		 */
		bool operator <= (const zTime &rt) const
		{
			return secs <= rt.secs;
		}

		/**
		 * \brief 重构==操作符，比较zTime结构是否相等
		 */
		bool operator == (const zTime &rt) const
		{
			return secs == rt.secs;
		}

		/**
		 * \brief 计时器消逝的时间，单位秒
		 * \param rt 当前时间
		 * \return 计时器消逝的时间，单位秒
		 */
		time_t elapse(const zTime &rt) const
		{
			if (rt.secs > secs)
				return (rt.secs - secs);
			else
				return 0;
		}

		/**
		 * \brief 计时器消逝的时间，单位秒
		 * \return 计时器消逝的时间，单位秒
		 */
		time_t elapse() const
		{
			zTime rt;
			return (rt.secs - secs);
		}

		/**
		 * \brief 得到当前分钟，范围0-59点
		 *
		 * \return 
		 */
		int getSec()
		{
			return tv.tm_sec;
		}
	
		/**
		 * \brief 得到当前分钟，范围0-59点
		 *
		 * \return 
		 */
		int getMin()
		{
			return tv.tm_min;
		}
		
		/**
		 * \brief 得到当前小时，范围0-23点
		 *
		 * \return 
		 */
		int getHour()
		{
			return tv.tm_hour;
		}
		
		/**
		 * \brief 得到天数，范围1-31
		 *
		 * \return 
		 */
		int getMDay()
		{
			return tv.tm_mday;
		}

		/**
		 * \brief 得到当前星期几，范围1-7
		 *
		 * \return 
		 */
		int getWDay()
		{
			return tv.tm_wday;
		}

		int getYDay()
		{
			return tv.tm_yday;
		}

		/**
		 * \brief 得到当前月份，范围1-12
		 *
		 * \return 
		 */
		int getMonth()
		{
			return tv.tm_mon+1;
		}
		
		/**
		 * \brief 得到当前年份
		 *
		 * \return 
		 */
		int getYear()
		{
			return tv.tm_year+1900;
		}	

	private:

		/**
		 * \brief 存储时间，单位秒
		 */
		time_t secs;
		
		/**
		 * \brief tm结构，方便访问
		 */
		struct tm tv;


};

class Timer
{
	public:
		Timer(const float how_long , const int delay=0) : _long((int)(how_long*1000)), _timer(delay*1000)
		{
			//how_long , which unit is second;
		}

		/**
		 * \brief 重新设置定时器的精度和开始计时时间
		 * \param how_long  定时器的时间，单位：毫秒
		 * \param ctv       当前时间
		 *  
		 */ 
		void reset(const unsigned long how_long, const zRTime &cur)
		{
			_long = how_long;
			_timer = cur;
			_timer.addDelay(_long);
		}

		Timer(const float how_long , const zRTime &cur) : _long((int)(how_long*1000)), _timer(cur)
		{
			_timer.addDelay(_long);
		}
		void next(const zRTime &cur)
		{
			_timer=cur;
			_timer.addDelay(_long);
		} 
		bool operator() (const zRTime& current)
		{
			if (_timer <= current) {
				_timer = current;
				_timer.addDelay(_long);
				return true;
			}

			return false;
		}
	private:
		int _long;
		zRTime _timer;
};

class zClocker
{
	public:
		zClocker(const unsigned int how_long , const unsigned int interval) : _long(how_long*1000),_interval(interval*1000),_timer()
		{
			assert(_long <= _interval);
			_timer.addDelay(getNextDelay());
		}

		zClocker() : _long(0),_interval(0),_timer()
		{   
			//assert(_long <= _interval);
			//_timer.addDelay(getNextDelay());
		}   

		void init(const unsigned int how_long , const unsigned int interval)
		{
			_timer.now();
			_long = how_long*1000;
			_interval = interval*1000;
			assert(_long <= _interval);
			_timer.addDelay(getNextDelay());
		}

		bool operator() (const zRTime& current)
		{
			if (_timer <= current) {
				_timer = current;
				_timer.addDelay(getNextDelay());
				return true;
			}

			return false;
		}
		//还有多长时间闹钟(单位:s)
		unsigned int remainderTime(const zRTime& current)
		{
			return (current.elapse(_timer)/1000L);
		}
	private:
		unsigned long getNextDelay()
		{
			unsigned long lastmsec = _timer.local_msecs() % _interval;
			if (lastmsec < _long)
			{
				return _long - lastmsec;
			}
			else
			{
				return _interval - lastmsec + _long;
			}
		}
		unsigned long _long;
		unsigned long _interval;
		zRTime _timer;
};

//时间间隔具有随机性
class RandTimer
{
	public:
		RandTimer(const float how_long , const int delay=0) : _long((int)(how_long*1000)), _timer(delay*1000)
		{

		}

		RandTimer(const float how_long , const zRTime &cur);
		void next(const zRTime &cur);
		bool operator() (const zRTime& current);
	private:
		int _long;
		zRTime _timer;
};

struct FunctionInterval
{
	struct timespec _tv_1;
	struct timespec _tv_2;
	const unsigned long _need_log;
	const char *_fun_name;
	FunctionInterval(const unsigned long interval):_need_log(interval)
	{
		_tv_1.tv_sec=-1;
		_tv_1.tv_nsec=-1;
	}
	void interval(const char *func=NULL);
};
struct FunctionTime
{
	private:
	struct timespec _tv_1;
	struct timespec _tv_2;
	const unsigned long _need_log;
	const char *_fun_name;
	const char *_dis;
	const int _dis_len;
	public:
	FunctionTime(const unsigned long interval,const char *func=NULL,const char *dis=NULL,const int dis_len=16):_need_log(interval),_fun_name(func),_dis(dis),_dis_len(dis_len)
	{
		clock_gettime(CLOCK_REALTIME, &_tv_1);
	}
	~FunctionTime();
};

struct CommandTime
{
private:
	struct timespec _tv_1;
	struct timespec _tv_2;
	const unsigned long _need_log;
	const char *_where;
	const BYTE _cmd;
	const BYTE _para;
public:
	CommandTime(const unsigned long interval, const char *where, BYTE cmd, BYTE para) 
		: _need_log(interval), _where(where), _cmd(cmd), _para(para)
	{
		clock_gettime(CLOCK_REALTIME, &_tv_1);
	}
	~CommandTime();
};

struct BlockTime
{
private:
	struct timespec _tv_1;
	struct timespec _tv_2;
	const unsigned long _interval;
	const char* _where;
public:
	BlockTime(const unsigned long interval, const char *where)
		: _interval(interval), _where(where)
	{
		clock_gettime(CLOCK_REALTIME, &_tv_1);
	}
	~BlockTime() {}
	void elapse();
};

#define TIMES_HASH 1024

struct FunctionTimes
{
	struct Times
	{
		//Times();
		Times():_log_timer(600),_times(0),_total_time(0)
		{
			bzero(_dis,sizeof(_dis));
		}
		Timer _log_timer;
		char _dis[TIMES_HASH];
		unsigned long _times;
		unsigned long _total_time;
		zMutex _mutex;
	};
	private:
	static Times _times[TIMES_HASH]; 
	int _which;
	struct timespec _tv_1;
	struct timespec _tv_2;
	public:
	FunctionTimes(const int which , const char *dis);
	~FunctionTimes();
};

//直接从数据库操作函数中计算执行时间
struct FunctionTimesV2
{
	struct Times
	{
		Times():_log_timer(600),_times(0),_total_time(0)
		{
			bzero(_sqlOP, sizeof(_sqlOP));
		}
		Timer _log_timer;
		QWORD _times;
		QWORD _total_time;
		char _sqlOP[16];
		zMutex _mutex;
	};
	private:
	static std::map<QWORD, Times*> _times;
	static bool clearMapFlag;
	QWORD _addr;
	struct timespec _tv_1;
	struct timespec _tv_2;
	public:
	FunctionTimesV2(const char *sqlFunc, const QWORD &addr);
	~FunctionTimesV2();
};
/*
struct CmdAnalysis
{
	CmdAnalysis(const char *disc,DWORD time_secs):_log_timer(time_secs)
	{
		bzero(_disc,sizeof(disc));
		strncpy(_disc,disc,sizeof(_disc)-1);
		bzero(_data,sizeof(_data));
		_switch=false;
	}
	struct
	{
		DWORD num;
		DWORD size;
	}_data[256][256] ;
	zMutex _mutex;
	Timer _log_timer;
	char _disc[256];
	bool _switch;//开关
	void add(const BYTE &cmd, const BYTE &para , const DWORD &size)
	{
		if(!_switch)
		{
			return ;
		}
		_mutex.lock(); 
		_data[cmd][para].num++;
		_data[cmd][para].size +=size;
		zRTime ct;
		if(_log_timer(ct))
		{
			for(int i = 0 ; i < 256 ; i ++)
			{
				for(int j = 0 ; j < 256 ; j ++)
				{
					if(_data[i][j].num)
						Fir::logger->debug("%s:%d,%d,%d,%d",_disc,i,j,_data[i][j].num,_data[i][j].size);
				}
			}
			bzero(_data,sizeof(_data));
		}
		_mutex.unlock(); 
	}
};
// */
/*
struct FunctionTimes
{
	private:
		int _times;
		Timer _timer;
	public:
		FunctionTimes():_times(0),_timer(60)
		{
		}
		void operator();
		
};
// */
#endif
