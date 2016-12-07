#include "zLogger.h"

#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _USE_GCC_4
#include <log4cxx/level.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/logger.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/dailyrollingfileappender.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/rolling/rolloverdescription.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/fileoutputstream.h>
#include <log4cxx/helpers/date.h>
#include <log4cxx/logstring.h>
#include <log4cxx/spi/loggerfactory.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/spi/location/locationinfo.h>

#else
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/helpers/dateformat.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/system.h>
#endif

#include <iostream>
#include <locale.h>

#include "zTime.h"
#include "zServerInfo.h"
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include <string>

#ifdef _USE_GCC_4
//重载为了创建文件的符号链接
class LocalDailyRollingFileAppender : public log4cxx::DailyRollingFileAppender
{
	public:
		LocalDailyRollingFileAppender();
		virtual ~LocalDailyRollingFileAppender();
		std::string filenamepattern;

		bool relinkfile(log4cxx::helpers::Pool& pool);
		bool my_rollover(log4cxx::helpers::Pool& p);
		
		virtual void subAppend(const log4cxx::spi::LoggingEventPtr& event, log4cxx::helpers::Pool &pool);
		virtual void activateOptions(log4cxx::helpers::Pool& pool);
};

class LocalTimeBasedRollingPolicy : public log4cxx::rolling::TimeBasedRollingPolicy
{
	public:
		std::string getCurFileName(log4cxx::helpers::Pool &pool);
		~LocalTimeBasedRollingPolicy(){}
};

LocalDailyRollingFileAppender::LocalDailyRollingFileAppender()
{
}
LocalDailyRollingFileAppender::~LocalDailyRollingFileAppender()
{
	close();
}
bool LocalDailyRollingFileAppender::relinkfile(log4cxx::helpers::Pool& pool)
{
	using namespace log4cxx;
	using namespace log4cxx::rolling;
	using namespace log4cxx::helpers;
	using namespace log4cxx::pattern;
	
	LocalTimeBasedRollingPolicy policy;
	policy.setFileNamePattern(filenamepattern);
	policy.activateOptions(pool);
	std::string symlinkname = getFile();
	std::string filename = policy.getCurFileName(pool);
	
	struct stat statbuf;
	if(::lstat(symlinkname.c_str(), &statbuf) >= 0)
	{
		if(!S_ISLNK(statbuf.st_mode))
		{ //该文件已经存在了但是不是符号链接
			std::string msg = symlinkname;
			msg += "文件已经存在，但不是符号链接，请检查";
			log4cxx::helpers::LogLog::warn(LOG4CXX_STR(msg));
			return false;
		}
	}
	::unlink(symlinkname.c_str());
	//printf("创建%s--->>%s的符号链接\n", symlinkname.c_str(), filename.c_str());
	if( -1 == ::symlink(filename.c_str(), symlinkname.c_str()))
	{
		std::string msg = symlinkname;
		msg += "创建符号链接失败，请检查";
		msg += symlinkname;
		msg += "-->>";
		msg += filename;
		log4cxx::helpers::LogLog::warn(LOG4CXX_STR(msg));
        char buff[255];
        sprintf(buff,"errno=%d,Msg %s",errno,strerror(errno));
        log4cxx::helpers::LogLog::warn(LOG4CXX_STR(buff));
		return false;
	}
	return true;
}
bool LocalDailyRollingFileAppender::my_rollover(log4cxx::helpers::Pool& p)
{
	using namespace log4cxx;
	using namespace log4cxx::rolling;
	using namespace log4cxx::helpers;
	using namespace log4cxx::spi;
	
	if (rollingPolicy != NULL) 
	{
		{
			synchronized sync(mutex);
			try {
				RolloverDescriptionPtr rollover1(rollingPolicy->rollover(getFile(), p));

				if (rollover1 != NULL) {
					if (rollover1->getActiveFileName() == getFile()) {
						closeWriter();

						bool success = true;

#if 0 //不用rename了
						if (rollover1->getSynchronous() != NULL) {
							success = false;

							try {
								success = rollover1->getSynchronous()->execute(p);
							} catch (std::exception& ex) {
								LogLog::warn(LOG4CXX_STR("Exception on rollover"));
							}
						}
#endif
						this->relinkfile(p);

						if (success) {
							if (rollover1->getAppend()) {
								fileLength = File().setPath(rollover1->getActiveFileName()).length(p);
							} else {
								fileLength = 0;
							}

							//
							//  async action not yet implemented
							//
							ActionPtr asyncAction(rollover1->getAsynchronous());
							if (asyncAction != NULL) {
								asyncAction->execute(p);
							}

							setFile(
									rollover1->getActiveFileName(), rollover1->getAppend(),
									bufferedIO, bufferSize, p);
						} else {
							setFile(
									rollover1->getActiveFileName(), true, bufferedIO, bufferSize, p);
						}
					} else {
						OutputStreamPtr os(new FileOutputStream(
									rollover1->getActiveFileName(), rollover1->getAppend()));
						WriterPtr newWriter(createWriter(os));
						closeWriter();
						setFile(rollover1->getActiveFileName());
						setWriter(newWriter);

						bool success = true;
#if 0 //不用rename了
						if (rollover1->getSynchronous() != NULL) {
							success = false;

							try {
								success = rollover1->getSynchronous()->execute(p);
							} catch (std::exception& ex) {
								LogLog::warn(LOG4CXX_STR("Exception during rollover"));
							}
						}
#endif		
						this->relinkfile(p);

						if (success) {
							if (rollover1->getAppend()) {
								fileLength = File().setPath(rollover1->getActiveFileName()).length(p);
							} else {
								fileLength = 0;
							}

							//
							//   async action not yet implemented
							//
							ActionPtr asyncAction(rollover1->getAsynchronous());
							if (asyncAction != NULL) {
								asyncAction->execute(p);
							}
						}

						writeHeader(p);
					}

					return true;
				}
			} catch (std::exception& ex) {
				LogLog::warn(LOG4CXX_STR("Exception during rollover"));
			}
		}

	}

	return false;
}

