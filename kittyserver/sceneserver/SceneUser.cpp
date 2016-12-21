//角色类一般函数实现

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
#include "gmtool.pb.h"
#include "serialize.pb.h"
#include "dataManager.h"
#include "tbx.h"
#include "SceneTaskManager.h"
#include "SceneMail.h"
#include "SceneToOtherManager.h"
#include "chat.pb.h"
#include "key.h"
#include "buffer.h"

const DWORD DelCount = 30;
std::map<DWORD,std::vector<pb::ThreeArgPara> > SceneUser::s_randMap;
SceneUser::SceneUser(SceneTask *t,QWORD setCharid) :charid(setCharid), m_trade(this),m_store_house(this),m_buildManager(this),m_taskManager(this),m_atlasManager(this),m_achievementManager(this),m_emailManager(this),m_dressManager(this),m_paperManager(this),m_burstEventManager(this),m_giftPackage(this),m_addressManager(this),m_activeManger(*this),m_eventmanager(*this),m_friend(*this),m_kittyGarden(this),m_orderManager(*this),m_leavemessage(*this),m_managertrain(*this),m_managerordersystem(*this),m_managersignin(*this),m_market(*this),m_unitybuild(*this),m_active(*this)
{
    m_groupID = 0;
    accid = 0;
    needSave = false;
    _online = false;
    bzero(lastSaveCharBaseMD5,16);
    bzero(lastSaveMD5,16);
    VisitID = 0 ;
    m_LastCheckTimer = 0;
    initCarnivalShopData();
    autoBidFlg = false;
    if(t)
        m_gateid = t->getID();
    else 
        m_gateid = 0;
}

SceneUser::~SceneUser()
{
}

bool SceneUser::sendCmdToMe(const void *pstrCmd, const DWORD nCmdLen) const
{
    if ((DWORD)nCmdLen > zSocket::MAX_DATASIZE)
    {
        Fir::logger->error("发送消息过大:%d",nCmdLen);
        return false;
    }
    SceneTask *gate = getGate();
    if(gate)
    {
        return gate->sendCmdToUser(charid, pstrCmd, nCmdLen);
    }

    return false;
}

bool SceneUser::sendCmdToGateway(const void *pstrCmd, const DWORD nCmdLen) const
{
    if ((DWORD)nCmdLen > zSocket::MAX_DATASIZE)
    {    
        Fir::logger->error("发送消息过大:%d",nCmdLen);
        return false;
    } 
    SceneTask *gate = getGate(); 
    if(gate)
    {
        return gate->sendCmd(pstrCmd,nCmdLen);
    }

    return false;
}


DWORD SceneUser::getStoreLimit()
{
    return m_store_house.getAttr(HelloKittyMsgData::Attr_Ware_Grid_Val);
}

bool SceneUser::loop()
{
    bool candel = false;
    do{
        if(_online)
        {
            break ;
        }
        if(!mapVisit.empty())
        {
            break;
        }
        if(!m_eventmanager.canDel())
        {
            break;
        }
        if(autoBidFlg)
        {
            break;
        }
        candel = true;
    }while(0);
    if(candel)
    {
        m_LastCheckTimer++;
    }
    else
    {
        m_LastCheckTimer = 0;
    }
    if(m_LastCheckTimer < DelCount)
    {
        loopBuffer(this);
        m_activeManger.timerCheck();
        m_eventmanager.timerCheck();   
        m_buildManager.loop();
        loopCarnivalBox();
        //m_orderManager.timerCheck();
        m_kittyGarden.loop();
        m_managertrain.timerCheck();
        m_managerordersystem.timerCheck();
        m_market.timerCheck();
        m_unitybuild.timercheck();
        m_active.timerCheck();
        return true;
    }
    else
    {
        DWORD now = SceneTimeTick::currentTime.sec();
        if(!(charbase.offlinetime && now - charbase.offlinetime >= 10 * 60))
        {
            return true;
        }
    }
    return false;
}

void SceneUser::AddVisit(QWORD PlayerID,DWORD GateId)
{
    auto iter = mapVisit.find(PlayerID);
    if (iter == mapVisit.end() || iter->second != GateId)
    {
        DoFirstVisit(PlayerID,GateId);
        notifyGarden(PlayerID);

    }
    if(iter != mapVisit.end())
    {
        for(auto it = mapVisitTimer.begin(); it != mapVisitTimer.end();it++)
        {
            if(it->second == PlayerID)
            {
                mapVisitTimer.erase(it);
                break;

            }

        }

    }
    mapVisit[PlayerID] = GateId;
    DWORD now = SceneTimeTick::currentTime.sec();
    mapVisitTimer[now] = PlayerID;
    m_friend.PlayerVisit(PlayerID);
    HelloKittyMsgData::AckAddGuest ack;
    ack.set_charid(charid);
    HelloKittyMsgData::guestinfo *pinfo = ack.mutable_guardguest();
    if(pinfo)
    {
        pinfo->set_visittimer(now);
        HelloKittyMsgData::playerShowbase *pShow = pinfo->mutable_guest();
        if(pShow)
        {
            SceneUser::getplayershowbase(PlayerID,*pShow);
        }
    }
    std::string ret;
    encodeMessage(&ack,ret); 
    broadcastMsgInMap(ret.c_str(),ret.size());
    BroadCastPersonNum();
} 

bool SceneUser::isDailyData()
{
    DWORD now = SceneTimeTick::currentTime.sec();
    struct tm tm_old,tm_now;
    zRTime::getLocalTime(tm_now, now);
    zRTime::getLocalTime(tm_old, charbin.dailydata().dailydatatime());
    if(tm_old.tm_year != tm_now.tm_year || tm_old.tm_mon != tm_now.tm_mon || tm_old.tm_mday != tm_now.tm_mday)
    {
        return true;
    }
    return false;

}

bool SceneUser::flushGardenMsg(const DWORD GateId,const QWORD sendCharID)
{

    HelloKittyMsgData::AckAllGuest ackall;
    ackall.set_charid(charid);
    for(auto it =mapVisitTimer.rbegin(); it != mapVisitTimer.rend();it++) 
    {
        HelloKittyMsgData::guestinfo *info = ackall.add_guardguest();
        if(info != NULL)
        {
            info->set_visittimer(it->first);
            HelloKittyMsgData::playerShowbase *pShow = info->mutable_guest();
            if(pShow)
            {
                SceneUser::getplayershowbase(it->second,*pShow);
            }

        }
    }
    std::string retall;
    encodeMessage(&ackall,retall); 
    SceneTaskManager::getMe().broadcastUserCmdToGateway(GateId,sendCharID,retall.c_str(),retall.size());

    HelloKittyMsgData::AckEnterGarden ackEnterGarden;
    flushKittyGardenInfo(sendCharID,ackEnterGarden);

    std::string ret;
    encodeMessage(&ackEnterGarden,ret);
    return SceneTaskManager::getMe().broadcastUserCmdToGateway(GateId,sendCharID,ret.c_str(),ret.size());
}

