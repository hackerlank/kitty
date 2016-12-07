#include "SceneUser.h"
#include "zMetaData.h"
#include <stdarg.h>
#include "SceneServer.h"
#include "zMetaData.h"
#include "TimeTick.h"
#include "SceneUserManager.h"
#include <zlib.h>
#include <bitset>
#include "RecordClient.h"
#include "LoginUserCommand.h"
#include "xmlconfig.h"
#include <limits>
#include "ResType.h"
#include "RedisMgr.h"
#include "json/json.h"
#include "login.pb.h"
#include "extractProtoMsg.h"
#include "serialize.pb.h"
#include "dataManager.h"
#include "tbx.h"
#include "SceneMapDataManager.h"
#include "enterkitty.pb.h"
#include "RecordFamily.h"
#include "Misc.h"

bool SceneUser::forBid(const DWORD time,const char *reason)
{
    zMemDB* tempHandle = zMemDBPool::getMe().getMemDBHandle(charid);
    connHandleID handle = SceneService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle || !handle)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return false;
    }
    bool update = false;
    DWORD endTime = tempHandle->getInt("forbid",charid,"forbidtime");
    if(endTime)
    {
        update = true;
    }

    Record record,where;
    record.put("charid",charid);
    record.put("endtime",time);
    record.put("reason",reason);
    
    std::ostringstream temp;
    temp << "charid=" << charid;
    where.put("charid",temp.str());
    
    DWORD retcode = update ? SceneService::dbConnPool->exeUpdate(handle, "t_forbid", &record,&where) : SceneService::dbConnPool->exeInsert(handle, "t_forbid", &record);
    SceneService::dbConnPool->putHandle(handle);
    if(retcode == (DWORD)-1)
    {
        Fir::logger->debug("[封号] 数据库操作出错(%lu,%u,%s)",charid,time,reason);
        return false;
    }
    tempHandle->setInt("forbid",charid,"forbidtime",time);
    DWORD now = SceneTimeTick::currentTime.sec();
    if(now < time)
    {
        HelloKittyMsgData::AckForBid ackForBid;
        ackForBid.set_reason("被封号");
        std::string ret;
        encodeMessage(&ackForBid,ret);
        sendCmdToMe(ret.c_str(),ret.size());

        //踢下线
        CMD::SCENE::t_UserKickOff kickOff;
        kickOff.charID = charid;
        ret.clear();
        encodeMessage(&kickOff,sizeof(kickOff),ret);
        sendCmdToGateway(ret.c_str(),ret.size());
        unreg();
    }
    return true;
}

bool SceneUser::forBidSys(const DWORD time,const char *reason)
{
    zMemDB* tempHandle = zMemDBPool::getMe().getMemDBHandle(charid);
    connHandleID handle = SceneService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle || !handle)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return false;
    }
    bool update = false;
    DWORD endTime = tempHandle->getInt("forbidsys",charid,"forbidtime");
    if(endTime)
    {
        update = true;
    }

    Record record,where;
    record.put("charid",charid);
    record.put("endtime",time);
    record.put("reason",reason);
    
    std::ostringstream temp;
    temp << "charid=" << charid;
    where.put("charid",temp.str());
    
    DWORD retcode = update ? SceneService::dbConnPool->exeUpdate(handle, "t_forbidsys", &record,&where) : SceneService::dbConnPool->exeInsert(handle, "t_forbidsys", &record);
    SceneService::dbConnPool->putHandle(handle);
    if(retcode == (DWORD)-1)
    {
        Fir::logger->debug("[禁言] 数据库操作出错(%lu,%u,%s)",charid,time,reason);
        return false;
    }
    tempHandle->setInt("forbidsys",charid,"forbidtime",time);
    DWORD now = SceneTimeTick::currentTime.sec();
    if(now < time)
    {
        HelloKittyMsgData::AckForBid ackForBid;
        ackForBid.set_reason(reason);
        ackForBid.set_optype(1);

        std::string ret;
        encodeMessage(&ackForBid,ret);
        sendCmdToMe(ret.c_str(),ret.size());
    }
    return true;
}


bool SceneUser::isForbidSys()
{
    const QWORD charID = charid;
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charID);
    if(!handle)
    {
        return false;
    }
    DWORD endTime = handle->getInt("forbidsys",charID,"forbidtime");
    DWORD now = SceneTimeTick::currentTime.sec();
    if(now < endTime)
    {
        HelloKittyMsgData::AckForBid ackForBid;
        ackForBid.set_reason("被禁言");
        ackForBid.set_optype(1);

        std::string ret;
        encodeMessage(&ackForBid,ret);
        sendCmdToMe(ret.c_str(),ret.size());
        return false;
    }
    return true;
}





