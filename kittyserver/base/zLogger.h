/**
 * \file
 * \version  $Id: zLogger.h 19808 2008-02-25 08:51:20Z dzl $
 * \author  ,okyhc@263.sina.com
 * \date 2004年11月18日 15时25分57秒 CST
 * 
 * \brief Fir新的日志系统声明文件
 *
 * 本文件是定义了三个类<code>zLogger::zLevel</code>,<code>zLogger</code>,<code>zLogger::zLoggerLocalFileAppender</code>
 */

#ifndef _ZLOGGER_H_
#define _ZLOGGER_H_

#ifdef _USE_GCC_4
#include <log4cxx/logger.h>
#include <log4cxx/dailyrollingfileappender.h>
#include <log4cxx/rolling/timebasedrollingpolicy.h>
#include <log4cxx/helpers/datetimedateformat.h>
#else
#include <log4cxx/logger.h>
#include <log4cxx/dailyrollingfileappender.h>
#endif

#include "zType.h"
#include "zMutex.h"
#include <fstream>


/**
 * \def MSGBUF_MAX
 * \brief Fir日志系统定义的最大日志长度 
 */
#define MSGBUF_MAX 4096

/**
 * \brief Fir项目的日志类，以Log4cxx基础构建的。
 *
 * 目前实现了三种写日志方式，即控制台、本地文件和Syslog系统。Syslog的等级为user。
 *
 * 默认日志级别是#DEBUG
 *
 * 此类为线程安全类。
 */

#ifdef _USE_GCC_4
//继承一个新的 Logger
namespace log4cxx
{
        namespace spi {
          namespace location {
            class LocationInfo;
          }
        }
        // Any sub-class of Logger must also have its own implementation of
        // LoggerFactory.
        class XFactory :
                public virtual spi::LoggerFactory,
                public virtual helpers::ObjectImpl
        {
        public:
                DECLARE_ABSTRACT_LOG4CXX_OBJECT(XFactory)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(XFactory)
                        LOG4CXX_CAST_ENTRY(spi::LoggerFactory)
                END_LOG4CXX_CAST_MAP()

                XFactory();
                virtual LoggerPtr makeNewLoggerInstance(
                   log4cxx::helpers::Pool& pool,
                   const LogString& name) const;
        };

        typedef helpers::ObjectPtrT<XFactory> XFactoryPtr;

        /**
        A simple example showing Logger sub-classing. It shows the
        minimum steps necessary to implement one's {@link LoggerFactory}.
        Note that sub-classes follow the hierarchy even if its loggers
        belong to different classes.
        */
        class XLogger : public Logger
        {
        // It's enough to instantiate a factory once and for all.
        static XFactoryPtr factory;

        public:
                DECLARE_ABSTRACT_LOG4CXX_OBJECT(XLogger)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(XLogger)
                        LOG4CXX_CAST_ENTRY_CHAIN(Logger)
                END_LOG4CXX_CAST_MAP()

                /**
                        Just calls the parent constuctor.
                */
                XLogger(log4cxx::helpers::Pool& pool,
                        const LogString& name1) : Logger(pool, name1) {}

                /**
                        Nothing to activate.
                */
                void activateOptions() {}
				
				void setName(const std::string& setName)
				{
					name = setName;
				}

				static LoggerPtr getLogger(const LogString& name);

				static LoggerPtr getLogger(const helpers::Class& clazz);
        };

        typedef helpers::ObjectPtrT<XLogger> XLoggerPtr;

};

class zLogger
{
	private:
		//log4cxx::LoggerPtr _logger;
		log4cxx::XLoggerPtr _logger;

		bool addConsoleLog();
	public:
		zLogger(const std::string& name);
		~zLogger();
		
		bool setLevel(const std::string& level);
		bool removeConsoleLog();
		bool addLocalFileLog(const std::string& filename);
        bool removeLocalFileLog(const std::string& filename);
		bool addBasicFileLog(const std::string& filename);
		const log4cxx::LogString  getName();
		void setName(const std::string& setName);

		bool errorM(const char * fmt, ...) __attribute__((format(printf,2,3))) ;
		bool fatalM(const char * fmt, ...) __attribute__((format(printf,2,3)));
		bool warnM(const char * fmt, ...) __attribute__((format(printf,2,3)));