void SceneUser::DelVisit(QWORD PlayerID)
{
    mapVisit.erase(PlayerID);
    for(auto it = mapVisitTimer.begin();it != mapVisitTimer.end();it++)
    {
        if(it->second == PlayerID)
        {
            mapVisitTimer.erase(it);
            break;
        }
    }
    HelloKittyMsgData::AckDelGuest ack;
    ack.set_charid(charid);
    ack.set_guestid(PlayerID);
    std::string ret;
    encodeMessage(&ack,ret); 
    broadcastMsgInMap(ret.c_str(),ret.size());

    BroadCastPersonNum();
}

bool SceneUser::isVisit(QWORD PlayerID)
{
    return mapVisit.find(PlayerID) != mapVisit.end();
}

//第一次访问家园
void SceneUser::DoFirstVisit(QWORD PlayerID,DWORD GateId)
{
    flushGardenMsg(GateId,PlayerID);
}

//回到自己的家园
void SceneUser::UserReturn()
{
#if 0
    flushGardenMsg(m_gateid,charid);
#endif
}
//建筑操作
void SceneUser::opBuild(QWORD PlayerID,const HelloKittyMsgData::Builditype &rBuild)//建筑操作
{
    //不是自己，也不是访问者，不可以操作
    if(charid != PlayerID && mapVisit.find(PlayerID) == mapVisit.end())
    {
        HelloKittyMsgData::AckopBuilding ack;
        ack.set_result(HelloKittyMsgData::PlayerOpEventResult_NOVisit);
        ack.set_charid(PlayerID);
        HelloKittyMsgData::Builditype *pbuild =  ack.mutable_build();
        if(pbuild)
        {
            *pbuild = rBuild;
        }
        std::string ret;
        encodeMessage(&ack,ret);
        SceneTaskManager::getMe().broadcastUserCmdToGateway(PlayerID,ret.c_str(),ret.size());

    }
    else
    {
        HelloKittyMsgData::AckopBuilding ack;
        ack.set_charid(charid);
        HelloKittyMsgData::Builditype *pbuild =  ack.mutable_build();
        if(pbuild)
        {
            *pbuild = rBuild;
        }
        ack.set_result(HelloKittyMsgData::PlayerOpEventResult_Suc); 
        DWORD eventId = 0;
        //bool   needNotice = false;
        do{
            if(rBuild.isicon() == 0)
            {
                BuildBase* pBuild = m_buildManager.getBuild(rBuild.buildid());
                if(pBuild && pBuild->isTypeBuild(Build_Type_Road))
                {
                    m_activeManger.OpRoad(PlayerID,rBuild.buildid(),ack); 
                    break;
                }
            }
            m_eventmanager.OpRoad(PlayerID,rBuild,ack);
            if(ack.result() == HelloKittyMsgData::PlayerOpEventResult_Suc && PlayerID != charid)
            {
                //查看本服玩家
                SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerID);
                if(user)
                {
                    user->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_helpotherevent_Num,1,"help other event",true);
                    break;
                }
                zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(PlayerID);
                if(!handle2)
                {
                    Fir::logger->error("找不到Mem,%lu",PlayerID % MAX_MEM_DB+1);
                    break;

                }
                DWORD SenceId = handle2->getInt("playerscene",PlayerID,"sceneid");
                if(SenceId != 0)
                {
                    Fir::logger->error("寻找其他server处理 ，id %d",SenceId);
                    BYTE buf[zSocket::MAX_DATASIZE] = {0};
                    CMD::SCENE::t_addOrConsumeItem *sendCmd = (CMD::SCENE::t_addOrConsumeItem *)buf;
                    constructInPlace(sendCmd);
                    sendCmd->charID = PlayerID;
                    sendCmd->ItemID = HelloKittyMsgData::Attr_helpotherevent_Num;
                    sendCmd->ItemNum = 1;
                    sendCmd->bIsAdd = true;
                    sendCmd->bjudgeFull = false;
                    snprintf(sendCmd->remark,80,"help other event");
                    std::string ret;
                    encodeMessage(sendCmd,sizeof(*sendCmd),ret);
                    if(SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
                    {
                        break;
                    }

                }
            }

        }while(0);

        //发奖励
        if(ack.process() == HelloKittyMsgData::EventProcess_final && ack.result() == HelloKittyMsgData::PlayerOpEventResult_Suc)
        {

            if(ack.award_size() > 0)
            {
                HelloKittyMsgData::vecAward rvecaward;
                for(int i = 0 ;i < ack.award_size();i++)
                {
                    HelloKittyMsgData::Award* pawrd =  ack.mutable_award(i);
                    if(pawrd)
                    {
                        HelloKittyMsgData::Award* padd =  rvecaward.add_award();
                        if(padd)
                        {
                            *padd = *pawrd;
                        }


                    }
                }
                DoAward(PlayerID,rvecaward,eventId);//不发通知，因为返回里已经有通知

            }
        }
        Fir::logger->debug("AckopBuilding player %lu  ower %lu build %lu result %d eventid %d process %d award %d",PlayerID,charid,ack.build().buildid(),ack.result(),ack.eventid(),ack.process(),ack.award_size());
        std::string ret;
        encodeMessage(&ack,ret);
        returnMsgInMap(PlayerID,ret.c_str(),ret.size());

    }



}
void SceneUser::DoAward(QWORD PlayerID, const HelloKittyMsgData::vecAward& rvecAward,DWORD EventID,bool NeedNotice)
{
    do{

        if(PlayerID == charid)
        {
            DoBuildAward(rvecAward,EventID,charid,NeedNotice);
            break;

        }
        //查看本服玩家
        SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerID);
        if(user)
        {
            user->DoBuildAward(rvecAward,EventID,charid,NeedNotice);
            break;
        }
        zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(PlayerID);
        if(!handle2)
        {
            Fir::logger->error("找不到Mem,%lu",PlayerID % MAX_MEM_DB+1);
            break;

        }

        DWORD SenceId = handle2->getInt("playerscene",PlayerID,"sceneid");
        if(SenceId != 0)
        {
            Fir::logger->error("寻找其他server处理 ，id %d",SenceId);
            BYTE buf[zSocket::MAX_DATASIZE] = {0};
            CMD::SCENE::t_awardtoPlayer_scenescene *sendCmd = (CMD::SCENE::t_awardtoPlayer_scenescene *)buf;
            constructInPlace(sendCmd);
            sendCmd->charid = PlayerID;
            sendCmd->charowner = charid;
            sendCmd->eventid = EventID;
            sendCmd->bnotice = NeedNotice;
            sendCmd->size = rvecAward.ByteSize();
            rvecAward.SerializeToArray(sendCmd->data,sendCmd->size);
            std::string ret;
            encodeMessage(sendCmd,sizeof(*sendCmd) + sendCmd->size,ret);
            if(SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
            {
                break;
            }

        }
        //玩家无实例，发邮件 
        SceneMailManager::getMe().sendSysMailToPlayerForEvent(PlayerID,EventID,charid,rvecAward);

    }while(0);

}

