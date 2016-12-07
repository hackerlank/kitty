#include "EventManger.h"
#include "zMisc.h"
#include "TimeTick.h"
#include "buildManager.h"
#include "SceneUser.h"
#include "Misc.h"
bool EventBase::OpRoad(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack)
{
    ack.set_eventid(m_Id);
    HelloKittyMsgData::Builditype *pbuild =  ack.mutable_build();
    setBuildId(pbuild);
    ack.set_result(HelloKittyMsgData::PlayerOpEventResult_Suc);
    return false;
}

bool EventBase::UseIncon()
{
    return false;
}


bool EventBase::bindforother()
{
    return true;
}

bool EventBase::needpushevent()
{
    return true;
}
bool EventBase::canOp(QWORD PlayerId)
{
    return true;
}

DWORD EventBase::getOpReserverTimer(QWORD PlayerId)
{
    return 0;
}

bool EventBase::init(DWORD bindBuild)

{
    if(m_InsId == 0)
        return false;
    bool needPushClient = (bindBuild == 0);
    if(needPushClient)
    {
        NoticeBindBuild(true);
    }
    if(needpushevent())
        NoticeNewEvent(true,needPushClient);
    if(!UseIncon())
    {
        BuildBase *temp = m_rManager.getUser().m_buildManager.getBuild(m_InsId);
        if(temp)
            temp->setEvent(true);
    }


    return true;
}

void EventBase::OnlineSend(QWORD PlayerId,HelloKittyMsgData::Evententer*info)
{
    NoticeBindBuild(info,PlayerId); 
    if(needpushevent())
        NoticeNewEvent(info);

}

void EventBase::destroy()
{
    //清除建筑物上标记
    if(needpushevent())
        NoticeNewEvent(false,true);
    NoticeBindBuild(false);
    if(!UseIncon())
    {
        BuildBase *temp = m_rManager.getUser().m_buildManager.getBuild(m_InsId);
        if(temp)
            temp->setEvent(false);
        if(temp && temp->getBreak())
        {
            temp->setBreak(false);
        }
    }

}

bool EventBase::init(const HelloKittyMsgData::SaveForEvent& rEvent)
{
    m_EndTimer = rEvent.endtimer();
    m_step = rEvent.value();
    if(rEvent.opplayer_size() == rEvent.optimer_size())
    {
        for(int i =0 ; i != rEvent.opplayer_size();i++)
        {
            m_otherOpTimer[rEvent.opplayer(i)] = rEvent.optimer(i);

        }

    }
    return init(rEvent.buildid());

}


EventBase::~EventBase()
{
    //DONothing 
}

bool EventBase::isOverTimer(DWORD nowTimer)
{
    if(m_EndTimer == 0)
    {
        return false;
    }
    return nowTimer >= m_EndTimer;
}

EventBase::EventBase(EventManager &rManager,DWORD Id ,HelloKittyMsgData::PlayerEventType type,DWORD intraltimer):m_rManager(rManager),m_Id(Id),m_type(type),m_InsId(0),m_step(0)    
{
    m_EndTimer  = intraltimer == 0 ? 0 :  SceneTimeTick::currentTime.sec() + intraltimer;
}

DWORD EventBase::getId()
{
    return m_Id;

}
DWORD EventBase::getTotalReserverTimer()
{
    DWORD nowTimer = SceneTimeTick::currentTime.sec();
    return m_EndTimer < nowTimer ? 0 : m_EndTimer - nowTimer;
}
bool EventBase::needdestroy()
{
    return true;
}

HelloKittyMsgData::PlayerEventType EventBase::getType()
{
    return m_type;
}


DWORD EventBase::getEndTimer()
{
    return m_EndTimer;
}

void EventBase::setPlayerOp(QWORD charid,HelloKittyMsgData::EventBuildNotice &ack)
{
    ack.set_canop(canOp(charid) ? 0 : 1);
    ack.set_reservetime(getOpReserverTimer(charid));

}

void EventBase::setBuildId(HelloKittyMsgData::Builditype *pbuild)
{
    if(pbuild)
    {
        if(UseIncon())
        {
            pbuild->set_isicon(1);
        }
        else
        {
            pbuild->set_isicon(0);
        }
        pbuild->set_buildid(m_InsId);
    }

}