void LocalDailyRollingFileAppender::subAppend(const log4cxx::spi::LoggingEventPtr& event, log4cxx::helpers::Pool &p)
{
	std::string filename;
	std::string filename2 ;
	if(0)
	{
		filename = getFile();
		log4cxx::rolling::RolloverDescriptionPtr rollover1(rollingPolicy->rollover(getFile(), p));
		if(rollover1)
		{
			filename2 =  rollover1->getActiveFileName();
		}
	}
	//printf("开始change files filename:%s, filename2:%s\n", filename.c_str(), filename2.c_str());
	
	// The rollover check must precede actual writing. This is the
	// only correct behavior for time driven triggers.
	if (triggeringPolicy->isTriggeringEvent(this, event, getFile(), getFileLength())) 
	{
		//   wrap rollover request in try block since
		//    rollover may fail in case read access to directory
		//    is not provided.  However appender should still be in good
		//     condition and the append should still happen.
		try {
			my_rollover(p);
		} catch (std::exception& ex) {
			log4cxx::helpers::LogLog::warn(LOG4CXX_STR("Exception during rollover attempt."));
		}   
	}
	FileAppender::subAppend(event, p); 
}

void LocalDailyRollingFileAppender::activateOptions(log4cxx::helpers::Pool& pool) 
{
	using namespace log4cxx;
	using namespace log4cxx::rolling;
	using namespace log4cxx::helpers;
	using namespace log4cxx::pattern;

	LocalTimeBasedRollingPolicy* policy = new LocalTimeBasedRollingPolicy();
	LogString pattern(getFile());
	bool inLiteral = false;
	bool inPattern = false;

	for (size_t i = 0; i < datePattern.length(); i++) {
		if (datePattern[i] == 0x27 /* '\'' */) {
			inLiteral = !inLiteral;

			if (inLiteral && inPattern) {
				pattern.append(1, (logchar) 0x7D /* '}' */);
				inPattern = false;
			}
		} else {
			if (!inLiteral && !inPattern) {
				const logchar dbrace[] = { 0x25, 0x64, 0x7B, 0 }; // "%d{"
				pattern.append(dbrace);
				inPattern = true;
			}

			pattern.append(1, datePattern[i]);
		}
	}

	if (inPattern) {
		pattern.append(1, (logchar) 0x7D /* '}' */);
	}

	policy->setFileNamePattern(pattern);
	//policy->activateOptions(pool);
	filenamepattern = pattern;
	setTriggeringPolicy(policy);
	setRollingPolicy(policy);
	if(this->relinkfile(pool))
	{
		RollingFileAppenderSkeleton::activateOptions(pool);
	}
}

std::string LocalTimeBasedRollingPolicy::getCurFileName(log4cxx::helpers::Pool &pool)
{
	using namespace log4cxx;
	using namespace log4cxx::rolling;
	using namespace log4cxx::helpers;
	using namespace log4cxx::pattern;
	LogString buf; 
	ObjectPtr obj(new Date());  
	formatFileName(obj, buf, pool);  
	return buf;
}

using namespace log4cxx;
using namespace log4cxx::spi;
IMPLEMENT_LOG4CXX_OBJECT(XLogger)
IMPLEMENT_LOG4CXX_OBJECT(XFactory)

namespace log4cxx
{
	XFactoryPtr XLogger::factory = new XFactory();

	LoggerPtr XLogger::getLogger(const LogString& name)
	{
		return LogManager::getLogger(name, factory);
	}

	LoggerPtr XLogger::getLogger(const helpers::Class& clazz)
	{
		return XLogger::getLogger(clazz.getName());
	}

	XFactory::XFactory()
	{
	}

	LoggerPtr XFactory::makeNewLoggerInstance(log4cxx::helpers::Pool& pool, 
			const LogString& name) const
	{
		return new XLogger(pool, name);
	}
};

