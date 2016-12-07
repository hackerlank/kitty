#include "zTime.h"
#include "Fir.h"
#include "zMisc.h"

long zRTime::my_timezone = 0L;

/**
 * \brief 得到系统时区设置字符串
 *
 * \param s 时区将放入此字符串中
 * \return 返回参数s
 */
const std::string & zRTime::getLocalTZ()
{
	static std::string s;
	static bool init = false;
	if (!init)
	{
		std::ostringstream so;
		tzset();
		if (0L == my_timezone)
			my_timezone = timezone;
		so << tzname[0] << timezone/3600;
		s= so.str();
		init = true;
	}
	return s;
}

/**
 * \brief 锁时区操作，避免多线程导致时区错乱
 */
static zMutex tz_lock;

/**
 * \brief 保留时区信息
 * \param tzstr 缓冲字符串
 */
void zRTime::save_timezone(std::string &tzstr)
{
	tz_lock.lock();
	std::string ss = zRTime::getLocalTZ();
	std::ostringstream so;
	so << "TZ=" << ss;
	tzstr = so.str();
}

/**
 * \brief 恢复时区信息
 * \param tzstr 缓冲字符串
 */
void zRTime::restore_timezone(const std::string &tzstr)
{
	static char tmp_tzstr[256 + 1];
	bzero(tmp_tzstr, sizeof(tmp_tzstr));
	strncpy(tmp_tzstr, tzstr.c_str(), sizeof(tmp_tzstr) - 1);
	putenv(tmp_tzstr);
	tzset();
	tz_lock.unlock();
}

void FunctionInterval::interval(const char *func)
{
	clock_gettime(CLOCK_REALTIME, &_tv_2);
	unsigned long end=_tv_2.tv_sec*1000000L + _tv_2.tv_nsec/1000L;
	unsigned long begin= _tv_1.tv_sec*1000000L + _tv_1.tv_nsec/1000L;
	if(end - begin > _need_log)
	{
		Fir::logger->debug("%s执行时间间隔 %lu 微秒",func,end - begin);
	}
	_tv_1=_tv_2;
}
FunctionTime::~FunctionTime()
{
	clock_gettime(CLOCK_REALTIME, &_tv_2);
	unsigned long end=_tv_2.tv_sec*1000000L + _tv_2.tv_nsec/1000L;
	unsigned long begin= _tv_1.tv_sec*1000000L + _tv_1.tv_nsec/1000L;
	if(end - begin > _need_log)
	{
		char buf[_dis_len+1];
		bzero(buf,sizeof(buf));
		strncpy(buf,_dis,_dis_len);
		Fir::logger->debug("%s执行时间%lu 微秒,描述:%s",_fun_name,end - begin , buf);
	}
}
CommandTime::~CommandTime()
{
	clock_gettime(CLOCK_REALTIME, &_tv_2);
	unsigned long end=_tv_2.tv_sec*1000000L + _tv_2.tv_nsec/1000L;
	unsigned long begin= _tv_1.tv_sec*1000000L + _tv_1.tv_nsec/1000L;
	if(end - begin > _need_log)
	{
		Fir::logger->debug("在%s 解析指令[%d,%d]执行时间:%lu", _where, _cmd, _para, end-begin);
	}
}

void BlockTime::elapse()
{
	clock_gettime(CLOCK_REALTIME, &_tv_2);
	unsigned long end=_tv_2.tv_sec*1000000L + _tv_2.tv_nsec/1000L;
	unsigned long begin= _tv_1.tv_sec*1000000L + _tv_1.tv_nsec/1000L;
	if(end - begin > _interval)
	{
		Fir::logger->debug("[管理器超时]%s 执行时间:%lu毫秒", _where, (end-begin)/1000);
	}
}