void EventBase::NoticeBindBuild(bool add)//绑定建筑通知
{
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(!eventConf)
    {
        return ;
    }
    HelloKittyMsgData::EventBuildNotice ack;
    ack.set_charid(m_rManager.getUser().charid);
    HelloKittyMsgData::Builditype *pbuild =  ack.mutable_build();
    setBuildId(pbuild);
    if(add)
    {
        ack.set_eventid(m_Id);
        ack.set_data(m_step);
        ack.set_totalreservertimer(getTotalReserverTimer());
        LOG_ERR("Send add Build Messge Bulid :%lu event %d",m_InsId,m_Id);
    }
    else
    {
        ack.set_eventid(0);
        LOG_ERR("Send remove Build Messge Bulid :%lu event %d",m_InsId,m_Id);
    }
    if(bindforother())
    {
        const map<QWORD,DWORD>& mapVist =  m_rManager.getUser().getVist();
        for(auto iter = mapVist.begin(); iter != mapVist.end(); iter++)
        {
            if(add)
            {
                setPlayerOp(iter->first,ack);
            }
            std::string ret;
            encodeMessage(&ack,ret);
            m_rManager.getUser().returnMsgInMap(iter->first,ret.c_str(),ret.size());
            LOG_ERR("visit :%lu owner :%lu ack.canOp() %d data %d",iter->first,m_rManager.getUser().charid,ack.canop(),ack.data());


        }
    }
    if(add)
    {
        setPlayerOp(m_rManager.getUser().charid,ack);
    }
    std::string ret;
    encodeMessage(&ack,ret);
    m_rManager.getUser().returnMsgInMap(m_rManager.getUser().charid,ret.c_str(),ret.size());
    LOG_ERR("owner :%lu ack.canOp() %d data %d",m_rManager.getUser().charid,ack.canop(),ack.data());


}

void EventBase::NoticeBindBuild(HelloKittyMsgData::Evententer *info,QWORD PlayerId)//绑定建筑通知
{
    if(!bindforother() && PlayerId != m_rManager.getUser().charid)
        return ;
    LOG_ERR("Send load Build Messge Bulid :%lu event %d",m_InsId,m_Id);
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(!eventConf)
    {
        return ;
    }
    HelloKittyMsgData::EventBuildNotice * pAck = info->add_eventbuild();
    if(!pAck)
        return ;
    HelloKittyMsgData::EventBuildNotice &ack = *pAck;
    ack.set_charid(m_rManager.getUser().charid);
    HelloKittyMsgData::Builditype *pbuild =  ack.mutable_build();
    setBuildId(pbuild);
    ack.set_eventid(m_Id);
    ack.set_data(m_step);
    ack.set_totalreservertimer(getTotalReserverTimer());
    setPlayerOp(PlayerId,ack);


}
void EventBase::NoticeNewEvent(HelloKittyMsgData::Evententer *info)
{

    LOG_ERR("Send load  event %d",m_Id);
    HelloKittyMsgData::EventNotice *pAck = info->mutable_eventinfo(); 
    if(!pAck)
        return;
    HelloKittyMsgData::EventNotice& ack = *pAck;
    ack.set_charid(m_rManager.getUser().charid);
    ack.set_eventid(m_Id);
}
void EventBase::NoticeNewEvent(bool add,bool bClient)//新事件通知
{
    //ToClient
    if(bClient)
    {

        HelloKittyMsgData::EventNotice ack;
        ack.set_charid(m_rManager.getUser().charid);
        if(add)
        {
            ack.set_eventid(m_Id);
            LOG_ERR("Send add  event %d",m_Id);
        }
        else
        {
            ack.set_eventid(0);
            LOG_ERR("Send del  event %d",m_Id);
        }
        std::string ret; 
        encodeMessage(&ack,ret);
        m_rManager.getUser().broadcastMsgInMap(ret.c_str(),ret.size());

    }
    HelloKittyMsgData::PlayerEvent event;
    if(add)
    {
        event.set_eventid(m_Id);
    }
    else
    {
        event.set_eventid(0);

    }
    event.set_endtimer(m_EndTimer);
    m_rManager.getUser().sendhappenevent(event);

}
bool EventBase::canDel()
{
    return false;
}




