#ifndef __UTIL_UUID_H__
#define __UTIL_UUID_H__

#include <stdint.h>
#include "zType.h"
namespace utils
{
    // twitter snowflake算法
    // 64       63--------------22---------12---------0
    // 符号位   |     41位时间   |10位机器码|12位自增码|
    extern QWORD get_time();

    class unique_id_t
    {
        public:
            unique_id_t();
            ~unique_id_t();
            static QWORD generate(QWORD type,DWORD serverid);

        private:
            static DWORD sequence_;
    };

}

#endif // !__UTIL_UUID_H__
