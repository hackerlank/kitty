#include "uuid.h"
#include <sys/time.h>
#include <unistd.h>

namespace utils
{
    QWORD get_time()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        QWORD time = tv.tv_usec;
        time /= 1000;
        time += (tv.tv_sec * 1000);
        return time;
    }

    unique_id_t::unique_id_t()
    {
        sequence_ = 0;
    }

    unique_id_t::~unique_id_t()
    {

    }


 
    DWORD unique_id_t::sequence_ = 0;
    QWORD unique_id_t::generate(QWORD type,DWORD serverid)
    {
        QWORD value = 0;
        QWORD time = get_time() - type;
        // 保留后41位时间
        value = time << 22;

        // 中间10位是机器ID
        value |= (serverid & 0x3FF) << 12;

        // 最后12位是sequenceID
        value |= sequence_++ & 0xFFF;
        if (sequence_ == 0x1000)
        {
            sequence_ = 0;
        }

        return value;
    }
}