zLogger::zLogger(const std::string& name)
{
	_logger = log4cxx::XLogger::getLogger(name);
	addConsoleLog();
	setLevel("trace");
}

zLogger::~zLogger()
{
//	_logger->getLoggerRepository()->shutdown();
}

bool zLogger::addConsoleLog()
{
	log4cxx::PatternLayoutPtr cpl = new log4cxx::PatternLayout("\%d{\%y\%m\%d-\%H:\%M:\%S }%c %5p: %m%n");

	log4cxx::helpers::Pool pool;
	cpl->activateOptions(pool);
	log4cxx::ConsoleAppenderPtr appender = new log4cxx::ConsoleAppender(cpl);
	appender->setName("console");

	_logger->addAppender(appender);
	return true;
}

bool zLogger::removeConsoleLog()
{
	log4cxx::ConsoleAppenderPtr appender = _logger->getAppender("console");
	_logger->removeAppender(appender);
	appender->close();
	return true;
}

bool zLogger::removeLocalFileLog(const std::string& filename)
{
    Appender* appender = _logger->getAppender(filename);
    if(appender)
	_logger->removeAppender(appender);
	return true;


 }

bool zLogger::addLocalFileLog(const std::string& filename)
{
	log4cxx::PatternLayoutPtr fpl = new log4cxx::PatternLayout("\%d{\%y\%m\%d-\%H:\%M:\%S }%c %5p: %m%n");
	log4cxx::helpers::Pool pool;
	fpl->activateOptions(pool);

	LocalDailyRollingFileAppender* appender = new  LocalDailyRollingFileAppender();
	//appender->setDatePattern("'.'yyyy-MM-dd-HH");
	appender->setDatePattern(".\%y\%m\%d-\%H");
	appender->setLayout(fpl);
	appender->setName(filename);
	appender->setFile(filename);
	appender->activateOptions(pool);
    appender->setEncoding("UTF-8");
	_logger->addAppender(appender);

	return true;
}

bool zLogger::addBasicFileLog(const std::string& filename)
{
	log4cxx::PatternLayoutPtr fpl = new log4cxx::PatternLayout("%m%n");
	log4cxx::helpers::Pool pool;
	fpl->activateOptions(pool);

	LocalDailyRollingFileAppender* appender = new  LocalDailyRollingFileAppender();
	//appender->setDatePattern("'.'yyyy-MM-dd-HH");
	appender->setDatePattern(".\%y\%m\%d-\%H");
	appender->setLayout(fpl);
	appender->setName(filename);
	appender->setFile(filename);
	appender->activateOptions(pool);
    appender->setEncoding("UTF-8");
	_logger->addAppender(appender);

	return true;
}

/**
 * \brief 设置Logger的名字，它出现在每条日志信息中
 * \param setName 要被设置的名字
 */
void zLogger::setName(const std::string& setName)
{
    _logger->setName(setName);
}

//志等级 trace < debug < info < warn < error < fatal 
bool zLogger::setLevel(const std::string& level)
{
	if(level == "trace")
	{
		_logger->setLevel(log4cxx::Level::getTrace());
	}
	else if(level == "debug")
	{
		_logger->setLevel(log4cxx::Level::getDebug());
	}
	else if(level == "info")
	{
		_logger->setLevel(log4cxx::Level::getInfo());
	}
	else if(level == "warn")
	{
		_logger->setLevel(log4cxx::Level::getWarn());
	}
	else if(level == "error")
	{
		_logger->setLevel(log4cxx::Level::getError());
	}
	else if(level == "fatal")
	{
		_logger->setLevel(log4cxx::Level::getFatal());
	}
	else if(level == "off")
	{
		_logger->setLevel(log4cxx::Level::getOff());
	}
	else
	{
		_logger->setLevel(log4cxx::Level::getTrace());
	}
	return true;
}

const static int MaxLoggerSize = 1024;
//const static int MaxLoggerSize = 10*1024;

#define getMessage() \
	char   message[MaxLoggerSize];\
	bzero(message, sizeof(message)); \
	va_list   va; \
	va_start(va, fmt); \
	vsnprintf(message, MaxLoggerSize-1, fmt, va); \
	va_end(va);

#define processLine()\
	std::string tmp(message);\
	bzero(message, sizeof(message)); \
	unsigned int index = 0;\
	for(unsigned int i = 0; i < tmp.size(); i++)\
	{\
		message[index++] = tmp[i];\
		break;\
		if(tmp[i] == '\t') \
		{\
			message[index++] = '\\'; \
			message[index++] = 't';\
		}\
		else if(tmp[i] == '\n') \
		{\
			message[index++] = '\\'; \
			message[index++] = 'n';\
		}\
		else\
		{\
			message[index++] = tmp[i];\
		}\
	}