/***************************************EventLoseBoy*******************************************/
bool EventLoseBoy::OpRoad(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack)
{
    if(EventBase::OpRoad(PlayerId,ack))
    {
        return true;
    }
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(!eventConf)
    {
        ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
        return true;
    }
    if(PlayerId == m_rManager.getUser().charid)
    {
        ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
        return true;
    }

    bool bfinish = false;
    BYTE rate = zMisc::randBetween(0,100);
    if(m_step < eventConf->target.size())
    {
        if(rate <= eventConf->target[m_step])
        {
            bfinish = true;
        }

    }
    else
    {
        bfinish = true;
    }
    if(!bfinish)
    {
        m_step++;
        NoticeBindBuild(false);
        m_InsId = m_rManager.getUser().m_buildManager.getAnyBuildBytype(eventConf->buildevent,false);
        NoticeBindBuild(true);

    }
    if(bfinish)
    {
        ack.set_process(HelloKittyMsgData::EventProcess_final);
    }

    return true;

}
bool  EventLoseBoy::bindforother()
{
    return false;
}



bool EventLoseBoy::init(DWORD bindBuild)
{
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(eventConf)
    {
        if(bindBuild != 0)
        {
            m_InsId = m_rManager.getUser().m_buildManager.getAnyBuildById(bindBuild);
        }
        if(m_InsId == 0)
        {
            m_InsId = m_rManager.getUser().m_buildManager.getAnyBuildBytype(eventConf->buildevent,false);

        }
    }
    return EventBase::init(bindBuild);

}

EventLoseBoy::EventLoseBoy(EventManager &rManager,DWORD Id,DWORD intraltimer):EventBase(rManager,Id,HelloKittyMsgData::PlayerEvent_LoseBoy,intraltimer)
{
}



/************************************EventGoldHill************************************************/
bool EventGoldHill::OpRoad(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack)//操作该建筑
{
    if(EventBase::OpRoad(PlayerId,ack))
    {
        return true;
    }
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(!eventConf)
    {
        ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
        return true;
    }
    DWORD nowTimer = SceneTimeTick::currentTime.sec();
    auto iter = m_otherOpTimer.find(PlayerId);
    if(iter != m_otherOpTimer.end())
    {
        if(nowTimer < iter->second  + eventConf->event->cooltimer()*60)
        {
            ack.set_result(HelloKittyMsgData::PlayerOpEventResult_Cool);
            return true;


        }


    }
    m_otherOpTimer[PlayerId] = nowTimer;


    bool bfinish = false;
    m_step++;
    if(m_step >= eventConf->target.size())
    {
        bfinish = true;
    }
    if(!bfinish)
    {
        NoticeBindBuild(true); 
    }
    if(bfinish)
    {
        ack.set_process(HelloKittyMsgData::EventProcess_final);
    }

    return true;



}

bool EventGoldHill::UseIncon()
{
    return true;
}

DWORD EventGoldHill::getOpReserverTimer(QWORD PlayerId)
{
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(!eventConf)
    {
        return 0;
    }
    DWORD nowTimer = SceneTimeTick::currentTime.sec();
    DWORD cooltimer = eventConf->event->cooltimer()*60;
    if(cooltimer == 0)
    {
        return 0;
    }
    auto it = m_otherOpTimer.find(PlayerId);
    if(it == m_otherOpTimer.end())
        return 0;
    return it->second + cooltimer > nowTimer ? it->second + cooltimer - nowTimer : 0;
}


bool EventGoldHill::init(DWORD bindBuild)//初始化
{
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(eventConf && eventConf->buildevent.size() > 0 && eventConf->target.size() > 0)
    {
        if(bindBuild != 0)
        {
            m_InsId = bindBuild;

        }
        if(m_InsId == 0)
        {
            m_InsId = eventConf->buildevent[0];

        }

    }
    return EventBase::init(bindBuild);

}



EventGoldHill::EventGoldHill(EventManager &rManager,DWORD Id,DWORD intraltimer):EventBase(rManager,Id,HelloKittyMsgData::PlayerEvent_GoldHill,intraltimer)
{
}