void SceneUser::sendhappenevent(const HelloKittyMsgData::PlayerEvent &event)
{
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(charid);
    if(handle2)
    {
        if(event.eventid() == 0)
        {
            handle2->del("plyerevent",charid,NULL);
            m_friend.eventclose();
        }
        else 
        {
            char tepbuf[zSocket::MAX_DATASIZE];
            DWORD size = handle2->getBin("plyerevent", charid, tepbuf); 
            if(size == 0)
            {
                event.SerializeToArray(tepbuf,event.ByteSize());  
                handle2->setBin("plyerevent", charid, tepbuf,event.ByteSize());
                m_friend.eventhappen();

            }


        }
    }
}

bool SceneUser::brushDailyData()
{
    //日常临时数据
    initDailyData();
    //重置日常任务
    m_taskManager.resetDailyTask();
    //npc摊位
    m_trade.randNpcStallDay();
    return true;
}

void SceneUser::initDailyData()
{
    HelloKittyMsgData::DailyData *dailyData = charbin.mutable_dailydata();
    if(!dailyData)
    {
        return;
    }
    dailyData->set_dailydatatime(SceneTimeTick::currentTime.sec());     
    dailyData->set_addgold(0);
    dailyData->set_addgem(0);
    dailyData->set_addexp(0);
    dailyData->set_addhappy(0);
    dailyData->set_addfriend(0);
    dailyData->set_vistorother(0);
    dailyData->set_addatlas(0);
    dailyData->set_finishburstevent(0);
    dailyData->set_buildorlevel(0);
    dailyData->set_randtoy(0);
    dailyData->set_todayprivatelmnum(0);
    dailyData->set_costgem(0);
    dailyData->set_finishtask(0);
    dailyData->set_ordervalue(0);
    dailyData->set_compositeitem(0);
    dailyData->set_tradenum(0);
    dailyData->set_trainget(0);
    dailyData->set_helptrain(0);
    dailyData->set_finishdailytask(0);
    dailyData->set_orderaccept(0);
    dailyData->set_familyordernum(0);
    dailyData->set_cointoytime(0);
    charbin.set_finisnorder(0);

    HelloKittyMsgData::DivineData *divine = dailyData->mutable_divine();
    if(!divine)
    {
        return;
    }
    divine->set_answer(0);
    divine->set_lucklevel(0);
    divine->set_randorder("");
    divine->set_randtime(0);
    divine->set_firstkey(0);
    divine->set_status(HelloKittyMsgData::DS_Begin);
    divine->set_bufferid(0);
    while(dailyData->stardata_size() != 3)
    {
        HelloKittyMsgData::StarData *starInfo = dailyData->add_stardata();
        if(!starInfo)
        {
            return;
        }
        starInfo->set_cnt(0);
        starInfo->set_curstep(0);
    }
    for(int cnt = 0;cnt < dailyData->stardata_size();++cnt)
    {
        HelloKittyMsgData::StarData *starInfo = const_cast<HelloKittyMsgData::StarData*>(&(dailyData->stardata(cnt)));
        if(!starInfo)
        {
            return;
        }
        starInfo->set_cnt(0);
        starInfo->set_curstep(0);
        starInfo->set_startype(HelloKittyMsgData::StarType(cnt));
        DWORD step = 1;
        while(DWORD(starInfo->stepdata_size()) < pb::Conf_t_StarReward::maxLevel)
        {
            HelloKittyMsgData::StarStepData *stepData = starInfo->add_stepdata();
            if(stepData)
            {
                stepData->set_begintime(0);
                stepData->set_step(step);
                stepData->set_sec(0);
            }
            step += 1;
        }
    }

    HelloKittyMsgData::SuShiData *suShiInfo = dailyData->mutable_sushidata();
    if(!suShiInfo)
    {
        return;
    }
    suShiInfo->set_cnt(0);
    suShiInfo->set_curstep(0);
    int cnt = 1;
    while(DWORD(suShiInfo->stepdata_size()) < pb::Conf_t_SushiLevel::maxLevel)
    {
        HelloKittyMsgData::SuShiStepData *stepData = suShiInfo->add_stepdata();
        if(stepData)
        {
            stepData->set_step(cnt);
            stepData->set_gole(0);
        }
        cnt += 1;
    }
}

void SceneUser::resetAllData()
{
    charbase.onlinetime = 0;
    HelloKittyMsgData::Serialize ret;
    setupBinaryArchive(ret);
#if 0
    m_store_house.reset();
    m_buildManager.reset();
    m_taskManager.reset();
    m_atlasManager.reset();
    m_achievementManager.reset();
    m_paperManager.reset();
    m_burstEventManager.reset();
    charbin.Clear();
#endif
    onlineInit(false,getGate());
}

bool SceneUser::changeTime()
{
    //工人cd
    m_buildManager.flushWorker();
    if(!isDailyData())
    {
        return false;
    }
    brushDailyData();
    return true;
}

bool   SceneUser::havePersonOnline()
{
    return  is_online() || haveVisit();
}
bool  SceneUser::haveVisit()
{
    return !mapVisit.empty();
}

bool  SceneUser::checkPush(const HelloKittyMsgData::vecAward& rvecAward)
{
    std::map<DWORD,DWORD> itemMap;
    HelloKittyMsgData::vecAward rItem;
    for(int i = 0; i != rvecAward.award_size();i++)
    {
        const HelloKittyMsgData::Award& rcofig = rvecAward.award(i);
        if(rcofig.awardtype() < HelloKittyMsgData::Attr_Max)
        {
            continue;
        }
        HelloKittyMsgData::Award* pAward = rItem.add_award();
        if(pAward)
        {
            *pAward = rcofig;
        }
        if(itemMap.find(rcofig.awardtype()) == itemMap.end())
        {
            itemMap.insert(std::pair<DWORD,DWORD>(rcofig.awardtype(),rcofig.awardval()));
        }
        else
        {
            itemMap[rcofig.awardtype()] += rcofig.awardval();
        }
    }
    return m_store_house.hasEnoughSpace(itemMap);


}

bool  SceneUser::pushItem(const HelloKittyMsgData::vecAward& rvecAward,const char *reMark)
{
    HelloKittyMsgData::vecAward rItem;
    for(int i = 0; i != rvecAward.award_size();i++)
    {
        const HelloKittyMsgData::Award& rcofig = rvecAward.award(i);
        if(!m_store_house.addOrConsumeItem(rcofig.awardtype(),rcofig.awardval(),reMark,true))
        {
            break;
        }
    }
    return true;


}