void zLogger::debug(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->debug(message);
	msgMut.unlock();
}
void zLogger::trace(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->trace(message);
	msgMut.unlock();
}
void zLogger::info(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->info(message);
	msgMut.unlock();
}
void zLogger::warn(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->warn(message);
	msgMut.unlock();
}
void zLogger::error(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->error(message);
	msgMut.unlock();
}
void zLogger::fatal(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->fatal(message);
	msgMut.unlock();
}
void zLogger::iffy(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->info(message);
	msgMut.unlock();
}
void zLogger::alarm(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->info(message);
	msgMut.unlock();
}


void zLogger::sdebug(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->debug(message);
	msgMut.unlock();
}
void zLogger::strace(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->trace(message);
	msgMut.unlock();
}
void zLogger::sinfo(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->info(message);
	msgMut.unlock();
}
void zLogger::swarn(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->warn(message);
	msgMut.unlock();
}
void zLogger::serror(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->error(message);
	msgMut.unlock();
}
void zLogger::sfatal(const char* fmt, ...)
{
	msgMut.lock();
	getMessage();
	_logger->fatal(message);
	msgMut.unlock();
}

/**
 * \brief 写fatal程序日志,并发送到监控服务器
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::fatalM(const char * fmt, ...)
{
    msgMut.lock();
    getMessage();

    _logger->fatal(message);
    msgMut.unlock();
    return true;
}

/**
 * \brief 写error程序日志,并发送到监控服务器
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::errorM(const char * fmt, ...)
{
    msgMut.lock();
    getMessage();

    _logger->error(message);
    msgMut.unlock();
    return true;
}
/**
 * \brief 写warn程序日志,并发送到监控服务器
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::warnM(const char * fmt, ...)
{
    msgMut.lock();
    getMessage();

    _logger->warn(message);
    msgMut.unlock();
    return true;
}
/**
 * \brief 得到Logger的名字，它出现在每条日志信息中
 * \return  Logger名字
 */
const log4cxx::LogString zLogger::getName()
{
    return _logger->getName();
}
#else

const log4cxx::LevelPtr zLogger::zLevel::LEVELALARM(ZEBRA_NEW log4cxx::Level(ALARM_INT,_T("ALARM"),3));
const log4cxx::LevelPtr zLogger::zLevel::LEVELIFFY(ZEBRA_NEW log4cxx::Level(IFFY_INT,_T("IFFY"),3));
const log4cxx::LevelPtr zLogger::zLevel::LEVELTRACE(ZEBRA_NEW log4cxx::Level(TRACE_INT,_T("TRACE"),3));
const log4cxx::LevelPtr zLogger::zLevel::LEVELGBUG(ZEBRA_NEW log4cxx::Level(GBUG_INT,_T("GBUG"),3));

const zLogger::zLevel *  zLogger::zLevel::OFF=ZEBRA_NEW zLevel(log4cxx::Level::OFF);
const zLogger::zLevel *  zLogger::zLevel::FATAL=ZEBRA_NEW zLevel(log4cxx::Level::FATAL);
const zLogger::zLevel *  zLogger::zLevel::ALARM=ZEBRA_NEW zLevel(LEVELALARM);
const zLogger::zLevel *  zLogger::zLevel::ERROR=ZEBRA_NEW zLevel(log4cxx::Level::ERROR);
const zLogger::zLevel *  zLogger::zLevel::IFFY=ZEBRA_NEW zLevel(LEVELIFFY);
const zLogger::zLevel *  zLogger::zLevel::WARN=ZEBRA_NEW zLevel(log4cxx::Level::WARN);
const zLogger::zLevel *  zLogger::zLevel::TRACE=ZEBRA_NEW zLevel(LEVELTRACE);
const zLogger::zLevel *  zLogger::zLevel::INFO=ZEBRA_NEW zLevel(log4cxx::Level::INFO);
const zLogger::zLevel *  zLogger::zLevel::GBUG=ZEBRA_NEW zLevel(LEVELGBUG);
const zLogger::zLevel *  zLogger::zLevel::DEBUG=ZEBRA_NEW zLevel(log4cxx::Level::DEBUG);
const zLogger::zLevel *  zLogger::zLevel::ALL=ZEBRA_NEW zLevel(log4cxx::Level::ALL);

#define getMessage(msg,msglen,pat)	\
do	\
{	\
	va_list ap;	\
	bzero(msg, msglen);	\
	va_start(ap, pat);		\
	vsnprintf(msg, msglen - 1, pat, ap);	\
	va_end(ap);	\
}while(false)

/**
 * \brief zLevel构造函数
 * \param  level 等级数字，类内部定义
 */
zLogger::zLevel::zLevel(log4cxx::LevelPtr level):zlevel(level)
{
}


log4cxx::helpers::TimeZonePtr zLogger::zLoggerLocalFileAppender::tz(log4cxx::helpers::TimeZone::getDefault());

/**
 * \brief 构造一个本地文件Appender 
 */
zLogger::zLoggerLocalFileAppender::zLoggerLocalFileAppender()
{
}

