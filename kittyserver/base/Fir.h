#ifndef _Fir_h_
#define _Fir_h_

#include "zFirNew.h"
#include "zVersion.h"
#include "zProperties.h"
#include "zLogger.h"
#include "firstring.h"
#include "zSingleton.h"
#include "zVersion.h"

namespace Fir
{
	/**
	 * \brief 游戏时间
	 *
	 */
	extern volatile QWORD qwGameTime;
	/**
	 * \brief 游戏启动时时
	 */
	extern volatile QWORD qwStartTime;

	/**
	 * \brief 日志指针
	 *
	 */
	extern zLogger *logger;

	/**
	 * \brief 存取全局变量的容器
	 *
	 */
	extern zProperties global;

	/**
	 * \brief 存储一些全局调试信息
	 *
	 */
	extern DWORD debug;
	extern DWORD currPtr;
	extern DWORD size;
	extern DWORD wait_size;
	extern DWORD max_size;
	extern DWORD function_times; // 用来开启function_times功能
	extern std::map<std::string, unsigned long> thread_stack_addr;

	extern void initGlobal();
	extern void finalGlobal();

};
using namespace Fir;
#define LOG_ERR(format, ...) {\
    char line[255];\
    sprintf(line,"[%s:%d]",__FILE__,__LINE__);\
    strcat(line,format);\
    Fir::logger->error(line, ## __VA_ARGS__);\
}





#endif