/****************************************EventFixBuild*************************************************/
bool EventFixBuild::OpRoad(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack)//操作该建筑
{
    if(EventBase::OpRoad(PlayerId,ack))
    {
        return true;
    }
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(!eventConf)
    {
        ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
        return true;
    }
    DWORD nowTimer = SceneTimeTick::currentTime.sec();
    auto iter = m_otherOpTimer.find(PlayerId);
    if(iter != m_otherOpTimer.end())
    {
        if(nowTimer < eventConf->event->cooltimer()*60 + iter->second)
        {
            ack.set_result(HelloKittyMsgData::PlayerOpEventResult_Cool);
            return true;


        }


    }
    m_otherOpTimer[PlayerId] = nowTimer;


    bool bfinish = false;
    BYTE rate = zMisc::randBetween(0,100);
    if(m_step < eventConf->target.size())
    {
        if(rate  <= eventConf->target[m_step])
        {
            bfinish = true;
        }

    }
    else
    {
        bfinish = true;
    }
    if(!bfinish)
    {
        m_step++;

    }
    if(bfinish)
    {
        ack.set_process(HelloKittyMsgData::EventProcess_final);
    }
    else
    {
        NoticeBindBuild(true);
    }

    return true;


}

DWORD EventFixBuild::getOpReserverTimer(QWORD PlayerId)
{
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(!eventConf)
    {
        return 0;
    }
    DWORD nowTimer = SceneTimeTick::currentTime.sec();
    DWORD cooltimer = eventConf->event->cooltimer()*60;
    if(cooltimer == 0)
    {
        return 0;
    }
    auto it = m_otherOpTimer.find(PlayerId);
    if(it == m_otherOpTimer.end())
        return 0;
    return it->second + cooltimer > nowTimer ? it->second + cooltimer - nowTimer : 0;
}





bool EventFixBuild::init(DWORD bindBuild)//初始化
{
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(eventConf)
    {
        if(bindBuild != 0)
        {
            m_InsId = m_rManager.getUser().m_buildManager.getAnyBuildById(bindBuild);

        }
        if(m_InsId == 0)
        {
            m_InsId = m_rManager.getUser().m_buildManager.getAnyBuildBytype(eventConf->buildevent,false);

        }

    }
    BuildBase *temp = m_rManager.getUser().m_buildManager.getBuild(m_InsId);
    if(temp )
    {
        temp->setBreak(true);
    }
    return EventBase::init(bindBuild);


}


EventFixBuild::EventFixBuild(EventManager &rManager,DWORD Id):EventBase(rManager,Id,HelloKittyMsgData::PlayerEvent_FixBuild,0)
{
} 


bool EventFixBuild::canDel()
{
    return true;
}

/******************************************************EventPlant**************************************/                
bool EventPlant::OpRoad(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack)//操作该建筑
{
    if(EventBase::OpRoad(PlayerId,ack))
    {
        return true;
    }
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(!eventConf)
    {
        ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
        return true;
    }
    bool bfinish = false;
    DWORD nowTimer = SceneTimeTick::currentTime.sec();  
    switch(m_step)
    {
        case ePlanteStateNone: //主人种植
            {
                if(PlayerId != m_rManager.getUser().charid)
                {
                    ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
                    return true;
                }
                m_step = ePlanteStatePlant;
                m_EndTimer = nowTimer + eventConf->event->timer()*60;
                LOG_ERR("Plant getTotalReserverTimer: %d",getTotalReserverTimer());
            }
            break;
        case ePlanteStatePlant://玩家加速
            {
                if(m_otherOpTimer.find(PlayerId) != m_otherOpTimer.end())
                {
                    ack.set_result(HelloKittyMsgData::PlayerOpEventResult_Cool);
                    return true;

                }
                if(eventConf->target.size() > 0)
                {
                    DWORD Rate = eventConf->target[0];
                    if(nowTimer < m_EndTimer && Rate >0 && Rate < 100)
                    {
                        DWORD olddiff = (m_EndTimer - nowTimer);
                        DWORD diff = (m_EndTimer - nowTimer)*(100 - Rate) / (float)(100);
                        m_EndTimer = nowTimer + diff;
                        m_otherOpTimer[PlayerId] = nowTimer;
                        LOG_ERR(" %lu upspeed getTotalReserverTimer: %d old %d ",PlayerId,getTotalReserverTimer(),olddiff);

                    }
                }

            }
            break;
        case ePlanteStatemature://主人收割
            {
                if(PlayerId != m_rManager.getUser().charid)
                {
                    ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
                    return true;
                }
                m_step = ePlanteStatecool;
                m_EndTimer = nowTimer + eventConf->reflushtimemin*60;
                bfinish = true; 
            }
            break;
        default:
            ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
            break;
    }

    if(bfinish)
    {
        ack.set_process(HelloKittyMsgData::EventProcess_final);
    }
    NoticeBindBuild(true);

    return true;


}



bool EventPlant::needpushevent()
{
    return false;
}