/**
 * \brief 析构时，回收DateFormat内存
 */
zLogger::zLoggerLocalFileAppender::~zLoggerLocalFileAppender()
{
	SAFE_DELETE(df);
}

#define remove_and_link(oldpath, newpath) \
	do { \
		remove(T2A(newpath.c_str())); \
		symlink(T2A(oldpath.c_str()), T2A(newpath.c_str())); \
	} while(false)

void zLogger::zLoggerLocalFileAppender::my_rollOver()
{
	/* Compute filename, but only if datePattern is specified */
	if (datePattern.empty())
	{
		errorHandler->error(_T("Missing DatePattern option in my_rollOver()."));

		return;
	}

	log4cxx::String datedFilename = my_fileName + df->format(now);

	// It is too early to roll over because we are still within the
	// bounds of the current interval. Rollover will occur once the
	// next interval is reached.
	if (scheduledFilename == datedFilename)
	{
		return;
	}

	// close current file, and rename it to datedFilename
	this->closeWriter();

	try
	{
		// This will also close the file. This is OK since multiple
		// close operations are safe.
		// append to the file, not clear the old content
		this->setFile(datedFilename, this->fileAppend, this->bufferedIO, this->bufferSize);

		//remove and link
		remove_and_link(datedFilename, my_fileName);
	}
	catch (log4cxx::helpers::Exception&)
	{
		errorHandler->error(_T("setFile(") + datedFilename + _T(", false) call failed."));
	}

	scheduledFilename = datedFilename;
}

void zLogger::zLoggerLocalFileAppender::subAppend(const log4cxx::spi::LoggingEventPtr& event)
{
	int64_t n = log4cxx::helpers::System::currentTimeMillis();

	if (n >= nextCheck) 
	{
		now = n;
		nextCheck = rc.getNextCheckMillis(now);

		try 
		{
			my_rollOver();
		} 
		catch (log4cxx::helpers::Exception& e)
		{
			log4cxx::helpers::LogLog::error(_T("my_rollOver() failed."), e);
		}
	}

	FileAppender::subAppend(event);
}

void zLogger::zLoggerLocalFileAppender::setMyFile(const log4cxx::String& file)
{
	// Trim spaces from both ends. The users probably does not want
	// trailing spaces in file names.
	my_fileName = log4cxx::helpers::StringHelper::trim(file);
}

/**
 * \brief 设置时区
 *
 * \param timeZone 时区字符串 
 */
void zLogger::zLoggerLocalFileAppender::setTimeZone(const log4cxx::String &timeZone)
{
	std::string tzstr;
	//save the timezone infomation
	zRTime::save_timezone(tzstr);
	tz=log4cxx::helpers::TimeZone::getTimeZone(timeZone);
	//restore the timezone infomation
	zRTime::restore_timezone(tzstr);
}

/**
 * \brief 激活所设置的选项
 */
void zLogger::zLoggerLocalFileAppender::activateOptions()
{
	//设置时区和预定文件
	rc.setTimeZone(tz);
	//DailyRollingFileAppender
	if (!datePattern.empty())
	{
		std::string tzstr;
		//save the timezone infomation
		zRTime::save_timezone(tzstr);
		rc.setType(rc.computeTriggeringPeriod(datePattern));
		//restore the timezone infomation
		zRTime::restore_timezone(tzstr);

		rc.printPeriodicity();

		now = log4cxx::helpers::System::currentTimeMillis();
		df = ZEBRA_NEW log4cxx::helpers::DateFormat(datePattern, tz);

		//FileAppender
		if (!my_fileName.empty())
		{
			scheduledFilename = my_fileName + df->format(now);

			try
			{
				//如果原来的my_fileName文件不是符号连接，而是个真实的文件，需要重新命名
				//重新命名的规则是，根据文件最后修改日期来确定文件名的后缀名
				struct stat my_fileStats;
				if (::lstat(T2A(my_fileName.c_str()), &my_fileStats) == 0 && S_ISREG(my_fileStats.st_mode))
				{
					log4cxx::String new_fileName = my_fileName + df->format((int64_t)my_fileStats.st_mtime * 1000);
					struct stat new_fileStats;
					if (::lstat(T2A(new_fileName.c_str()), &new_fileStats) == 0 && S_ISREG(new_fileStats.st_mode))
					{
						log4cxx::helpers::LogLog::error(my_fileName + _T("不是符号连接，必须重新命名为") + new_fileName + _T("，但是新文件已经存在并且为普通文件，为了避免日志遗漏，请将新文件名重新命名为其他文件，或者对两个文件的内容进行拼接。"));
						log4cxx::helpers::LogLog::error(_T("程序异常退出。"));
						exit(0);
					}
					//remove and rename
					{
						//remove the new filename
						remove(T2A(new_fileName.c_str()));

						//rename the old file to new filename
						std::string a_my_fileName = T2A(my_fileName.c_str());
						std::string a_new_filename = T2A(new_fileName.c_str());
						if (::rename(a_my_fileName.c_str(), a_new_filename.c_str()) == 0)
						{
							log4cxx::helpers::LogLog::debug(my_fileName + _T(" -> ") + new_fileName);
						}
						else
						{
							log4cxx::helpers::LogLog::error(
									_T("Failed to rename [") + my_fileName + _T("] to [") +
									new_fileName + _T("]."));
							log4cxx::helpers::LogLog::error(_T("程序异常退出。"));
							exit(0);
						}
					}
				}

				this->setFile(scheduledFilename, this->fileAppend, this->bufferedIO, this->bufferSize);

				//remove and link
				remove_and_link(scheduledFilename, my_fileName);
			}
			catch(log4cxx::helpers::Exception& e)
			{
				errorHandler->error(_T("Unable to open file: ") + scheduledFilename,
						e, log4cxx::spi::ErrorCode::FILE_OPEN_FAILURE);
			}
		}
		else
		{
			log4cxx::helpers::LogLog::warn(_T("File option not set for appender [")+name+_T("]."));
			log4cxx::helpers::LogLog::warn(_T("Are you using FileAppender instead of ConsoleAppender?"));
		}
	}
	else
	{
		log4cxx::helpers::LogLog::error(
				_T("DatePattern options are not set for appender [")
				+ name + _T("]."));
	}
}