bool  SceneUser::pushItemWithoutCheck(const HelloKittyMsgData::vecAward& rvecAward,const char *reMark)
{
    HelloKittyMsgData::vecAward rItem;
    for(int i = 0; i != rvecAward.award_size();i++)
    {
        const HelloKittyMsgData::Award& rcofig = rvecAward.award(i);
        if(!m_store_house.addOrConsumeItem(rcofig.awardtype(),rcofig.awardval(),reMark,true,false))
        {
            break;
        }
    }
    return true;


}


void SceneUser::DoBuildAward(const HelloKittyMsgData::vecAward& rvecAward,DWORD EventID,QWORD owerid,bool NeedNotice)
{
    char buff[255];
    sprintf(buff,"BuildOp EventID :%u owerid :%lu",EventID,owerid);
    if(is_online())
    {
        bool bCanPush  = checkPush(rvecAward);
        if(bCanPush)
        {
            pushItem(rvecAward,buff);
            if(NeedNotice)
            {
                HelloKittyMsgData::returnEventAward ack;
                ack.set_charid(owerid);
                ack.set_eventid(EventID);
                ack.set_ismail(0);
                for(int i = 0; i != rvecAward.award_size();i++)
                {
                    const HelloKittyMsgData::Award& rcofig = rvecAward.award(i);
                    HelloKittyMsgData::Award* pAward = ack.add_award();
                    if(pAward)
                    {
                        *pAward = rcofig;
                    }
                }
                std::string ret;
                encodeMessage(&ack,ret);
                sendCmdToMe(ret.c_str(),ret.size());


            }

        }
        else
        {
            SceneMailManager::getMe().sendSysMailToPlayerForEvent(charid,EventID,owerid,rvecAward);
            if(NeedNotice)
            {
                HelloKittyMsgData::returnEventAward ack;
                ack.set_charid(owerid);
                ack.set_eventid(EventID);
                ack.set_ismail(1);
                for(int i = 0; i != rvecAward.award_size();i++)
                {
                    const HelloKittyMsgData::Award& rcofig = rvecAward.award(i);
                    HelloKittyMsgData::Award* pAward = ack.add_award();
                    if(pAward)
                    {
                        *pAward = rcofig;
                    }
                }
                std::string ret;
                encodeMessage(&ack,ret);
                sendCmdToMe(ret.c_str(),ret.size());
            }

        }


    }
    else
    {
        SceneMailManager::getMe().sendSysMailToPlayerForEvent(charid,EventID,owerid,rvecAward);

    }

}

SceneTask* SceneUser::getGate() const
{
    if(m_gateid == 0)
        return NULL;
    return SceneTaskManager::getMe().getTaskByID(m_gateid);
}

bool SceneUser::sendOtherUserMsg(const void *pstrCmd, const DWORD nCmdLen)
{
    for(auto iter = mapVisit.begin();iter != mapVisit.end();++iter)
    {
        SceneTaskManager::getMe().broadcastUserCmdToGateway(iter->second,iter->first,pstrCmd,nCmdLen);
    }
    return true;
}
//消息通知游客和自己,地图消息：比如说垃圾产生和清除,  
void SceneUser::broadcastMsgInMap(const void *pstrCmd, const int nCmdLen,bool bexpVisit )
{
    if(is_online())
    {
        if(!bexpVisit || VisitID == 0)
        {
            sendCmdToMe(pstrCmd, nCmdLen);
        }
    }
    sendOtherUserMsg(pstrCmd,nCmdLen);
}

void SceneUser::returnMsgInMap(QWORD PlayerID,const void *pstrCmd, const int nCmdLen)
{
    if(PlayerID == charid && is_online()) 
    {
        sendCmdToMe(pstrCmd, nCmdLen);
        return ;

    }

    const map<QWORD,DWORD>& mapVist =  getVist();
    auto it = mapVist.find(PlayerID);
    if(it != mapVist.end())
    {
        SceneTaskManager::getMe().broadcastUserCmdToGateway(it->second,it->first,pstrCmd,nCmdLen);
    }
}

bool SceneUser::isOtherOnline(QWORD PlayerID)
{
    SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerID);
    if(user)
    {
        return (user->is_online());
    }
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(handle)
    {
        return  handle->checkSet("playerset",0 ,"online" , PlayerID);
    }
    return false;

}

void SceneUser::SendMsgToOher(QWORD PlayerID,const void *pstrCmd, const DWORD nCmdLen)
{
    SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerID);
    if(user)
    {
        user->sendCmdToMe(pstrCmd,nCmdLen);
        return ;
    }

    SceneTaskManager::getMe().broadcastUserCmdToGateway(PlayerID,pstrCmd,nCmdLen); 

}
void  SceneUser::BroadCastPersonNum()
{
    if(!havePersonOnline())
        return ;
    DWORD count = static_cast<DWORD>(mapVisit.size());
    if(_online && VisitID == 0)
    {
        count++;
    }
    HelloKittyMsgData::AckSynPerson ack;
    ack.set_charid(charid);
    ack.set_num(count);
    std::string ret;
    encodeMessage(&ack,ret);
    return broadcastMsgInMap(ret.c_str(),ret.size(),true);
}

QWORD SceneUser::getvisit()
{
    return VisitID;
}

