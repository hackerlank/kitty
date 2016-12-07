/**
 * \file: RecordFamily.cpp
 * \version  $Id: RecordFamily.cpp 64 2013-04-23 02:05:08Z  $
 * \author  , @ztgame.com 
 * \date 2007年01月30日 06时37分12秒 CST
 * \brief 档案对象
 *
 * 
 */


#include "Fir.h"
#include "zDBConnPool.h"
#include "RecordFamily.h"
#include "zMemDB.h"
#include "SceneServer.h"
#include "RedisMgr.h"
#include "TimeTick.h"
#include "SceneUser.h"


bool RecordFamily::readFamily(QWORD familyID,CMD::RECORD::FamilyBase &m_base)
{
    using namespace CMD::RECORD;

    zMemDB* reshandle = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (reshandle==NULL) 
    {
        return false;
    }
    // 同步
    if (!reshandle->getBin("family", familyID, "familybase", (char*)&m_base))
    {
        Fir::logger->error("[读取家族],family失败,%lu",familyID);
        return false;
    }
    return true;
}

bool RecordFamily::readFamily(QWORD familyID,HelloKittyMsgData::BaseFamilyInfo &info,QWORD playerID)
{
    CMD::RECORD::FamilyBase baseInfo;
    if(!readFamily(familyID,baseInfo))
    {
        return false;
    } 
    trans(baseInfo,info,playerID);
    return true;



}

void RecordFamily::trans(const CMD::RECORD::FamilyBase &baseInfo,HelloKittyMsgData::BaseFamilyInfo &info,QWORD playerID)
{
    info.set_familyid(baseInfo.m_familyID);
    info.set_familyname(baseInfo.m_strName);
    info.set_familyicon(baseInfo.m_icon);
    info.set_personnum(GetMemSize(baseInfo.m_familyID));
    info.set_pulictype(HelloKittyMsgData::FamilyPublicType(baseInfo.m_limmitType));
    info.set_lowlevel(baseInfo.m_lowLevel);
    info.set_highlevel(baseInfo.m_highLevel);
    info.set_familylevel(baseInfo.m_level);
    info.set_ranking(baseInfo.m_ranking);
    info.set_lastranking(baseInfo.m_lastranking);
    info.set_familynotice(baseInfo.m_notice);
    info.set_totalscore(baseInfo.m_score);
    std::set<QWORD> setMember;
    getMember(setMember,baseInfo.m_familyID);
    info.set_personnum(setMember.size()); 
    if(getFamilyID(playerID) == baseInfo.m_familyID)
    {
        info.set_relation(1);
    }
    else if(checkPlayerIsApply(playerID,baseInfo.m_familyID))
    {
        info.set_relation(2);
    }
    else
    {
        info.set_relation(0);
    }
    HelloKittyMsgData::playerShowbase *base = info.mutable_leadershow();
    if(base != NULL)
        SceneUser::getplayershowbase(getFamilyLeader(baseInfo.m_familyID),*base);


}

QWORD  RecordFamily::getFamilyLeader(QWORD familyID)
{
    CMD::RECORD::FamilyBase m_base;
    if(!readFamily(familyID,m_base))
    {
        return 0;
    }
    return m_base.m_charid;


}


bool RecordFamily::update(const CMD::RECORD::FamilyBase &base)
{
    connHandleID handle = SceneService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }

    char where[128]={0};
    snprintf(where, sizeof(where) - 1, "f_familyID=%lu", base.m_familyID);
    unsigned int retcode = SceneService::dbConnPool->exeUpdate(handle, "t_family", record_family, (BYTE *)(&base), where);
    SceneService::dbConnPool->putHandle(handle);

    if (1 == retcode)
    {
        Fir::logger->trace("[家族读写],familyID=%lu", base.m_familyID);
    }
    else if (((unsigned int)-1) == retcode)
    {
        Fir::logger->error("[家族读写,失败],familyID=%lu", base.m_familyID);
        return false;
    }

    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(base.m_familyID);
    if (redishandle==NULL)
    {
        Fir::logger->error("[读取家族],获取内存数据库失败 familyID=%lu", base.m_familyID);
        return false;
    }

    if (!redishandle->setBin("family", base.m_familyID, "familybase",(const char*)&base, sizeof(CMD::RECORD::FamilyBase)))
    {
        Fir::logger->error("[读取家族],family失败,%lu",base.m_familyID);
        return false;
    }
    return true;
}



