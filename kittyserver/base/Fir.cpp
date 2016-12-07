#include <stdlib.h>
#include <iostream>

#include "zMisc.h"
#include "Fir.h"
#include "zTime.h"

namespace Fir
{
    __thread unsigned int seedp = 0;
    __thread struct drand48_data nrand48_buffer;
    __thread unsigned short int rand48_seedp[3];

    std::map<std::string, unsigned long> thread_stack_addr;

    volatile QWORD qwGameTime = 0;

    volatile QWORD qwStartTime = 0;

    zLogger *logger = NULL;

    zProperties global;

    DWORD debug = 0;
    DWORD currPtr = 0;
    DWORD wait_size = 0;
    DWORD max_size = 0;
    DWORD function_times = 0;


    /**
     * \brief 初始化一些全局变量
     *
     */
    void initGlobal()
    {
        global["threadPoolCapacity"] = "2048";
        global["httpThreadPoolCapacity"] = "2048";
        Fir::global["server"] = "192.168.1.162";
        Fir::global["port"] = "10000";
        Fir::global["ifname"] = "eth0";
        Fir::global["mysql"] = "mysql://Fir:Fir@192.168.1.162:3306/Fir";
        Fir::global["log"] = "debug";
        zRTime::getLocalTZ();
    }
    /**
     * \brief 释放一些全局变量
     *
     */
    void finalGlobal()
    {
        //SAFE_DELETE(Fir::logger);
    }
};