void SceneUser::setVisit(QWORD PlayerId)
{
    bool bNeedDelLock =true;
    do{
        if(VisitID == PlayerId)
            break ;
        //原来在自己家
        //进自己家
        //0==0
        //到别人家
        if(VisitID == 0 && PlayerId != 0)
        {

            SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerId);
            if(user)
            {
                user->AddVisit(charid,m_gateid);

            }
            else
            {
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(PlayerId);
                if(!handle)
                    break ;
                DWORD SenceId = handle->getInt("playerscene",PlayerId,"sceneid");
                if(SenceId != 0)
                {
                    Fir::logger->info("寻找其他server处理 ，id %d",SenceId);
                    CMD::SCENE::t_SetVisit_scenescene sendCmd;
                    sendCmd.charid = charid;
                    sendCmd.ownerid = PlayerId;
                    sendCmd.chargateid = m_gateid;
                    std::string ret;
                    encodeMessage(&sendCmd,sizeof(sendCmd),ret);
                    if(!SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
                    {
                        Fir::logger->error(" 找不到对应server，id %d",SenceId);

                    }
                    else
                    {
                        bNeedDelLock = false;
                    }


                }

            }
            addVertiseOther(1);
        }
        //原来在别人家 
        //回自己家
        if(VisitID != 0  && PlayerId == 0)
        {
            SceneUser* user = SceneUserManager::getMe().getUserByID(VisitID);
            if(user)
            {
                user->DelVisit(charid);
            }
            else
            {
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(VisitID);
                if(!handle)
                    break ;
                DWORD SenceId = handle->getInt("playerscene",VisitID,"sceneid");
                if(SenceId != 0)
                {
                    Fir::logger->info("寻找其他server处理 ，id %d",SenceId);
                    CMD::SCENE::t_SetVisit_scenescene sendCmd;
                    sendCmd.charid = charid;
                    sendCmd.ownerid = VisitID;
                    std::string ret;
                    encodeMessage(&sendCmd,sizeof(sendCmd),ret);
                    if(!SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
                    {
                        Fir::logger->error(" 找不到对应server，id %d",SenceId);

                    }

                }
            }
            notifyVistor();
        }
        //到另外别人家
        if(VisitID != 0 && PlayerId != 0)
        {
            SceneUser* user1 = SceneUserManager::getMe().getUserByID(VisitID);
            if(user1)
            {
                user1->DelVisit(charid);
            }
            else
            {
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(VisitID);
                if(!handle)
                    break ;

                DWORD SenceId = handle->getInt("playerscene",VisitID,"sceneid");
                if(SenceId != 0)
                {
                    Fir::logger->info("寻找其他server处理 ，id %d",SenceId);
                    CMD::SCENE::t_SetVisit_scenescene sendCmd;
                    sendCmd.charid = charid;
                    sendCmd.ownerid = VisitID;
                    std::string ret;
                    encodeMessage(&sendCmd,sizeof(sendCmd),ret);
                    if(!SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
                    {
                        Fir::logger->error(" 找不到对应server，id %d",SenceId);

                    }

                }

            }
            SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerId);
            if(user)
            {
                user->AddVisit(charid,m_gateid);

            }
            else
            {
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(PlayerId);
                if(!handle)
                    break ;

                DWORD SenceId = handle->getInt("playerscene",PlayerId,"sceneid");
                if(SenceId != 0)
                {
                    Fir::logger->info("寻找其他server处理 ，id %d",SenceId);
                    CMD::SCENE::t_SetVisit_scenescene sendCmd;
                    sendCmd.charid = charid;
                    sendCmd.ownerid = PlayerId;
                    sendCmd.chargateid = m_gateid;
                    std::string ret;
                    encodeMessage(&sendCmd,sizeof(sendCmd),ret);
                    if(!SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
                    {
                        Fir::logger->error(" 找不到对应server，id %d",SenceId);

                    }
                    else
                    {
                        bNeedDelLock = false;
                    }

                }

            }
            addVertiseOther(1);
        }
    }while(0);
    if(PlayerId > 0 && bNeedDelLock)
    { 
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(PlayerId);
        if(handle)
            handle->delLock("playerlock",PlayerId,"newplayer");

    }

    VisitID = PlayerId;

}


bool SceneUser::checkMaterialMap(const std::map<DWORD,DWORD> &materialMap,const bool notify)
{
    for(auto iter = materialMap.begin();iter != materialMap.end();++iter)
    {
        if(iter->first < HelloKittyMsgData::Attr_Max)
        {
            if(!m_store_house.checkAttr(HelloKittyMsgData::AttrType(iter->first),iter->second))
            {
                if(notify)
                {
                    opErrorReturn(HelloKittyMsgData::Item_Not_Enough,iter->first);
                }
                return false;
            }
        }
        else
        {
            if(!m_store_house.hasEnoughItem(iter->first,iter->second))
            {
                if(notify)
                {
                    opErrorReturn(HelloKittyMsgData::Item_Not_Enough,iter->first);
                }
                return false;
            }
        }
    }
    return true;
}

bool SceneUser::reduceMaterialMap(const std::map<DWORD,DWORD> &materialMap,const char *reMark)
{
    for(auto iter = materialMap.begin();iter != materialMap.end();++iter)
    {
        if(!m_store_house.addOrConsumeItem(iter->first,iter->second,reMark,false))
        {   
            return false;
        }
    }
    return true;
}

bool SceneUser::isUnLock(const DWORD buildType)
{
    QWORD key = hashKey(buildType,1);
    const pb::Conf_t_building *buildConf = tbx::building().get_base(key);
    if(!buildConf)
    {
        return false;
    }
    if(buildConf->getUnlockMap().empty())
    {
        return true;
    }
    if(buildConf->buildInfo->buildkind() == Build_Type_Road)
    {
        return m_unLockBuildSet.find(10010041) != m_unLockBuildSet.end() || m_unLockBuildSet.find(10010065) != m_unLockBuildSet.end() || m_unLockBuildSet.find(10010066) != m_unLockBuildSet.end() || m_unLockBuildSet.find(10010067) != m_unLockBuildSet.end();
    }
    return m_unLockBuildSet.find(buildType) != m_unLockBuildSet.end();
}

bool SceneUser::unLock(const DWORD buildType)
{
    QWORD key = hashKey(buildType,1);
    const pb::Conf_t_building *buildConf = tbx::building().get_base(key);
    if(!buildConf)
    {
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"解锁(%u)",buildType);
    if(!checkMaterialMap(buildConf->getUnlockMap(),true) || !reduceMaterialMap(buildConf->getUnlockMap(),temp))
    {
        return false;
    }
    m_unLockBuildSet.insert(buildType);
    return updateUnLock(buildType);
}

bool SceneUser::updateUnLock(const DWORD typeID)
{
    HelloKittyMsgData::AckUpdateUnLockBuild update;
    if(m_unLockBuildSet.find(typeID) != m_unLockBuildSet.end())
    {
        update.add_id(typeID);
    }
    else
    {
        for(auto iter = m_unLockBuildSet.begin();iter != m_unLockBuildSet.end();++iter)
        {
            update.add_id(*iter);
        }
    }

    std::string ret;
    encodeMessage(&update,ret);
    return sendCmdToMe(ret.c_str(),ret.size());
}


bool SceneUser::addItempOrEmail(const std::map<DWORD,DWORD> &itemMap,const char *mark)
{
    bool ret = true;
    if(m_store_house.hasEnoughSpace(itemMap))
    {
        m_store_house.addOrConsumeItem(itemMap,mark,true);
    }
    else
    {
        const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_WareFull_ID);
        if(emailConf)
        {
            std::vector<HelloKittyMsgData::ReplaceWord> argVec;
            EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,itemMap);
        }
        else
        {
            ret = false;
            Fir::logger->debug("[发送道具邮件失败] (%s,%lu,%s)",charbase.nickname,charid,mark);
        }
    }
    return ret;
}


