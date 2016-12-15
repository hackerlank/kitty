#include "burstEventManager.h"
#include "SceneUser.h"
#include "tbx.h"
#include "TimeTick.h"
#include "Misc.h"
#include "buffer.h"

BurstEventManager::BurstEventManager(SceneUser *owner) : m_owner(owner)
{
}

BurstEventManager::~BurstEventManager()
{
}

void BurstEventManager::reset()
{
    m_eventMap.clear();
}


bool BurstEventManager::save(HelloKittyMsgData::Serialize& binary)
{
#if 0
    for(auto iter = m_eventMap.begin();iter != m_eventMap.end();++iter)
    {
        HelloKittyMsgData::BurstEvent *temp = binary.add_burstevent();
        if(temp)
        {
            *temp = iter->second;
        }
    }
#endif
    return true;
}

bool BurstEventManager::load(const HelloKittyMsgData::Serialize& binary)
{
#if 0
    reset();
    const std::map<QWORD,QWORD>& oldNewKeyMap = m_owner->m_buildManager.getOldNewKeyMap();
    for(int index = 0;index < binary.burstevent_size();++index)
    {
        HelloKittyMsgData::BurstEvent &temp = const_cast<HelloKittyMsgData::BurstEvent&>(binary.burstevent(index));
        auto iter = oldNewKeyMap.find(temp.tempid());
        if(iter != oldNewKeyMap.end())
        {
            temp.set_tempid(iter->second);
            m_eventMap.insert(std::pair<QWORD,HelloKittyMsgData::BurstEvent>(temp.tempid(),temp));
            m_npcKeySet.insert(temp.rewardkey());
            m_colIDMap.insert(std::pair<DWORD,QWORD>(temp.rewardkey(),temp.tempid()));
            m_owner->m_buildManager.opBurstEventMap(temp.tempid(),temp.npckey(),true);
        }
    }
#endif
    return true;
}

bool BurstEventManager::fullMessage(HelloKittyMsgData::UserBaseInfo &binary)
{
   for(auto iter = m_eventMap.begin();iter != m_eventMap.end();++iter)
   {
       HelloKittyMsgData::BurstEvent *temp = binary.add_burstevent();
       if(temp)
       {
           *temp = iter->second;
       }
   }
   return true;
}
 

