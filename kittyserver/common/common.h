#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include "Fir.h"
#include "zDBConnPool.h"

#pragma pack(1)

namespace CMD
{ 
    struct stSaleCell
    {
        DWORD cellid;
        DWORD itemid;
        DWORD itemcount;
        DWORD status; // 0空 1放上物品 2已售
        char nickname[MAX_NAMESIZE]; // 购买者的名字
        stSaleCell()
        {
            bzero(this,sizeof(*this));
        }
    };

}
const dbCol activedb[] = { 
    { "f_type",     zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_subtype",     zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_id",     zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_begintime",     zDBConnPool::DB_DWORD,  sizeof(DWORD) },  
    { "f_endtime",    zDBConnPool::DB_DWORD,  sizeof(DWORD)  }, 
    { "f_condition",    zDBConnPool::DB_DWORD,    sizeof(DWORD) }, 
    { "f_conditionparam",   zDBConnPool::DB_DWORD,    sizeof(DWORD) },
    { "f_preactive",   zDBConnPool::DB_DWORD,    sizeof(DWORD) },
    { "f_award",      zDBConnPool::DB_STR,  100 },
    { "f_title",      zDBConnPool::DB_STR,  20 },
    { "f_desc",      zDBConnPool::DB_STR,  100 },
    { "f_open",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_platemsg",      zDBConnPool::DB_STR,  100 },
    { "f_rewardmaxcnt",     zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_rewardcurcnt",     zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_subcondition",     zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { NULL, 0, 0}
};
struct activedata
{
    DWORD f_type;
    DWORD f_subtype;
    DWORD f_id;
    DWORD  f_begintime;
    DWORD f_endtime;
    DWORD f_condition;
    DWORD f_conditionparam;
    DWORD f_preactive;
    char f_award[100];
    char f_title[20];
    char f_desc[100];
    DWORD f_open;
    char f_platemsg[100];
    DWORD f_rewardmaxcnt;
    DWORD f_rewardcurcnt;
    DWORD f_subcondition;

    activedata()
    {
        f_type = 0;
        f_subtype = 0;
        f_id = 0 ;
        f_begintime = 0 ;
        f_endtime = 0 ;
        f_condition = 0 ;
        f_conditionparam = 0 ;
        f_preactive = 0 ;
        f_open = 0 ;
        memset(f_award,0,sizeof(f_award));
        memset(f_title,0,sizeof(f_title));
        memset(f_desc,0,sizeof(f_desc));
        bzero(f_platemsg,sizeof(f_platemsg));
        f_rewardmaxcnt = 0;
        f_rewardcurcnt = 0;
        f_subcondition = 0;
    }

}__attribute__ ((packed));


#pragma pack()
#define ISSTATICNPC(ID) (ID > 0 && ID <= 10)
#define ACTIVECNPCMin 11
#define ACTIVECNPCMAx 100
#define ISACTIVECNPC(ID) (ID >= ACTIVECNPCMin && ID <= ACTIVECNPCMAx)
enum EPLAYERFLAG
{
    EPLAYERFLAG_TRAINHELP = 1,
};
#define SETFLAG(rData,flag) {QWORD bit = QWORD(1) << flag; rData |= bit;}
#define DELFLAG(rData, flag) {QWORD bit = QWORD(1) << flag; QWORD mark = 0;mark = (~mark) ^ bit;rData &= mark;}
#define CHECKFLAG(rData, flag)   (rData & (QWORD(1) << flag) > 0)
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#endif