bool EventPlant::init(DWORD bindBuild)//初始化
{
    const pb::Conf_t_event *eventConf = tbx::event().get_base(m_Id); 
    if(eventConf && eventConf->buildevent.size() > 0 )
    {
        if(bindBuild != 0)
        {
            m_InsId = m_rManager.getUser().m_buildManager.getAnyBuildById(bindBuild);
        }
        if(m_InsId == 0)
        {
            m_InsId = m_rManager.getUser().m_buildManager.getAnyBuildById(eventConf->buildevent,false);
        }
    }
    return EventBase::init(bindBuild);


}


EventPlant::EventPlant(EventManager &rManager,DWORD Id):EventBase(rManager,Id,HelloKittyMsgData::PlayerEvent_Plant,0)
{

}

bool EventPlant::isOverTimer(DWORD nowTimer)
{
    if(m_step == ePlanteStatePlant && EventBase::isOverTimer(nowTimer))
    {
        m_step = ePlanteStatemature;
        m_otherOpTimer.clear();
        NoticeBindBuild(true);
    }
    else if(m_step == ePlanteStatecool && EventBase::isOverTimer(nowTimer))
    {
        m_step = ePlanteStateNone;
        NoticeBindBuild(true);
    }
    return false;


}

DWORD EventPlant::getTotalReserverTimer()
{
    if(m_step == ePlanteStatePlant || m_step == ePlanteStatecool)
        return EventBase::getTotalReserverTimer();
    return 0;

}
bool EventPlant::needdestroy()
{
    return false;
}


bool EventPlant::canDel()
{
    return true;
}

bool EventPlant::canOp(QWORD PlayerId)
{
    LOG_ERR("checkop :%lu ower %lu step %d",PlayerId,m_rManager.getUser().charid,m_step);
    switch(m_step)
    {
        case ePlanteStateNone: //主人种植
            {
                return PlayerId == m_rManager.getUser().charid;
            }
            break;
        case ePlanteStatePlant://玩家加速
            {
                return m_otherOpTimer.find(PlayerId) == m_otherOpTimer.end();
            }
            break;
        case ePlanteStatemature://主人收割
            {
                return PlayerId == m_rManager.getUser().charid;
            }
            break;
        default:
            break;
    }
    return false;


}
/******************************************************EventManager************************************/
EventManager::EventManager(SceneUser& rUser):m_rUser(rUser)
{
}

SceneUser& EventManager::getUser()
{
    return m_rUser;
}