/**
 * \brief 构造一个zLogger 
 *
 * \param  name zLogger的名字，将会出现在输出的日志中的每一行
 */
zLogger::zLogger(const log4cxx::String &name)
{
	///std::cout << __PRETTY_FUNCTION__ << std::endl;
	bzero(message, sizeof(message));
	logger = log4cxx::Logger::getLogger(name);
	logger->setLevel(log4cxx::Level::DEBUG);
	m_info = NULL;
	addConsoleLog();
}

/**
 * \brief 析构函数
 */
zLogger::~zLogger()
{
}

void zLogger::setInfoM(zServerInfoManager * const info)
{
	msgMut.lock();
	m_info = info;
	msgMut.unlock();
}

/**
 * \brief 写fatal程序日志,并发送到监控服务器
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::fatalM(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	if(m_info) m_info->sendLogInfo(getName().c_str(),"fatal",message);
	
	logger->fatal(message);
	msgMut.unlock();
	return true;
}

/**
 * \brief 写error程序日志,并发送到监控服务器
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::errorM(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	if(m_info) m_info->sendLogInfo(getName().c_str(),"error",message);
	
	logger->error(message);
	msgMut.unlock();
	return true;
}


/**
 * \brief 写warn程序日志,并发送到监控服务器
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::warnM(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	if(m_info) m_info->sendLogInfo(getName().c_str(),"warn",message);
	
	logger->warn(message);
	msgMut.unlock();
	return true;
}

/**
 * \brief 得到Logger的名字，它出现在每条日志信息中
 * \return	Logger名字
 */
const log4cxx::String & zLogger::getName()
{
	return logger->getName();
}

/**
 * \brief 设置Logger的名字，它出现在每条日志信息中
 * \param setName 要被设置的名字
 */
void zLogger::setName(const log4cxx::String & setName)
{
	((PowerLogger *)logger.p)->setName(setName);
}

/**
 * \brief 添加控制台输出Log
 * \return	成功返回true，否则返回false 
 */
bool zLogger::addConsoleLog()
{
	log4cxx::PatternLayoutPtr cpl = ZEBRA_NEW log4cxx::PatternLayout("\%d{\%y\%m\%d-\%H:\%M:\%S }%c %5p: %m%n");
	std::string s = zRTime::getLocalTZ();
	cpl->setTimeZone(s);
	cpl->activateOptions();

	log4cxx::ConsoleAppenderPtr carp = ZEBRA_NEW log4cxx::ConsoleAppender(cpl);
	carp->setName("console");

	logger->addAppender(carp);
	return true;
}

/**
 * \brief 移除控制台Log输出
 */
void zLogger::removeConsoleLog()
{
	log4cxx::AppenderPtr ap=logger->getAppender("console");
	logger->removeAppender(ap);
	ap->close();
}

/**
 * \brief 加一个本地文件Log输出
 *
 * \param file 要输出的文件名，Logger会自动地添加时间后缀 
 * \return 成功返回true，否则返回false
 */
bool zLogger::addLocalFileLog(const log4cxx::String &file)
{
	log4cxx::PatternLayoutPtr fpl = ZEBRA_NEW log4cxx::PatternLayout("\%d{\%y\%m\%d-\%H:\%M:\%S }%c %5p: %m%n");
	std::string s = zRTime::getLocalTZ();
	fpl->setTimeZone(s);
	fpl->activateOptions();

	zLoggerLocalFileAppender * farp = ZEBRA_NEW  zLoggerLocalFileAppender();
	farp->setDatePattern(".\%y\%m\%d-\%H");
	farp->setTimeZone(s);
	farp->setMyFile(file);
	farp->setLayout(fpl);
	farp->activateOptions();
	farp->setName("localfile:"+file);

	logger->addAppender(farp);
	return true;
}