		void debug(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void trace(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void info(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void warn(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void error(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void fatal(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void iffy(const char * fmt, ...) __attribute__((format(printf,2,3)));
		void alarm(const char * fmt, ...) __attribute__((format(printf,2,3)));

		void sdebug(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void strace(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void sinfo(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void swarn(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void serror(const char* fmt, ...) __attribute__((format(printf,2,3)));
		void sfatal(const char* fmt, ...) __attribute__((format(printf,2,3)));



	private:
		char message[MSGBUF_MAX];
		zMutex msgMut;
};
#else
class zLogger
{
	public:
		/**
		 * \brief zLevel声明了几个日志等级
		 *
		 * 除了用log4cxx提供的标准日志等级作为日志等级外，还自定义了游戏日志等级.
		 *
		 * 程序日志等级关系为 #OFF> #FATAL> #ERROR> #WARN> #INFO> #DEBUG> #ALL
		 *
		 * 游戏日志等级关系为 #ALARM> #IFFY> #TRACE> #GBUG
		 *
		 * 游戏日志等级与程序日志等级关系: #ALARM=#ERROR, #IFFY=#WARN, #TRACE=#INFO, #GBUG=#DEBUG
		 */
		class zLevel
		{
			friend class zLogger;
			private:
			/**
			 * \brief Fir项目所支持日志等级数字定义
			 */
			enum zLevelInt
			{
				ALARM_INT	= log4cxx::Level::ERROR_INT,
				IFFY_INT	= log4cxx::Level::WARN_INT,
				TRACE_INT	= log4cxx::Level::INFO_INT,
				GBUG_INT	= log4cxx::Level::DEBUG_INT
			};

			static const log4cxx::LevelPtr LEVELALARM;
			static const log4cxx::LevelPtr LEVELIFFY;
			static const log4cxx::LevelPtr LEVELTRACE;
			static const log4cxx::LevelPtr LEVELGBUG;

			log4cxx::LevelPtr zlevel;
			zLevel(log4cxx::LevelPtr level);

			public:
			/**
			 * \brief 当zLogger等级设置为OFF，除了用#forceLog函数，否则不会输出任何日志
			 */
			static const zLevel * OFF;
			/**
			 * \brief 当zLogger等级设置为FATAL，只输出FATAL等级的日志
			 *
			 * 程序致命错误，已经无法提供正常的服务功能。
			 */
			static const zLevel * FATAL;
			/**
			 * \brief 当zLogger等级设置为ERROR，只输出大于等于此等级的日志
			 *
			 * 错误，可能不能提供某种服务，但可以保证程序正确运行。
			 */
			static const zLevel * ERROR;
			/**
			 * \brief 当zLogger等级设置为ALARM，与ERROR同一级别
			 *
			 * 报警，游戏数据发生错误，比如检测到有外挂，游戏数据异常等等。与ERROR同一级别。
			 */
			static const zLevel * ALARM;
			/**
			 * \brief 当zLogger等级设置为WARN，只输出大于等于此等级的日志
			 *
			 * 警告，某些地方需要引起注意，比如没有配置文件，但程序用默认选项可以使用。
			 */
			static const zLevel * WARN;
			/**
			 * \brief 当zLogger等级设置为IFFY。
			 *
			 * 可疑的，需要追查的一些游戏数据，比如说一个变动的数据超出某种范围。与WARN同一级别。
			 */
			static const zLevel * IFFY;
			/**
			 * \brief 当zLogger等级设置为INFO，只输出大于等于此等级的日志
			 *
			 * 信息，提供一般信息记录，多用于一些程序状态的记录。
			 */
			static const zLevel * INFO;
			/**
			 * \brief 当zLogger等级设置为TRACE
			 *
			 * 	游戏跟踪，几乎要游戏中所有的关键数据跟踪，便于日后查找问题。
			 *
			 * 	比如角色升级，PK死亡，任务跟踪等等游戏事件。与INFO同一级别。
			 */
			static const zLevel * TRACE;
			/**
			 * \brief 当zLogger等级设置为DEBUG，输出所有等级的日志
			 */
			static const zLevel * DEBUG;
			/**
			 * \brief 当zLogger等级设置为GBUG
			 *
			 * 调试用的游戏数据。与DEBUG同一级别。
			 */
			static const zLevel * GBUG;
			/**
			 * \brief 当zLogger等级设置为ALL，输出所有等级的日志
			 */
			static const zLevel * ALL;
		};

		zLogger(const log4cxx::String & name="Fir");
		~zLogger();

		const log4cxx::String & getName();
		void setName(const log4cxx::String & setName);
		bool addConsoleLog();
		void removeConsoleLog();
		bool addLocalFileLog(const log4cxx::String &file);
		bool addBasicFileLog(const log4cxx::String &file);
		bool addDailyLocalFileLog(const log4cxx::String &file);
		void removeLocalFileLog(const log4cxx::String &file);

		void setLevel(const zLevel * zLevelPtr);
		void setLevel(const std::string &level);
		bool log(const zLevel * zLevelPtr,const char * pattern, ...) __attribute__((format(printf,3,4)));
		bool forceLog(const zLevel * zLevelPtr,const char * pattern, ...) __attribute__((format(printf,3,4)));
		bool debug(const char * pattern, ...) __attribute__((format(printf,2,3)));
		bool error(const char * pattern, ...) __attribute__((format(printf,2,3)));
		bool info(const char * pattern, ...) __attribute__((format(printf,2,3)));
		bool fatal(const char * pattern, ...) __attribute__((format(printf,2,3)));
		bool warn(const char * pattern, ...) __attribute__((format(printf,2,3)));
		bool alarm(const char * pattern, ...) __attribute__((format(printf,2,3)));
		bool iffy(const char * pattern, ...) __attribute__((format(printf,2,3)));
		bool trace(const char * pattern, ...) __attribute__((format(printf,2,3)));
		bool gbug(const char * pattern, ...) __attribute__((format(printf,2,3)));

		void setInfoM(zServerInfoManager * const info);
		bool errorM(const char * pattern, ...) __attribute__((format(printf,2,3)));
		bool fatalM(const char * pattern, ...) __attribute__((format(printf,2,3)));
		bool warnM(const char * pattern, ...) __attribute__((format(printf,2,3)));

		// 安全版的日志
		bool slog(const zLevel * zLevelPtr,const char * pattern, ...) __attribute__((format(printf, 3, 4)));
		bool sdebug(const char * pattern, ...) __attribute__((format(printf, 2, 3)));
		bool serror(const char * pattern, ...) __attribute__((format(printf, 2, 3)));
		bool sinfo(const char * pattern, ...) __attribute__((format(printf, 2, 3)));
		bool sfatal(const char * pattern, ...) __attribute__((format(printf, 2, 3)));
		bool swarn(const char * pattern, ...) __attribute__((format(printf, 2, 3)));
		bool salarm(const char * pattern, ...) __attribute__((format(printf, 2, 3)));
		bool strace(const char * pattern, ...) __attribute__((format(printf, 2, 3)));

	
	
	private:
		/**
		 * \brief 本类只为了能以正确的时间写文件名,而从log4cxx::DailyRollingFileAppender继承实现的
		 *
		 * 增加了#setTimeZone函数和重构了#activateOptions函数,其他的用法请参见log4cxx手册
		 */
		class zLoggerLocalFileAppender:public log4cxx::DailyRollingFileAppender
		{
			private:
				/**
				 * \brief 写日志文件名所用的时区
				 */
				static log4cxx::helpers::TimeZonePtr tz;
				log4cxx::String my_fileName;
				void my_rollOver();
				void subAppend(const log4cxx::spi::LoggingEventPtr& event);
			public:
				zLoggerLocalFileAppender();
				~zLoggerLocalFileAppender();

				void setMyFile(const log4cxx::String& file);
				void setTimeZone(const log4cxx::String &timeZone);
				void activateOptions();
		};

		class PowerLogger:public log4cxx::Logger
		{
			public:
				void setName(const log4cxx::String  &s)
				{
					name=s;
				}
				
			protected:
				PowerLogger(const log4cxx::String &s):Logger(s) { }

		};

		zServerInfoManager* m_info;
		log4cxx::LoggerPtr logger;
		char message[MSGBUF_MAX];
		zMutex msgMut;
};

#endif

class FuncLog
{
public:
        static void logger(BYTE type, const char* pFuncName, const char* desc="", DWORD num=1); 
};

//任务日志
class TaskLog
{
public:
        static void logger(DWORD accid, QWORD charid, const char* pUserName, DWORD userLevel, DWORD taskID, const char* pTaskName, BYTE op, const char* desc=""); 
};

class MemLog
{
public:
	static const BYTE Object = 1;
	static const BYTE SceneUser = 2;
	static const BYTE Scene = 3;
	static const BYTE UserSession = 4;
	static const BYTE SceneSession = 5;
	static const BYTE SceneNpc = 6;
	static const BYTE zSkill = 7;
	static void logger(BYTE type, const void* addr, bool newORdel, const char* desc, ...);
	static char message[MSGBUF_MAX];
};
#endif