void EventManager::timerCheck()
{
    DWORD NowTimer = SceneTimeTick::currentTime.sec();  
    for(auto iter = m_vecEvent.begin(); iter != m_vecEvent.end();)
    {
        EventBase *pEvent = *iter;
        if(pEvent == NULL || pEvent->isOverTimer(NowTimer))
        {
            if(pEvent)
            {
                pEvent->destroy();
            }
            iter = m_vecEvent.erase(iter);
            SAFE_DELETE(pEvent);

        }
        else
        {
            ++iter;
        }

    }
    if(!m_rUser.is_online())
    {
        return ;
    }
    const std::unordered_map<unsigned int, const pb::Conf_t_event *> &tbxMap = tbx::event().getTbxMap();
    std::unordered_map<unsigned int, const pb::Conf_t_event *> sel;
    DWORD allRate  = 0;
    for(auto iter = tbxMap.begin();iter != tbxMap.end();++iter)
    {
        const pb::Conf_t_event* pconf = iter->second;
        if(!pconf)
        {
            continue;
        }
        //TODO:
        //先判定任务
        if(pconf->event->task() > 0) 
        {
            if(!m_rUser.m_taskManager.checkTaskHasDone(pconf->event->task()))
            {
                continue;
            }
        }
        //然后等级
        if(!m_rUser.checkLevel(pconf->event->level()))
        {
            continue;
        }
        //建筑数
        if(!checkCanCreateByBuild(pconf,false,NowTimer))
        {
            continue;
        }
        HelloKittyMsgData::PlayerEventType EventType = static_cast<HelloKittyMsgData::PlayerEventType>(pconf->event->order());

        //其他事件
        //创建
        if(EventType != HelloKittyMsgData::PlayerEvent_Plant)
        {
            auto it = m_lastCreateTime.find(iter->first);
            if(it != m_lastCreateTime.end())
            {
                if(NowTimer < it->second)
                {
                    continue;
                }
            }
        }

        bool bSame = false;
        for(auto it = m_vecEvent.begin();it != m_vecEvent.end();++it)
        {
            EventBase * pbase = *it;
            if(pbase && pbase->getId() == iter->first)
            {
                bSame = true;
                break;
            }
            const pb::Conf_t_event *eventConf = tbx::event().get_base(pbase->getId()); 
            if(!eventConf)
            {
                continue;
            }

            if(pconf->event->send() == DOSNED && pconf->event->send() == eventConf->event->send())
            {
                bSame = true;
                break;
            }

        }
        if(bSame)
        {
            continue;
        }
        DWORD Nexttimer =  NowTimer + zMisc::randBetween(pconf->reflushtimemin*60,pconf->reflushtimemax*60);
        if(EventType != HelloKittyMsgData::PlayerEvent_Plant)
        {
            m_lastCreateTime[iter->first] = Nexttimer;
            sel[iter->first] = pconf;
            allRate += pconf->event->rate();
        }
        else
        {
            createEvent(iter->first,pconf);
        }
    }
    DWORD rate = zMisc::randBetween(0,allRate); 
    for(auto iter = sel.begin();iter != sel.end();++iter)
    {
        if(iter->second->event->rate() >= rate)
        {
            createEvent(iter->first,iter->second);
            break;
        }
        else
        {
            rate -= iter->second->event->rate();
        }

    }


}
void EventManager::createEvent(DWORD EventID,const pb::Conf_t_event* pconf)
{
    HelloKittyMsgData::PlayerEventType EventType = static_cast<HelloKittyMsgData::PlayerEventType>(pconf->event->order());

    EventBase *pevent =NULL;
    switch (EventType)
    {
        case  HelloKittyMsgData::PlayerEvent_LoseBoy :
            {
                pevent = new EventLoseBoy(*this,EventID,pconf->event->timer()*60);
            }
            break;
        case HelloKittyMsgData::PlayerEvent_GoldHill :
            {
                pevent = new EventGoldHill(*this,EventID,pconf->event->timer()*60);
            }
            break;
        case HelloKittyMsgData::PlayerEvent_FixBuild:
            {
                pevent = new EventFixBuild(*this,EventID); 
            }
            break;

        case HelloKittyMsgData::PlayerEvent_Plant:
            {
                pevent = new EventPlant(*this,EventID);
            }
            break;
        default:
            break;

    }
    if(pevent)
    {
        if(!pevent->init())
        {
            SAFE_DELETE(pevent);
        }
        else
        {
            m_vecEvent.push_back(pevent);

        }

    }
}

void EventManager::OpRoad(QWORD PlayerId,const HelloKittyMsgData::Builditype &rBuild,HelloKittyMsgData::AckopBuilding &ack)
{
    bool find = false;
    for(auto iter = m_vecEvent.begin(); iter != m_vecEvent.end();)
    {
        EventBase *pEvent = *iter;
        if(pEvent == NULL )
        {
            ++iter;
            continue;

        }
        bool bsel = false;
        if(rBuild.isicon() == 1 )
        {
            bsel = pEvent->UseIncon();
        }
        else 
        {
            bsel = !pEvent->UseIncon();
        }
        if(!bsel)
        {
            ++iter;
            continue;
        }
        if(pEvent->getInsId() == rBuild.buildid())
        {
            find = true;
            pEvent->OpRoad(PlayerId,ack);
            if(ack.result() ==HelloKittyMsgData::PlayerOpEventResult_Suc && ack.process() == HelloKittyMsgData::EventProcess_final)
            {
                DoAward(PlayerId,ack,pEvent);
                if(pEvent->needdestroy())
                {
                    pEvent->destroy(); 
                    SAFE_DELETE(pEvent);
                    iter = m_vecEvent.erase(iter);
                }
            }
            break;
        }
        ++iter;

    }

    if(!find)
    {
        ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
        return ;
    }

}