bool zLogger::addBasicFileLog(const log4cxx::String &file)
{
	log4cxx::PatternLayoutPtr fpl = ZEBRA_NEW log4cxx::PatternLayout("%m%n");
	std::string s = zRTime::getLocalTZ();
	fpl->setTimeZone(s);
	fpl->activateOptions();

	zLoggerLocalFileAppender * farp = ZEBRA_NEW  zLoggerLocalFileAppender();
	farp->setDatePattern(".\%y\%m\%d-\%H");
	farp->setTimeZone(s);
	farp->setMyFile(file);
	farp->setLayout(fpl);
	farp->activateOptions();
	farp->setName("localfile:"+file);

	logger->addAppender(farp);
	return true;
}

/**
 * \brief 每天的日志输出
 *
 *
 */
bool zLogger::addDailyLocalFileLog(const log4cxx::String &file)
{
    //log4cxx::PatternLayoutPtr fpl = new log4cxx::PatternLayout("\%d{\%y\%m\%d-\%H:\%M:\%S }%c %5p: %m%n");
    log4cxx::PatternLayoutPtr fpl = ZEBRA_NEW log4cxx::PatternLayout("\%d{\%y\%m\%d-\%H:\%M:\%S }%c %5p: %m%n");
    std::string s = zRTime::getLocalTZ();
    fpl->setTimeZone(s);
    fpl->activateOptions();

    zLoggerLocalFileAppender * farp = ZEBRA_NEW  zLoggerLocalFileAppender();
    //farp->setDatePattern(".\%y\%m\%d-\%H");
    farp->setDatePattern(".\%y\%m\%d");
    farp->setTimeZone(s);
    farp->setMyFile(file);
    farp->setLayout(fpl);
    farp->activateOptions();
    farp->setName("localfile:"+file);

    logger->addAppender(farp);
    return true;
}

/**
 * \brief 移出一个本地文件Log输出
 * \param file 要移除的Log文件名 
 */
void zLogger::removeLocalFileLog(const log4cxx::String &file)
{
	log4cxx::AppenderPtr ap=logger->getAppender("localfile:"+file);
	logger->removeAppender(ap);
	ap->close();
}

/**
 * \brief 设置写日志等级
 * \param  zLevelPtr 日志等级.参见 #zLogger::zLevel
 */
void zLogger::setLevel(const zLevel * zLevelPtr)
{
	logger->setLevel(zLevelPtr->zlevel);
}

/**
 * \brief 设置写日志等级
 * \param  level 日志等级
 */
void zLogger::setLevel(const std::string &level)
{
	if ("off" == level)
		setLevel(zLevel::OFF);
	else if ("fatal" == level)
		setLevel(zLevel::FATAL);
	else if ("alarm" == level)
		setLevel(zLevel::ALARM);
	else if ("error" == level)
		setLevel(zLevel::ERROR);
	else if ("iffy" == level)
		setLevel(zLevel::IFFY);
	else if ("warn" == level)
		setLevel(zLevel::WARN);
	else if ("trace" == level)
		setLevel(zLevel::TRACE);
	else if ("info" == level)
		setLevel(zLevel::INFO);
	else if ("gbug" == level)
		setLevel(zLevel::GBUG);
	else if ("debug" == level)
		setLevel(zLevel::DEBUG);
	else if ("all" == level)
		setLevel(zLevel::ALL);
}

/**
 * \brief 写日志
 * \param  zLevelPtr 日志等级参见 #zLogger::zLevel
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::log(const zLevel * zLevelPtr,const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->log(zLevelPtr->zlevel,message);
	msgMut.unlock();
	return true;
}
bool zLogger::slog(const zLevel * zLevelPtr,const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->log(zLevelPtr->zlevel,message);
	msgMut.unlock();
	return true;
}


/**
 * \brief 强制写日志,不受日志等级限制
 * \param  zLevelPtr 日志等级参见 #zLogger::zLevel
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::forceLog(const zLevel * zLevelPtr,const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->forcedLog(zLevelPtr->zlevel,message);
	msgMut.unlock();
	return true;
}

/**
 * \brief 写fatal程序日志
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::fatal(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->fatal(message);
	msgMut.unlock();
	return true;
}
bool zLogger::sfatal(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->fatal(message);
	msgMut.unlock();
	return true;
}

/**
 * \brief 写error程序日志
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::error(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->error(message);
	msgMut.unlock();
	return true;
}
bool zLogger::serror(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->error(message);
	msgMut.unlock();
	return true;
}


/**
 * \brief 写warn程序日志
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::warn(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->warn(message);
	msgMut.unlock();
	return true;
}
bool zLogger::swarn(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->warn(message);
	msgMut.unlock();
	return true;
}

/**
 * \brief 写info程序日志
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::info(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->info(message);
	msgMut.unlock();
	return true;
}
bool zLogger::sinfo(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->info(message);
	msgMut.unlock();
	return true;
}

/**
 * \brief 写debug程序日志
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::debug(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->debug(message);
	msgMut.unlock();
	return true;
}
bool zLogger::sdebug(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->debug(message);
	msgMut.unlock();
	return true;
}

/**
 * \brief 写alarm游戏日志
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::alarm(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->log(zLevel::ALARM->zlevel,message);
	msgMut.unlock();
	return true;
}
bool zLogger::salarm(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->log(zLevel::ALARM->zlevel,message);
	msgMut.unlock();
	return true;
}

/**
 * \brief 写iffy游戏日志
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::iffy(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->log(zLevel::IFFY->zlevel,message);
	msgMut.unlock();
	return true;
}

/**
 * \brief 写trace游戏日志
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::trace(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->log(zLevel::TRACE->zlevel,message);
	msgMut.unlock();
	return true;
}
bool zLogger::strace(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->log(zLevel::TRACE->zlevel,message);
	msgMut.unlock();
	return true;
}

/**
 * \brief 写gbug游戏日志
 * \param  pattern 输出格式范例，与printf一样
 * \return 成功返回true，否则返回false
 */