HelloKittyMsgData::FamilyOpResult RecordFamily::addMember(QWORD playerID,QWORD familyID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (handle==NULL) 
    {
        Fir::logger->error("add family member no DBHandle  %lu,%lu",familyID,playerID); 
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    if((QWORD)handle->getInt("family",playerID,"belong") > 0) 
    {
        Fir::logger->error("add family member belong err  %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_HasFamily;
    }
    removeApply(playerID);
    connHandleID DBhandle = SceneService::dbConnPool->getHandle();
    Record record;
    record.put("f_familyid",familyID);
    record.put("f_charid",playerID);  
    record.put("f_type",0);
    record.put("f_opTimer",SceneTimeTick::currentTime.sec());
    DWORD retcode = SceneService::dbConnPool->exeInsert(DBhandle, "t_familymember", &record);
    SceneService::dbConnPool->putHandle(DBhandle);
    if ((DWORD)-1 == retcode)
    {
        Fir::logger->error("add family member err %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(familyID);
    if(!handle2)
    {
        Fir::logger->error("add family member no DBHandle  %lu,%lu",familyID,playerID); 
        return HelloKittyMsgData::FamilyOpResult_OtherErr;

    }
    //设置一个家族有哪些玩家
    handle2->setSet("family",familyID,"include",playerID);
    //设置玩家属于哪个家族
    handle->setInt("family",playerID,"belong",familyID); 
    CMD::RECORD::FamilyMemberData data;
    handle->setBin("family", playerID, "familymemberdata", (const char*)&data, sizeof(CMD::RECORD::FamilyMemberData));
    return HelloKittyMsgData::FamilyOpResult_Suc;
}

HelloKittyMsgData::FamilyOpResult RecordFamily::addApply(QWORD playerID,QWORD familyID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (handle==NULL) 
    {
        Fir::logger->error("add family Apply no DBHandle  %lu,%lu",familyID,playerID); 
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    if((QWORD)handle->getInt("family",playerID,"belong") > 0) 
    {
        Fir::logger->error("add family Apply belong err  %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_HasFamily;
    }
    if((QWORD)handle->checkSet("family",playerID,"Apply",familyID)) 
    {
        Fir::logger->error("add family Apply apply ago err  %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_HasApply;
    }
    connHandleID DBhandle = SceneService::dbConnPool->getHandle();
    Record record;
    record.put("f_familyid",familyID);
    record.put("f_charid",playerID);  
    record.put("f_type",1);
    record.put("f_opTimer",SceneTimeTick::currentTime.sec());
    DWORD retcode = SceneService::dbConnPool->exeInsert(DBhandle, "t_familymember", &record);
    SceneService::dbConnPool->putHandle(DBhandle);
    if ((DWORD)-1 == retcode)
    {
        Fir::logger->error("add family Apply err %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (handle2==NULL) 
    {
        Fir::logger->error("add family Apply no DBHandle  %lu,%lu",familyID,playerID); 
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    //设置一个家族有哪些玩家申请
    handle2->setSet("family",familyID,"beApply",playerID);
    //设置玩家申请过哪个家族
    handle->setSet("family",playerID,"Apply",familyID); 
    return HelloKittyMsgData::FamilyOpResult_Suc2;
}

HelloKittyMsgData::FamilyOpResult RecordFamily::removeMember(QWORD playerID,QWORD familyID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (handle==NULL) 
    {
        Fir::logger->error("remove family member no MemDBHandle  %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    if((QWORD)handle->getInt("family",playerID,"belong") != familyID)
    {
        Fir::logger->error("remove family member no belong err  %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_NoFamily;

    }
    connHandleID DBhandle = SceneService::dbConnPool->getHandle();
    char where[128]={0};
    snprintf(where, sizeof(where) - 1, "f_familyid=%lu and f_charid=%lu", familyID,playerID);
    unsigned int retcode = SceneService::dbConnPool->exeDelete(DBhandle, "t_familymember",  where);
    SceneService::dbConnPool->putHandle(DBhandle);
    if ((DWORD)-1 == retcode)
    {
        Fir::logger->error("remove family member err %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    //设置一个家族有哪些玩家
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (handle2==NULL) 
    {
        Fir::logger->error("add family Apply no DBHandle  %lu,%lu",familyID,playerID); 
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    handle2->delSet("family",familyID,"include",playerID);
    //设置玩家属于哪个家族
    handle->setInt("family",playerID,"belong",0); 
    handle->del("family",playerID,"familymemberdata");
    return HelloKittyMsgData::FamilyOpResult_Suc;

}

HelloKittyMsgData::FamilyOpResult RecordFamily::removeApply(QWORD playerID,QWORD familyID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (handle==NULL) 
    {
        Fir::logger->error("remove family Apply no MemDBHandle  %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    if(!handle->checkSet("family",playerID,"Apply",familyID))
    {
        Fir::logger->error("remove family Apply no belong err  %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_NoApply;
    }
    connHandleID DBhandle = SceneService::dbConnPool->getHandle();
    char where[128]={0};
    snprintf(where, sizeof(where) - 1, "f_familyid=%lu and f_charid=%lu", familyID,playerID);
    unsigned int retcode = SceneService::dbConnPool->exeDelete(DBhandle, "t_familymember",  where);
    SceneService::dbConnPool->putHandle(DBhandle);
    if ((DWORD)-1 == retcode)
    {
        Fir::logger->error("remove family Apply err %lu,%lu",familyID,playerID);
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (handle2==NULL) 
    {
        Fir::logger->error("add family Apply no DBHandle  %lu,%lu",familyID,playerID); 
        return HelloKittyMsgData::FamilyOpResult_OtherErr;
    }
    //设置一个家族有哪些玩家
    handle2->delSet("family",familyID,"beApply",playerID);
    //设置玩家属于哪个家族
    handle->delSet("family",playerID,"Apply",familyID);  //设置一个家族有哪些玩家申请
    return HelloKittyMsgData::FamilyOpResult_Suc;

}

bool RecordFamily::doDel(QWORD familyID)
{
    connHandleID DBhandle = SceneService::dbConnPool->getHandle();
    char where[128]={0};
    snprintf(where, sizeof(where) - 1, "f_familyid=%lu", familyID);
    unsigned int retcode = SceneService::dbConnPool->exeDelete(DBhandle, "t_family",  where);
    SceneService::dbConnPool->putHandle(DBhandle);
    if ((DWORD)-1 == retcode)
    {
        Fir::logger->error("remove family  err %lu",familyID);
        return false;
    }
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (redishandle==NULL)
    {
        Fir::logger->error("[读取家族],获取内存数据库失败 familyID=%lu", familyID);
        return false;
    }

    //删除所有申请者
    std::set<QWORD> setApply;
    getApplyPlayer(setApply,familyID);
    for(auto iter = setApply.begin(); iter != setApply.end(); iter++)
    {
        removeApply(*iter,familyID);
    }
    zMemDB* redishandle2 = zMemDBPool::getMe().getMemDBHandle();
    if(!redishandle2)
    {
        Fir::logger->error("获取家族set MemDBHandle empty");
        return false;
    }
    redishandle2->delSet("family",0,"hasfamily",familyID);
    //删除排行榜数据
    CMD::RECORD::FamilyBase base;
    if(readFamily(familyID,base))
    {
        if(base.m_ranking != 0)
        {
            redishandle2->setInt("family",base.m_ranking,"Rank",0);
        }
    }
    if (!redishandle->del("family", familyID, "familybase"))
    {
        Fir::logger->error("[读取家族],family失败,%lu",familyID);
        return false;
    }
    return true;

}

DWORD RecordFamily::GetMemSize(QWORD familyID)
{
    std::set<QWORD> setMember;
    getMember(setMember,familyID);
    return setMember.size();
}

void  RecordFamily::getMember(std::set<QWORD> &setMember,QWORD familyID)
{
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (redishandle==NULL)
    {
        Fir::logger->error("[读取家族],获取内存数据库失败 familyID=%lu", familyID);
        return ;
    }
    if(!redishandle->getSet("family", familyID, "include", setMember))
    {
        Fir::logger->error("[读取家族 成员  familyID=%lu", familyID);
        return ;
    }

}

void  RecordFamily::getApplyPlayer(std::set<QWORD> &setApply,QWORD familyID)
{
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (redishandle==NULL)
    {
        Fir::logger->error("[读取家族],获取内存数据库失败 familyID=%lu", familyID);
        return ;
    }
    if(!redishandle->getSet("family", familyID, "beApply", setApply))
    {
        Fir::logger->error("[读取家族 成员  familyID=%lu", familyID);
        return ;
    }

}

QWORD RecordFamily::getFamilyID(QWORD playerID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (handle==NULL) 
    {
        LOG_ERR("getFamilyID(QWORD playerID) no MemDBHandle  %lu",playerID);
        return 0;
    }
    return (QWORD)handle->getInt("family",playerID,"belong"); 

}

QWORD  RecordFamily::getSelfApplySize(QWORD playerID)
{
    std::set<QWORD> setApply;
    getSelfApplyList(setApply,playerID);
    return setApply.size();
}

void  RecordFamily::getSelfApplyList(std::set<QWORD> &setApply,QWORD playerID)
{
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (redishandle==NULL)
    {
        Fir::logger->error("[读取家族],获取内存数据库失败 playerID=%lu", playerID);
        return ;
    }
    if(!redishandle->getSet("family", playerID, "Apply", setApply))
    {
        Fir::logger->error("[读取家族 成员  playerID %lu", playerID);
        return ;
    }

}

void RecordFamily::getFamilyList(std::set<QWORD> &setFamily,DWORD size)
{
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(); 
    if(!handle2)
    {
        Fir::logger->error("获取家族列表，MemDBHandle empty");
        return ;

    }
    if(size == 0)
    {
        if(!handle2->getSet("family", 0, "hasfamily", setFamily))
        {
            Fir::logger->error("获取家族列表，MemDBHandle get err");

        }
    }
    else
    {

        if(!handle2->getSet("family", 0, "hasfamily", setFamily,size))
        {
            Fir::logger->error("获取家族列表，MemDBHandle get err");

        }
    }
}

void RecordFamily::removeApply(QWORD playerID)
{
    std::set<QWORD> setApply;
    getSelfApplyList(setApply,playerID);

    for(auto iter = setApply.begin(); iter != setApply.end(); iter++)
    {
        std::set<QWORD> setApply;
        getSelfApplyList(setApply,playerID);
        removeApply(playerID,*iter);
    }
}

bool RecordFamily::checkHasfamily(QWORD familyID)
{
    zMemDB* redishandle2 = zMemDBPool::getMe().getMemDBHandle();
    if(!redishandle2)
    {
        Fir::logger->error("获取家族set MemDBHandle empty");
        return false;
    }
    return redishandle2->checkSet("family",0,"hasfamily",familyID);


}

bool RecordFamily::checkPlayerIsApply(QWORD playerID,QWORD familyID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (handle==NULL) 
    {
        Fir::logger->error("checkPlayerIsApply  %lu,%lu",familyID,playerID);
        return false;
    }
    return handle->checkSet("family",playerID,"Apply",familyID);
}


bool RecordFamily::readFamilyMemberBin(QWORD playerID,CMD::RECORD::FamilyMemberData &data)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (handle==NULL) 
    {
        return false;
    }
    if (handle->getBin("family", playerID, "familymemberdata", (char*)&data) == 0)
    {
        return false;
    }
    return true;

}

bool RecordFamily::updateMember(QWORD familyID,QWORD playerID,const CMD::RECORD::FamilyMemberData &data)
{
    connHandleID handle = SceneService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }

    Record where;
    std::ostringstream oss;
    oss << "f_familyID='" << familyID << "'";
    where.put("f_familyID", oss.str());
    oss.str(""); 
    oss << "f_charid='" << playerID << "'";
    where.put("f_charid", oss.str());
    Record record;
    record.put("f_contributionlast",data.m_contributionlast);
    record.put("f_contributionranklast",data.m_contributionranklast); 
    record.put("f_isgetaward",data.m_isgetaward);
    record.put("f_contribution",data.m_contribution);
    unsigned int retcode = SceneService::dbConnPool->exeUpdate(handle, "t_familymember", &record,  &where);
    SceneService::dbConnPool->putHandle(handle);
    if (((unsigned int)-1) == retcode)
    {
        Fir::logger->error("[家族读写,失败],familyID=%lu", familyID);
        return false;
    }
    zMemDB* handle3 = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (handle3) 
    {    
        handle3->setBin("family", playerID, "familymemberdata", (const char*)&data, sizeof(CMD::RECORD::FamilyMemberData));
    }
    return true;

}