void EventManager::DoAward(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack,EventBase *pEvent)
{
    if(pEvent ==NULL)
        return;
    const pb::Conf_t_event *eventConf = tbx::event().get_base(pEvent->getId()); 
    if(!eventConf)
        return ;
    //Doself
    const HelloKittyMsgData::vecAward * pvec = NULL;
    if(PlayerId == m_rUser.charid)
    {
        pvec = &(eventConf->rewardower);
    }
    else
    {
        pvec = &(eventConf->rewardguess);
    }
    if(pvec)
    {
        for(int i = 0; i != pvec->award_size();i++)
        {
            const HelloKittyMsgData::Award& rcofig = pvec->award(i);
            HelloKittyMsgData::Award* pAward = ack.add_award();
            if(pAward)
            {
                *pAward = rcofig;

            }
        }


    }
    //Other
    for(auto it = pEvent->GetOpPlayer().begin(); it != pEvent->GetOpPlayer().end();it++)
    {
        QWORD opPlayer = it->first;

        if(PlayerId == opPlayer )
        {
            continue;
        }
        if(opPlayer == m_rUser.charid)
        {
            getUser().DoAward(opPlayer,eventConf->rewardower,pEvent->getId(),true);
        }
        else
        {
            getUser().DoAward(opPlayer,eventConf->rewardguess,pEvent->getId(),true);

        }

    }
    //self
    if(PlayerId != m_rUser.charid && pEvent->GetOpPlayer().find(m_rUser.charid) ==  pEvent->GetOpPlayer().end())
    {
        getUser().DoAward(m_rUser.charid,eventConf->rewardower,pEvent->getId(),true);  
    }

}

EventManager::~EventManager()
{
    for(auto it = m_vecEvent.begin(); it != m_vecEvent.end(); it++)
    {
        SAFE_DELETE(*it); 

    }
}


bool EventManager::load(const HelloKittyMsgData::Serialize& binary)
{
    for(int i = 0; i != binary.event_size();i++)
    {
        const HelloKittyMsgData::SaveForEvent& rEvent = binary.event(i);
        DWORD eventid =  rEvent.eventid();
        DWORD flushtimer = rEvent.flushtimer();
        m_lastCreateTime[eventid] = flushtimer;
        if(rEvent.isrun() > 0)
        {
            const pb::Conf_t_event *pConf = tbx::event().get_base(eventid); 
            if(!pConf)
            {
                continue;
            }
            EventBase *pevent =NULL;
            HelloKittyMsgData::PlayerEventType EventType = static_cast<HelloKittyMsgData::PlayerEventType>(pConf->event->order());
            switch (EventType)
            {
                case  HelloKittyMsgData::PlayerEvent_LoseBoy :
                    {
                        pevent = new EventLoseBoy(*this,eventid,0);
                    }
                    break;
                case HelloKittyMsgData::PlayerEvent_GoldHill :
                    {
                        pevent = new EventGoldHill(*this,eventid,0);
                    }
                    break;
                case HelloKittyMsgData::PlayerEvent_FixBuild:
                    {
                        pevent = new EventFixBuild(*this,eventid); 
                    }
                    break;
                case HelloKittyMsgData::PlayerEvent_Plant:
                    {
                        pevent = new EventPlant(*this,eventid);
                    }
                    break;
                default:
                    break;
            }
            if(pevent)
            {
                if(!pevent->init(rEvent))
                {
                    SAFE_DELETE(pevent);
                }
                else
                {
                    m_vecEvent.push_back(pevent);

                }

            }

        }



    }
    return true;

}

bool EventManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_lastCreateTime.begin();iter != m_lastCreateTime.end();iter++)
    {
        HelloKittyMsgData::SaveForEvent *pSaveEvent = binary.add_event();
        if(!pSaveEvent)
            continue;
        pSaveEvent->set_eventid(iter->first);
        pSaveEvent->set_flushtimer(iter->second);
        EventBase * pCurEvent = NULL;
        for(auto it = m_vecEvent.begin(); it != m_vecEvent.end(); it++)
        {
            if((*it) && (*it)->getId() == iter->first)
            {
                pCurEvent = *it;
                break;
            }
        }
        if(!pCurEvent)
        {
            pSaveEvent->set_isrun(0);
            continue;
        }
        else
        {
            pSaveEvent->set_isrun(1);
        }

        pSaveEvent->set_endtimer(pCurEvent->getEndTimer());
        pSaveEvent->set_value(pCurEvent->getStep());
        for(auto it =  pCurEvent->GetOpPlayer().begin();it != pCurEvent->GetOpPlayer().end();++it)
        {
            pSaveEvent->add_opplayer(it->first);
            pSaveEvent->add_optimer(it->second);

        }
        if(!pCurEvent->UseIncon())
        {
            BuildBase* pbuildbase = getUser().m_buildManager.getBuild(pCurEvent->getInsId());  
            if(pbuildbase)
            {
                pSaveEvent->set_buildid(pbuildbase->getTypeID());
            }
        }
        else
        {
            pSaveEvent->set_buildid(pCurEvent->getInsId());
        }
    }
    return true;

}