bool zLogger::gbug(const char * pattern, ...)
{
	msgMut.lock();
	getMessage(message,MSGBUF_MAX,pattern);

	logger->log(zLevel::GBUG->zlevel,message);
	msgMut.unlock();
	return true;
}

#endif


/**
 * \brief 游戏功能统计
 * \param pFuncName: 功能名称
 * \param type: 0,代表功能；1，代表任务。
 * \param desc: 功能描述
 * \param num: 参与数量, 默认为: 1
 *
 */
void FuncLog::logger(BYTE type, const char* pFuncName, const char* desc, DWORD num)
{
        Fir::logger->trace("[游戏功能统计],0,0,0,%u,%s,%s,%u", type, pFuncName, desc, num);
}

char MemLog::message[MSGBUF_MAX];
#ifdef _USE_GCC_4
void MemLog::logger(BYTE type, const void* addr, bool newORdel, const char* fmt, ...)
#else
void MemLog::logger(BYTE type, const void* addr, bool newORdel, const char* desc, ...)
#endif
{
#ifdef _USE_GCC_4
	getMessage();
#else
	getMessage(message, MSGBUF_MAX, desc);
#endif
	char str[MSGBUF_MAX]={0};
//	std::stringstream str;
	if(newORdel)
	{
	//	str << "构造";
			strcat(str,"构造");
	}
	else
	{
//		str << "析构";
			strcat(str,"析构");
	}
	//str << ", ";
	strcat(str, ",");

	switch(type)
	{
		case MemLog::Object:
			{
//				str << "Object";
					strcat(str, "Object");
			}
			break;
		case MemLog::SceneUser:
			{
				//str << "SceneUser";
				strcat(str, "SceneUser");
			}
			break;
		case MemLog::Scene:
			{
//				str << "Scene";
				strcat(str, "Scene");
			}
			break;
		case MemLog::UserSession:
			{
//				str << "UserSession";
				strcat(str, "UserSession");
			}
			break;
		case MemLog::SceneSession:
			{
//				str << "SceneSession";
				strcat(str, "SceneSession");
			}
			break;
		case MemLog::SceneNpc:
			{
//				str << "SceneNpc";
				strcat(str, "SceneNpc");
			}
			break;
		case MemLog::zSkill:
			{
//				str << "zSkill";
				strcat(str, "zSkill");
			}
			break;
		default:
			{
//				str << "None";
				strcat(str, "None");
				break;
			}
	};
	strcat(str, ",地址:");
//	str << ", 地址:";
	char tmp[MSGBUF_MAX]={0};
	snprintf(tmp, sizeof(tmp), "0x%llx,%s", (long long)(addr), message);
//	str << (long long)(addr);
//	str << ", " << message;
	strncat(str, tmp, sizeof(str)-strlen(tmp)-1);
//	Fir::logger->trace("[内存分配统计] %s", str.str().c_str());
	Fir::logger->trace("[内存分配统计] %s", str);
}


void TaskLog::logger(DWORD accid, QWORD charid, const char* pUserName, DWORD userLevel, DWORD taskID, const char* pTaskName, BYTE op, const char* desc)
{
	std::stringstream str;
	str<<accid<<","<<charid<<","<<pUserName<<","<<userLevel<<","<<taskID<<","<<pTaskName<<",";
	switch(op)
	{
		case 0:
			str<<"接取,";
			break;
		case 1:
			str<<"注销,";
			break;
		case 2:
			str<<"完成,";
			break;
		default:
			str<<"未知,";
			break;
	};
	str<<desc;
	Fir::logger->trace("[任务],%s", str.str().c_str());
}