FunctionTimes::Times FunctionTimes::_times[TIMES_HASH]; 
FunctionTimes::FunctionTimes(const int which , const char *dis)
{
	if (1 == Fir::function_times)
	{
		_which = which;
		_times[_which]._mutex.lock(); 
		if(!_times[_which]._times)
		{
			strncpy(_times[_which]._dis,dis,sizeof(_times[_which]._dis));
		}
		clock_gettime(CLOCK_REALTIME, &_tv_1);
	}
}

FunctionTimes::~FunctionTimes()
{
	if (1 == Fir::function_times)
	{
		clock_gettime(CLOCK_REALTIME, &_tv_2);
		unsigned long end=_tv_2.tv_sec*1000000L + _tv_2.tv_nsec/1000L;
		unsigned long begin= _tv_1.tv_sec*1000000L + _tv_1.tv_nsec/1000L;
		_times[_which]._times++;
		_times[_which]._total_time += (end - begin);
		zRTime ct;
		if(_times[_which]._log_timer(ct))
		{
			Fir::logger->debug("[函数统计]:执行次数(%d):%lu,执行总时间:%lu 微秒,说明:%s",_which,_times[_which]._times,_times[_which]._total_time ,_times[_which]._dis);
			_times[_which]._times=0;
			_times[_which]._total_time=0;
		}
		_times[_which]._mutex.unlock(); 
	}
}

typedef std::map<QWORD, FunctionTimesV2::Times *>::iterator _TimesIter;
std::map<QWORD, FunctionTimesV2::Times *> FunctionTimesV2::_times;
bool FunctionTimesV2::clearMapFlag = false;
FunctionTimesV2::FunctionTimesV2(const char *sqlOP, const QWORD &addr)
{
	if (2 == Fir::function_times)
	{
		clearMapFlag = true;

		_addr = addr;
		_TimesIter iter = _times.find(_addr);
		if( _times.end() == iter )
		{
			Times *instance = FIR_NEW Times();
			strncpy(instance->_sqlOP, sqlOP, sizeof(instance->_sqlOP));
			iter = _times.insert(iter, std::pair<QWORD, Times*>(_addr, instance));
		}

		iter->second->_mutex.lock();
		clock_gettime(CLOCK_REALTIME, &_tv_1);
	}
	else if(clearMapFlag)
	{
		clearMapFlag = false;

		for(_TimesIter iter = _times.begin(); _times.end() != iter; ++ iter)
		{
			if(iter->second)
				SAFE_DELETE(iter->second);
		}
		_times.clear();
	}
}

FunctionTimesV2::~FunctionTimesV2()
{
	if (2 == Fir::function_times)
	{
		clock_gettime(CLOCK_REALTIME, &_tv_2);

		_TimesIter iter = _times.find(_addr);
		if( _times.end() == iter ) return;

		unsigned long end=_tv_2.tv_sec*1000000L + _tv_2.tv_nsec/1000L;
		unsigned long begin= _tv_1.tv_sec*1000000L + _tv_1.tv_nsec/1000L;
		iter->second->_times++;
		iter->second->_total_time += (end - begin);
		zRTime ct;
		if(iter->second->_log_timer(ct))
		{
			Fir::logger->debug("[函数统计V2]:执行次数:%lu,执行总时间:%lu 微秒,SQL操作:%s,地址:%lu", iter->second->_times, iter->second->_total_time,iter->second->_sqlOP, iter->first);
			iter->second->_times=0;
			iter->second->_total_time=0;
		}
		
		iter->second->_mutex.unlock();
	}
}

#define next_time(_long) (_long / 2 + zMisc::randBetween(0, _long))

void RandTimer::next(const zRTime &cur)
{
	_timer=cur;
	_timer.addDelay(next_time(_long));
}

bool RandTimer::operator() (const zRTime& current)
		{
			if (_timer <= current) {
				_timer = current;
				_timer.addDelay(next_time(_long));
				return true;
			}

			return false;
		}

RandTimer::RandTimer(const float how_long , const zRTime &cur) : _long((int)(how_long*1000)), _timer(cur)
{
			_timer.addDelay(next_time(_long));
}