bool EventManager::canDel()
{
    for(auto iter = m_vecEvent.begin(); iter != m_vecEvent.end(); iter++)
    {
        EventBase *pEvent = *iter;
        if(pEvent && !pEvent->canDel())
        {
            return false;
        }
    }
    return true;
}


void EventManager::fullMessage(QWORD PlayerId,HelloKittyMsgData::Evententer *info)
{
    HelloKittyMsgData::EventNotice *pAck = info->mutable_eventinfo(); 
    HelloKittyMsgData::EventNotice& ack = *pAck;
    ack.set_charid(0);
    ack.set_eventid(0);
    for(auto iter = m_vecEvent.begin();iter != m_vecEvent.end(); iter++)
    {
        EventBase *pBase = *iter;
        if(pBase)
        {
            pBase->OnlineSend(PlayerId,info);
        }
    }
}



bool EventManager::checkCanCreateByBuild(const pb::Conf_t_event* pConf,bool isGm,DWORD checktimer)
{
    if(!isGm && m_rUser.m_buildManager.getBuildNum() < pConf->event->needbuild())
    {
        return false;
    }
    HelloKittyMsgData::PlayerEventType EventType = static_cast<HelloKittyMsgData::PlayerEventType>(pConf->event->order());
    switch (EventType)
    {
        case  HelloKittyMsgData::PlayerEvent_LoseBoy :
            {
                return  getUser().m_buildManager.getAnyBuildBytype(pConf->buildevent,false) > 0; 
            }
            break;
        case HelloKittyMsgData::PlayerEvent_GoldHill :
            {
                return true;
            }
            break;
        case HelloKittyMsgData::PlayerEvent_FixBuild:
            {
                return getUser().m_buildManager.getAnyBuildBytype(pConf->buildevent,false) > 0;

            }
            break;
        case HelloKittyMsgData::PlayerEvent_Plant:
            {
                return getUser().m_buildManager.getAnyBuildById(pConf->buildevent,false) > 0;
            }
            break;
        default:
            return false;
            break;
    }
    return false;

}

void EventManager::Gmopenevent(DWORD eventid)
{
    DWORD NowTimer = SceneTimeTick::currentTime.sec();  
    for(auto iter = m_vecEvent.begin(); iter != m_vecEvent.end();)
    {
        EventBase *pEvent = *iter;
        if(pEvent == NULL || pEvent->needdestroy())
        {
            if(pEvent)
            {
                pEvent->destroy();
            }
            iter = m_vecEvent.erase(iter);
            SAFE_DELETE(pEvent);

        }
        else
        {
            ++iter;
        }

    }
    if(!m_rUser.is_online())
    {
        return ;
    }
    const std::unordered_map<unsigned int, const pb::Conf_t_event *> &tbxMap = tbx::event().getTbxMap();
    auto iter = tbxMap.find(eventid);
    if(iter != tbxMap.end())
    {
        const pb::Conf_t_event* pconf = iter->second;
        if(!pconf)
        {
            return ;
        }
        HelloKittyMsgData::PlayerEventType EventType = static_cast<HelloKittyMsgData::PlayerEventType>(pconf->event->order());
        //建筑数
        if(!checkCanCreateByBuild(pconf,true,NowTimer))
        {
            return ;
        }
        bool bSame = false;
        for(auto it = m_vecEvent.begin();it != m_vecEvent.end();++it)
        {
            EventBase * pbase = *it;
            if(pbase && pbase->getId() == iter->first)
            {
                bSame = true;
                break;
            }
            const pb::Conf_t_event *eventConf = tbx::event().get_base(pbase->getId()); 
            if(!eventConf)
            {
                continue;
            }

            if(pconf->event->send() == DOSNED && pconf->event->send() == eventConf->event->send())
            {
                bSame = true;
                break;
            }

        }
        if(bSame)
        {
            return ;
        }
        DWORD Nexttimer =  NowTimer + zMisc::randBetween(pconf->reflushtimemin*60,pconf->reflushtimemax*60);
        if(EventType != HelloKittyMsgData::PlayerEvent_Plant)
        {
            m_lastCreateTime[iter->first] = Nexttimer;
        }
        createEvent(iter->first,pconf);


    }


}