bool SceneUser::getplayershowbase(QWORD playerId,HelloKittyMsgData::playerShowbase &base)
{
    base.set_playerid(playerId);
    base.set_playername("");
    base.set_playerlevel(0);
    base.set_area(0);
    base.set_city("");
    base.set_contribute(0);
    base.set_charisma(0);
    base.set_popularnow(0);
    base.set_popularmax(0);
    HelloKittyMsgData::SexType sextype =  HelloKittyMsgData::Female ;
    base.set_playersex(sextype);
    HelloKittyMsgData::playerhead *phead = base.mutable_head();
    if(playerId == 0)
        return false;


    if(ISSTATICNPC(playerId))
    {
        HelloKittyMsgData::EnterGardenInfo info;  
        if(!SceneUser::getStaticNpc(playerId,info))
        {
            return false;
        }
        base = info.playershow();
        return true;
    }
    else 
    {

        SceneUser* user = SceneUserManager::getMe().getUserByID(playerId);   
        if(user)
        {
            base.set_playername(user->charbase.nickname);
            base.set_playerlevel(user->charbase.level);
            HelloKittyMsgData::SexType sextype = user->charbase.sex > 0 ? HelloKittyMsgData::Female : HelloKittyMsgData::Male;
            base.set_playersex(sextype);
            base.set_area(user->charbase.areaType);
            *phead = user->charbin.head();

            base.set_city(user->m_personInfo.city());
            base.set_contribute(user->charbin.contribute());
            base.set_charisma(user->charbin.charis());
            base.set_popularnow(user->charbin.popularnow());
            base.set_popularmax(user->charbin.popularmax());
        }
        else
        {
            CharBase charbase;
            if(!RedisMgr::getMe().get_charbase(playerId,charbase))
            {
                return false;

            }
            HelloKittyMsgData::Serialize binary;
            if(!RedisMgr::getMe().get_binary(playerId, binary))
            {
                return false;
            }
            base.set_playername(charbase.nickname);
            base.set_playerlevel(charbase.level);
            base.set_area(charbase.areaType);
            HelloKittyMsgData::SexType sextype = charbase.sex > 0 ? HelloKittyMsgData::Female : HelloKittyMsgData::Male;
            base.set_playersex(sextype);
            *phead = binary.charbin().head();
            base.set_city(binary.personalinfo().city());
            base.set_contribute(binary.charbin().contribute());
            base.set_charisma(binary.charbin().charis());
            base.set_popularnow(binary.charbin().popularnow());
            base.set_popularmax(binary.charbin().popularmax());
        }
        return true;

    }
    return false;

}
bool SceneUser::addItempOrEmail(const DWORD itemID,const DWORD num,const char *mark)
{
    std::map<DWORD,DWORD> tempMap;
    tempMap.insert(std::pair<DWORD,DWORD>(itemID,num));
    return addItempOrEmail(tempMap,mark);
}

bool SceneUser::notifyGarden(const QWORD charID)
{
    HelloKittyMsgData::AckNotifyGarden ack;
    zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(charID);
    if(!handleTemp)
    {
        return false;
    }
    HelloKittyMsgData::PersonInfo *temp = ack.mutable_person();
    if(temp)
    {
        temp->set_charid(charID);
        std::string nickName = std::string(handleTemp->get("rolebaseinfo",charID,"nickname"));
        temp->set_nickname(nickName);
        temp->set_flag(false);
        temp->set_whohome(charid);
    }
    std::string ret;
    encodeMessage(&ack,ret);
    return sendCmdToMe(ret.c_str(),ret.size());
}

bool SceneUser::notifyVistor()
{
    HelloKittyMsgData::AckNotifyGarden ack;
    HelloKittyMsgData::PersonInfo *temp = ack.mutable_person();
    if(!temp)
    {
        return false;
    }
    temp->set_charid(charid);
    temp->set_nickname(charbase.nickname);
    temp->set_flag(true);
    temp->set_whohome(charid);
    std::string ret;
    encodeMessage(&ack,ret);

    for(auto iter = mapVisit.begin();iter != mapVisit.end();++iter)
    {
        QWORD charID = iter->first;
        SceneUser *vistor = SceneUserManager::getMe().getUserByID(charID);
        if(vistor)
        {
            vistor->sendCmdToMe(ret.c_str(),ret.size());
        }
        else
        {
            SceneTaskManager::getMe().broadcastUserCmdToGateway(iter->second,iter->first,ret.c_str(),ret.size());
        }
    }
    HelloKittyMsgData::AckAllGuest ackall;
    ackall.set_charid(charid);
    for(auto it =mapVisitTimer.rbegin(); it != mapVisitTimer.rend();it++) 
    {
        HelloKittyMsgData::guestinfo *info = ackall.add_guardguest();
        if(info != NULL)
        {
            info->set_visittimer(it->first);
            HelloKittyMsgData::playerShowbase *pShow = info->mutable_guest();
            if(pShow)
            {
                SceneUser::getplayershowbase(it->second,*pShow);
            }

        }
    }
    std::string retall;
    encodeMessage(&ackall,retall); 
    sendCmdToMe(retall.c_str(),retall.size());


    return true;
}

DWORD SceneUser::countParseCD(const DWORD lastSec)
{
    return ceil(1.0 * lastSec / (30*60));
}

bool SceneUser::inGuide()
{
    return charbin.taskguidid() || pb::Conf_t_NewGuide::getNextGuide(charbase.guideid,false);
}

bool SceneUser::stopCd(const SYSTEMID &systemID)
{
    const pb::Conf_t_NewGuide *newGuide = tbx::NewGuide().get_base(charbase.guideid);
    if(newGuide)
    {
        const std::set<DWORD> &stopCDSet = newGuide->getCDSet();
        if(stopCDSet.find(systemID) != stopCDSet.end())
        {
            return true;
        }
    }
    const pb::Conf_t_Guide *guide = tbx::Guide().get_base(hashKey(charbin.taskguidid(),charbin.taskguidstep()));
    if(guide)
    {
        const std::set<DWORD> &stopCDSet = guide->getCDSet();
        if(stopCDSet.find(systemID) != stopCDSet.end())
        {
            return true;
        }
    }
    return false;
}

DWORD SceneUser::getContribute(const QWORD charID)
{
    return m_contrubuteMap.find(charID) == m_contrubuteMap.end() ? 0 : m_contrubuteMap[charID];
}

void SceneUser::opCharisma(const QWORD charID,const DWORD val,const char *reMark,const bool addOp)
{
    DWORD oldVal = m_acceptCharismaMap.find(charID) != m_acceptCharismaMap.end() ? m_acceptCharismaMap[charID] : 0;
    bool change = true;
    if(addOp)
    {
        if(m_acceptCharismaMap.find(charID) == m_acceptCharismaMap.end())
        {
            m_acceptCharismaMap.insert(std::pair<QWORD,DWORD>(charID,val));
        }
        else
        {
            m_acceptCharismaMap[charID] += val;
        }
    }
    else
    {
        if(m_acceptCharismaMap.find(charID) != m_acceptCharismaMap.end())
        {
            m_acceptCharismaMap[charID] = m_acceptCharismaMap[charID] > val ? m_acceptCharismaMap[charID] - val : 0;
        }
        else
        {
            change = false;
        }
    }
    if(change)
    {
        DWORD newVal = m_acceptCharismaMap.find(charID) != m_acceptCharismaMap.end() ? m_acceptCharismaMap[charID] : 0;
        Fir::logger->info("[操作魅力值] (%lu,%s,%u,%u,%s)",charid,addOp ? "增加" : "减少",oldVal,newVal,reMark);
        //updateContibute(charID);
    }
}