bool BurstEventManager::flushEvent()
{
    HelloKittyMsgData::AckUpdateBurstEvent message;
    for(auto iter = m_eventMap.begin();iter != m_eventMap.end();++iter)
    {
        HelloKittyMsgData::BurstEvent *temp = message.add_eventlist();
        if(temp)
        {
            *temp = iter->second;
        }
    }

    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool BurstEventManager::updateBurstEvent(const QWORD eventID)
{
    HelloKittyMsgData::BurstEvent* event = getEvent(eventID);
    if(!event)
    {
        return false;
    }
    HelloKittyMsgData::AckUpdateBurstEvent message;
    HelloKittyMsgData::BurstEvent *temp = message.add_eventlist();
    if(temp)
    {
        *temp = *event;
    }
    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool BurstEventManager::newEvent(const DWORD colID)
{
    bool ret = false;
    do
    {
        if(m_colIDMap.find(colID) != m_colIDMap.end())
        {
            break;
        }
        DWORD now = SceneTimeTick::currentTime.sec();
        DWORD npcKey = pb::Conf_t_BurstEventNpc::randExceptNpc(m_owner->charbase.level,m_npcKeySet);
        if(!npcKey)
        {
            break;
        }
        BuildBase* road = m_owner->m_buildManager.getEmptyRoad();
        if(!road)
        {
            break;
        }
        DWORD rewardKey = colID;
        Fir::logger->debug("[刷出突发事件]:(%lu,%s,%lu,%u,%u)",m_owner->charid,m_owner->charbase.nickname,road->getID(),npcKey,rewardKey);
        HelloKittyMsgData::BurstEvent temp;
        temp.set_tempid(road->getID());
        temp.set_npckey(npcKey);
        temp.set_rewardkey(rewardKey);
        temp.set_status(HelloKittyMsgData::BES_Accept);
        temp.set_begintime(now);
        m_eventMap.insert(std::pair<QWORD,HelloKittyMsgData::BurstEvent>(temp.tempid(),temp));
        updateBurstEvent(temp.tempid());
        m_npcKeySet.insert(npcKey);
        m_colIDMap.insert(std::pair<DWORD,QWORD>(rewardKey,temp.tempid()));
        m_owner->m_buildManager.opBurstEventMap(temp.tempid(),npcKey,true);
        ret = true;
    }while(false);
    return ret;
}


bool BurstEventManager::loop(const bool online)
{
    Fir::logger->debug("[刷出突发事件loop]");

    DWORD now = SceneTimeTick::currentTime.sec();
    std::vector<QWORD> delVec;
    std::set<DWORD> npcKeySet;
    std::set<DWORD> rewardKeySet;
    for(auto iter = m_eventMap.begin();iter != m_eventMap.end();++iter)
    {
        HelloKittyMsgData::BurstEvent &temp = const_cast<HelloKittyMsgData::BurstEvent&>(iter->second);
        if(now >= temp.begintime() + 3*60*60)
        {
            delVec.push_back(iter->first);
        }
        else
        {
            npcKeySet.insert(temp.npckey());
            rewardKeySet.insert(temp.rewardkey());
        }
    }
    for(auto iter = delVec.begin();iter != delVec.end();++iter)
    {
        delEvent(*iter,HelloKittyMsgData::BES_Del_Other);
    }

    DWORD eventNum = m_owner->m_buildManager.getRoadNum();
    eventNum /= 10;
    eventNum = eventNum ? eventNum - 1 : 0;
    eventNum = eventNum > 2 ? 2 : eventNum;
    DWORD addNum = 1;
    eventNum = 5;
    if(online)
    {
        DWORD hour = (m_owner->charbase.onlinetime - m_owner->charbase.offlinetime) / 3600; 
        addNum = hour;
    }
    addNum = eventNum;
    for(int cnt = 0;m_eventMap.size() < eventNum && addNum && cnt < 10;++cnt)
    {
        DWORD npcKey = pb::Conf_t_BurstEventNpc::randExceptNpc(m_owner->charbase.level,npcKeySet);
        DWORD rewardKey = pb::Conf_t_order::getOrderIdbyLv(m_owner->charbase.level,rewardKeySet); 
        if(npcKey && rewardKey)
        {
            BuildBase* road = m_owner->m_buildManager.getEmptyRoad();
            if(!road)
            {
                break;
            }
            Fir::logger->debug("[刷出突发事件]:(%lu,%s,%lu,%u,%u)",m_owner->charid,m_owner->charbase.nickname,road->getID(),npcKey,rewardKey);
            HelloKittyMsgData::BurstEvent temp;
            temp.set_tempid(road->getID());
            temp.set_npckey(npcKey);
            temp.set_rewardkey(rewardKey);
            temp.set_status(HelloKittyMsgData::BES_Accept);
            temp.set_begintime(now);
            m_eventMap.insert(std::pair<QWORD,HelloKittyMsgData::BurstEvent>(temp.tempid(),temp));
            updateBurstEvent(temp.tempid());
            npcKeySet.insert(npcKey);
            rewardKeySet.insert(rewardKey);
            m_owner->m_buildManager.opBurstEventMap(temp.tempid(),npcKey,true);
            --addNum;

        }
    }
    return true;
}

bool BurstEventManager::opEvent(const HelloKittyMsgData::ReqOpBurstEvent *cmd)
{
    bool ret = false;
    do
    {
        HelloKittyMsgData::BurstEvent *event = getEvent(cmd->tempid());
        if(!event || event->status() != HelloKittyMsgData::BES_Accept)
        {
            break;
        }
        if(cmd->optype() == HelloKittyMsgData::BEOT_Submit)
        {
            ret = checkTarget(cmd->tempid());
        }
        else
        {
            ret = delEvent(cmd->tempid(),HelloKittyMsgData::BES_Del_Other);
        }
        if(ret)
        {
            loop();
        }
    }while(false);
    return ret;
}

bool BurstEventManager::checkTarget(const QWORD tempid)
{
    HelloKittyMsgData::BurstEvent* temp = getEvent(tempid);
    if(!temp)
    {
        return false;
    }
    const pb::Conf_t_order *rewardConf = tbx::order().get_base(temp->rewardkey());
    if(!rewardConf || !m_owner->m_store_house.hasEnoughSpace(rewardConf->getRewardMap()))
    {
        m_owner->opErrorReturn(HelloKittyMsgData::WareHouse_Is_Full);
        return false;
    }
    const std::map<DWORD,DWORD>& randItemMap = rewardConf->getRandItemMap();
    char reMark[100] = {0};
    snprintf(reMark,sizeof(reMark),"突发事件扣除(%u,%u)",temp->npckey(),temp->rewardkey());
    if((!randItemMap.empty()) && (!m_owner->checkMaterialMap(randItemMap,true) || !m_owner->m_store_house.addOrConsumeItem(randItemMap,reMark,false)))
    {
        return false;
    }
    bzero(reMark,sizeof(reMark));
    snprintf(reMark,sizeof(reMark),"突发事件奖励(%u,%u)",temp->npckey(),temp->rewardkey());

    std::map<DWORD,DWORD> rewardMap;
    std::vector<DWORD> bufferVec;
    getBufferList(m_owner,bufferVec,BufferID_Add_Burst_Reward);
    for(auto iter = bufferVec.begin();iter != bufferVec.end();++iter)
    {
        const pb::Conf_t_buffer *bufferConf = tbx::buffer().get_base(BufferID_Add_Burst_Reward);
        if(!bufferConf)
        {
            continue;
        }
        for(auto it = rewardConf->getRewardMap().begin();it != rewardConf->getRewardMap().end();++it)
        {
            DWORD oldVal = rewardMap.find(it->first) == rewardMap.end() ? it->second : rewardMap[it->first];
            DWORD val = bufferConf->buffer->count() + bufferConf->buffer->ratio() * 1.0 * (oldVal) / 100;
            if(!bufferConf->buffer->optype())
            {
                rewardMap[it->first] = val + oldVal;
            }
        }
    }

    m_owner->m_store_house.addOrConsumeItem(rewardMap.empty() ? rewardConf->getRewardMap() : rewardMap,reMark,true);
    HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
    if(dailyData)
    {
        dailyData->set_finishburstevent(dailyData->finishburstevent()+1);
    }
    TaskArgue arg(Target_InterActive,Attr_Add_Finish_Burst_Event,Attr_Add_Finish_Burst_Event,dailyData->finishburstevent());
    m_owner->m_taskManager.target(arg);
    delEvent(tempid,HelloKittyMsgData::BES_Del_Finish);
    MiscManager::getMe().getAdditionalRewards(m_owner,AdditionalType_BrustEvent);
    return true;
}

HelloKittyMsgData::BurstEvent* BurstEventManager::getEvent(const QWORD tempid)
{
    auto iter = m_eventMap.find(tempid);
    if(iter == m_eventMap.end())
    {
        return NULL;
    }
    return const_cast<HelloKittyMsgData::BurstEvent*>(&(iter->second));
}

bool BurstEventManager::delEvent(const QWORD tempid,const HelloKittyMsgData::BurstEventStatus &delType)
{
    HelloKittyMsgData::BurstEvent* temp = getEvent(tempid);
    if(!temp)
    {
        return false;
    }
    Fir::logger->debug("[删除突发事件]:(%lu,%s,%lu,%u,%u)",m_owner->charid,m_owner->charbase.nickname,tempid,temp->npckey(),temp->rewardkey());
    m_owner->m_buildManager.opBurstEventMap(temp->tempid(),temp->npckey(),false);
    temp->set_status(delType);
    updateBurstEvent(temp->tempid());
    m_colIDMap.erase(temp->rewardkey());
    m_npcKeySet.erase(temp->rewardkey());
    m_eventMap.erase(tempid);
    return true;
}

bool BurstEventManager::delEvent(const DWORD colID)
{
    auto iter = m_colIDMap.find(colID);
    return iter != m_colIDMap.end() ? delEvent(iter->second,HelloKittyMsgData::BES_Del_Finish) : false;
}


