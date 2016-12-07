#ifndef _REDIS_MGR_H
#define _REDIS_MGR_H

#include "zSingleton.h"
#include "CharBase.h"
#include "serialize.pb.h"
#include "unitbuild.pb.h"
#include "rank.pb.h"
#include <set>

struct stPuppet
{
    CharBase charbase;
    HelloKittyMsgData::Serialize binary;
};

class RedisMgr : public Singleton<RedisMgr>
{
    friend class Singleton<RedisMgr>;
    public:
    virtual ~RedisMgr(){}

    bool get_charbase(const QWORD charid, CharBase& charbase); 
    bool get_binary(const QWORD charid, HelloKittyMsgData::Serialize& binary);
    bool get_binary(const QWORD charid, const char* input_buf, DWORD input_size, HelloKittyMsgData::Serialize& binary);
    bool get_puppet(const QWORD charid, stPuppet& puppet);
    bool is_login_first(const QWORD charid);
    bool get_unitybuildata(const QWORD f_id,HelloKittyMsgData::UnitRunInfo &binary);
    bool set_unitybuildata(HelloKittyMsgData::UnitRunInfo &binary);
    bool get_unitybuildcol(const QWORD playerID,std::set<DWORD> &setUse,BYTE type);//0所有栏位，1主动栏位，2被动栏位
    void get_unitybuildOnlySet(const QWORD playerID,const DWORD BuildID,std::set<QWORD> &setOnlyId);
    //根据栏位得到信息
    QWORD get_unitybuilddatabyColId(const QWORD playerID,const DWORD colID);
    bool get_unitybuildrankdata(const QWORD charid,HelloKittyMsgData::MaxUnityBuild &binary);
    bool set_unitybuildrankdata(const QWORD charid,HelloKittyMsgData::MaxUnityBuild &binary);
    bool del_unitybuildrankdata(const QWORD charid);

    private:
    RedisMgr(){}
};

#endif