void SceneUser::opContrubute(const QWORD charID,const DWORD val,const char *reMark,const bool addOp)
{
    DWORD oldVal = m_contrubuteMap.find(charID) != m_contrubuteMap.end() ? m_contrubuteMap[charID] : 0;
    bool change = true;
    if(addOp)
    {
        if(m_contrubuteMap.find(charID) == m_contrubuteMap.end())
        {
            m_contrubuteMap.insert(std::pair<QWORD,DWORD>(charID,val));
        }
        else
        {
            m_contrubuteMap[charID] += val;
        }
    }
    else
    {
        if(m_contrubuteMap.find(charID) != m_contrubuteMap.end())
        {
            m_contrubuteMap[charID] = m_contrubuteMap[charID] > val ? m_contrubuteMap[charID] - val : 0;
        }
        else
        {
            change = false;
        }
    }
    if(change)
    {
        DWORD newVal = m_contrubuteMap.find(charID) != m_contrubuteMap.end() ? m_contrubuteMap[charID] : 0;
        Fir::logger->info("[操作贡献值] (%lu,%s,%u,%u,%s)",charid,addOp ? "增加" : "减少",oldVal,newVal,reMark);
        updateContibute(charID);
    }
}

void SceneUser::updateContibute(const QWORD charID)
{
    HelloKittyMsgData::AckUpdateContribute updateContribute;
    if(!charID)
    {
        for(auto iter = m_contrubuteMap.begin();iter != m_contrubuteMap.end();++iter)
        {
            HelloKittyMsgData::Key64Val32Pair *pair = updateContribute.add_contribute();
            if(pair)
            {
                pair->set_key(iter->first);
                pair->set_val(iter->second);
            }
        }
    }
    else
    {
        if(m_contrubuteMap.find(charID) != m_contrubuteMap.end())
        {
            HelloKittyMsgData::Key64Val32Pair *pair = updateContribute.add_contribute();
            if(pair)
            {
                pair->set_key(charID);
                pair->set_val(m_contrubuteMap[charID]);
            }
        }
    }

    std::string ret;
    encodeMessage(&updateContribute,ret);
    sendCmdToMe(ret.c_str(),ret.size());
}

DWORD SceneUser::getContinueGift(const QWORD accepter)
{
    DWORD now = SceneTimeTick::currentTime.sec();
    auto it = m_giftContinue.find(accepter);
    if(it == m_giftContinue.end() || now > it->second.first + ParamManager::getMe().GetSingleParam(eParam_SendGift_CD))
    {
        //1
        m_giftContinue.erase(accepter);
        return 0;
    }
    return it->second.second;

}

DWORD SceneUser::addGiftCD(const QWORD accepter)
{
    DWORD now = SceneTimeTick::currentTime.sec();
    auto it = m_giftContinue.find(accepter);
    if(it == m_giftContinue.end() || now > it->second.first + ParamManager::getMe().GetSingleParam(eParam_SendGift_CD))
    {
        //1
        m_giftContinue[accepter] =  std::make_pair(now,1);
        return 1;
    }
    else
    {
        //+1
        m_giftContinue[accepter] = std::make_pair(now,it->second.second + 1);

    }
    return m_giftContinue[accepter].second;
}

DWORD SceneUser::rechargeRMB(const DWORD rmb,const DWORD activeID)
{
    HelloKittyMsgData::DailyData *daily = charbin.mutable_dailydata();
    if(!charbin.firstrecharge())
    {
        charbin.set_firstrecharge(rmb);
        m_active.doaction(HelloKittyMsgData::ActiveConditionType_First_Charge,rmb);
    }
    if(!daily->rechargetodayfrist())
    {
        daily->set_rechargetodayfrist(daily->rechargetodayfrist() + rmb);
        m_active.doaction(HelloKittyMsgData::ActiveConditionType_Daily_Charge,rmb);
    }
    daily->set_rechargetoday(daily->rechargetoday() + rmb);
    m_active.doaction(HelloKittyMsgData::ActiveConditionType_Daily_Charge_Sum,rmb);
    charbin.set_rechargetotal(charbin.rechargetotal() + rmb);
    m_active.doaction(HelloKittyMsgData::ActiveConditionType_All_Charge_Sum,rmb);
    if(activeID)
    {
        auto iter = m_activeRechargeMap.find(activeID);
        if(iter != m_activeRechargeMap.end())
        {
            m_activeRechargeMap.insert(std::pair<DWORD,DWORD>(activeID,rmb));
            m_active.doaction(HelloKittyMsgData::ActiveConditionType_Active_Charge,rmb);
        }
    }
    return rmb;
}

DWORD SceneUser::getActiveRecharge(const DWORD activeID)
{
    auto iter = m_activeRechargeMap.find(activeID);
    return iter != m_activeRechargeMap.end() ? iter->second : 0;
}

bool SceneUser::rewardActiveCode(const std::string &codeID)
{
    HelloKittyMsgData::ErrorCodeType code = HelloKittyMsgData::Error_Common_Occupy;
    bool ret = false;
    do
    {
        FieldSet* recordFile = SceneService::metaData->getFields(Fir::global["activecode"].c_str());
        connHandleID handle = SceneService::dbConnPool->getHandle();
        if(!recordFile || handle == (DWORD)-1)
        {
            break;
        }

        Record where;
        std::ostringstream temp;
        temp << "f_code=" << "'" << codeID << "'";
        where.put("f_code",temp.str());
        RecordSet *recordset = SceneService::dbConnPool->exeSelect(handle,recordFile,NULL,&where);
        if(!recordset)
        {
            code = HelloKittyMsgData::ActiveCode_Not_Exist;
            break;
        }
        SceneService::dbConnPool->putHandle(handle);
        for(DWORD index = 0;index < recordset->size();++index)
        {
            Record *rec = recordset->get(index);
            char buffer[zSocket::MAX_DATASIZE];
            bzero(buffer,sizeof(buffer));
            DWORD binarySize = rec->get("f_allbinary").getBinSize();
            memcpy(buffer,(const char*)rec->get("f_allbinary"),binarySize);
            HelloKittyMsgData::ActiveCode activeCode;
            activeCode.ParseFromArray(buffer,binarySize);
            if(m_codeTypeMap.find(activeCode.type()) != m_codeTypeMap.end())
            {
                code = HelloKittyMsgData::ActiveCode_Ready_Reward;
                break;
            }

            std::map<DWORD,DWORD> codeMap;
            for(int index = 0;index < activeCode.item_size();++index)
            {
                const HelloKittyMsgData::Key32Val32Pair &pair = activeCode.item(index);
                if(codeMap.find(pair.key()) == codeMap.end())
                {
                    codeMap.insert(std::pair<DWORD,DWORD>(pair.key(),pair.val()));
                }
                else
                {
                    codeMap[pair.key()] += pair.val();
                }
            }
            const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Active_Code);
            if(emailConf)
            {
                std::vector<HelloKittyMsgData::ReplaceWord> argVec;
                EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,codeMap);
            }
            m_codeTypeMap.insert(std::pair<DWORD,std::string>(activeCode.type(),codeID));
            Fir::logger->debug("[兑换激活码成功] (%lu,%s,%u,%s)",charid,charbase.nickname,activeCode.type(),codeID.c_str());
        }
        ret = true;
    }while(false);

    if(code != HelloKittyMsgData::Error_Common_Occupy)
    {
        opErrorReturn(code);
    }
    else if(ret)
    {
        std::string ackStr;
        HelloKittyMsgData::AckRewardActiveCode ack;
        ack.set_ret(ret ? HelloKittyMsgData::ART_True : HelloKittyMsgData::ART_False);
        encodeMessage(&ack,ackStr);
        sendCmdToMe(ackStr.c_str(),ackStr.size());
    }
    return ret;
}

