#include "key.h"

QWORD hashKey(const DWORD key1,const DWORD key2)
{
    return (key1 << 8) + key2;
}

bool randKey(const std::set<DWORD>& randSet,DWORD &ret)
{
    DWORD randVal = zMisc::randBetween(1,randSet.size());
    DWORD num = 1;
    for(auto iter = randSet.begin();iter != randSet.end();++iter,++num)
    {
        if(num == randVal)
        {
            ret = *iter;
            return true;
        }
    }
    return false;
}
 
QWORD hashRankKey(const DWORD key1,const QWORD key2)
{
    QWORD key = 0LL;
    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    key = tv.tv_sec;
    key *= 1000000LL;
    key += tv.tv_nsec;
    QWORD val = key1;
    val <<= 32;
    key += val;
    return key;
#if 0
    QWORD key = key2;
    QWORD val = key1;
    val <<= 32;
    key += val;
    val = zMisc::randBetween(1,10000);
    key += val;
    return key;
#endif
}

           