void SceneUser::opItemResourMap(const ITEMRESORETYPE type,const DWORD itemID,const DWORD num,const bool opType)
{
    do
    {
        AttrID attrID = Attr_SystemOrder_Get; 
        switch(type)
        {
            case Item_SysOrder:
                {
                    auto iter = m_sysOrderMap.find(itemID);
                    if(opType)
                    {
                        if(iter != m_sysOrderMap.end())
                        {
                            iter->second += num;
                        }
                        else
                        {
                            m_sysOrderMap.insert(std::pair<DWORD,DWORD>(itemID,num));
                        }
                    }
                    else
                    {
                        if(iter != m_sysOrderMap.end())
                        {
                            iter->second = iter->second > num ? iter->second - num : 0;
                        }
                    }
                }
                break;
            case Item_Produce:
                {
                    auto iter = m_prodeceMap.find(itemID);
                    if(opType)
                    {
                        if(iter != m_prodeceMap.end())
                        {
                            iter->second += num;
                        }
                        else
                        {
                            m_prodeceMap.insert(std::pair<DWORD,DWORD>(itemID,num));
                        }
                    }
                    else
                    {
                        if(iter != m_prodeceMap.end())
                        {
                            iter->second = iter->second > num ? iter->second - num : 0;
                        }
                    }
                    attrID = Attr_Produce_Get;
                }
                break;
            case Item_Composite:
                {
                    auto iter = m_compositeMap.find(itemID);
                    if(opType)
                    {
                        if(iter != m_compositeMap.end())
                        {
                            iter->second += num;
                        }
                        else
                        {
                            m_compositeMap.insert(std::pair<DWORD,DWORD>(itemID,num));
                        }
                    }
                    else
                    {
                        if(iter != m_compositeMap.end())
                        {
                            iter->second = iter->second > num ? iter->second - num : 0;
                        }
                    }
                    attrID = Attr_Composite_Get;
                }
                break;
            default:
                {
                    break;
                }
        }
        if(opType)
        {
             TaskArgue arg(Target_Add_Source,attrID,attrID,num);
             m_taskManager.target(arg);
             if(type == Item_SysOrder)
             {
                m_active.doaction(HelloKittyMsgData::ActiveConditionType_Regular_Order_Number,num,0,itemID,HelloKittyMsgData::AST_TimeLimit_Order);
             }
        }
    }while(false);
}

DWORD SceneUser::getItemResourNum(const ITEMRESORETYPE type,const DWORD itemID)
{
    DWORD num = 0;
    do
    {
        switch(type)
        {
            case Item_SysOrder:
                {
                    auto iter = m_sysOrderMap.find(itemID);
                    num = iter != m_sysOrderMap.end() ? iter->second : num;
                }
                break;
            case Item_Produce:
                {
                    auto iter = m_prodeceMap.find(itemID);
                    num = iter != m_prodeceMap.end() ? iter->second : num;
                }
                break;
            case Item_Composite:
                {
                    auto iter = m_compositeMap.find(itemID);
                    num = iter != m_compositeMap.end() ? iter->second : num;
                }
                break;
            default:
                {
                    break;
                }
        }
    }while(false);
    return num;
}

void SceneUser::loadRecord(const HelloKittyMsgData::Serialize &binary)
{
    for(int index = 0;index < binary.sysorderrecord_size();++index)
    {
        const HelloKittyMsgData::Key32Val32Pair &pair = binary.sysorderrecord(index);
        m_sysOrderMap.insert(std::pair<DWORD,DWORD>(pair.key(),pair.val()));
    }

    for(int index = 0;index < binary.compositerecord_size();++index)
    {
        const HelloKittyMsgData::Key32Val32Pair &pair = binary.compositerecord(index);
        m_compositeMap.insert(std::pair<DWORD,DWORD>(pair.key(),pair.val()));
    }

    for(int index = 0;index < binary.producerecord_size();++index)
    {
        const HelloKittyMsgData::Key32Val32Pair &pair = binary.producerecord(index);
        m_prodeceMap.insert(std::pair<DWORD,DWORD>(pair.key(),pair.val()));
    }
}

void SceneUser::saveRecord(HelloKittyMsgData::Serialize &binary)
{
    for(auto iter = m_sysOrderMap.begin();iter != m_sysOrderMap.end();++iter)
    {
        HelloKittyMsgData::Key32Val32Pair *pair = binary.add_sysorderrecord();
        if(pair)
        {
            pair->set_key(iter->first);
            pair->set_val(iter->second);
        }
    }

    for(auto iter = m_compositeMap.begin();iter != m_compositeMap.end();++iter)
    {
        HelloKittyMsgData::Key32Val32Pair *pair = binary.add_compositerecord();
        if(pair)
        {
            pair->set_key(iter->first);
            pair->set_val(iter->second);
        }
    }

    for(auto iter = m_prodeceMap.begin();iter != m_prodeceMap.end();++iter)
    {
        HelloKittyMsgData::Key32Val32Pair *pair = binary.add_producerecord();
        if(pair)
        {
            pair->set_key(iter->first);
            pair->set_val(iter->second);
        }
    }
}

bool SceneUser::adjusetGrid()
{
    DWORD level = m_store_house.getAttr(HelloKittyMsgData::Attr_Ware_Level);
    DWORD nowGrid = m_store_house.getAttr(HelloKittyMsgData::Attr_Ware_Grid_Val);
    const pb::Conf_t_WareHouseGrid *gridConf = tbx::WareHouseGrid().get_base(level);
    if(!gridConf)
    {
        return false;
    }
    if(gridConf->gridInfo->grid() > nowGrid)
    {
        m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Ware_Grid_Val,gridConf->gridInfo->grid() - nowGrid,"GM",true);
    }
    else
    {
        m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Ware_Grid_Val,nowGrid - gridConf->gridInfo->grid(),"GM",false);
    }
    return true;
}

