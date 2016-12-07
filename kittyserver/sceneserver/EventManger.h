#ifndef EVENTMANAGER_H__
#define EVENTMANAGER_H__
#include <set>
#include <vector> 
#include <map>
#include "zType.h"
#include "event.pb.h"
#include "serialize.pb.h"
#include "tbx.h"

class EventManager;
class EventBase
{
    public:
        virtual bool OpRoad(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack);//操作该建筑
        virtual void destroy() ;//销毁时该做的的事情
        virtual bool init(DWORD bindBuild = 0);//初始化
        bool  init(const HelloKittyMsgData::SaveForEvent& rEvent);
        virtual ~EventBase();
        virtual bool isOverTimer(DWORD nowTimer);//是否超时
        virtual void OnlineSend(QWORD PlayerId,HelloKittyMsgData::Evententer*info) ;//玩家进 ：上线，进入别人家
        virtual bool canDel();
        virtual bool UseIncon();
        virtual bool needdestroy();
        virtual bool bindforother();
        virtual bool needpushevent();
        virtual DWORD getTotalReserverTimer();
        virtual bool canOp(QWORD PlayerId);
        virtual DWORD getOpReserverTimer(QWORD PlayerId);



        EventBase(EventManager &rManager,DWORD Id ,HelloKittyMsgData::PlayerEventType type,DWORD intraltimer);
        DWORD getId();
        HelloKittyMsgData::PlayerEventType getType();
        DWORD getEndTimer();
        DWORD getStep() {return m_step;}
        QWORD getInsId(){return m_InsId;}
        const std::map<QWORD,DWORD> & GetOpPlayer() { return m_otherOpTimer;}
        void  NoticeBindBuild(bool add);//绑定建筑通知
        void  NoticeBindBuild(HelloKittyMsgData::Evententer* info,QWORD PlayerId);
        void  NoticeNewEvent(bool add,bool bClient);//新事件通知：事件发生时
        void  NoticeNewEvent(HelloKittyMsgData::Evententer* info);//玩家进 ：上线，进入别人家
    private:
        void setPlayerOp(QWORD charid,HelloKittyMsgData::EventBuildNotice &ack);
        void setBuildId(HelloKittyMsgData::Builditype *pbuild);

    protected:
        EventManager &m_rManager;
        DWORD m_Id;//当前事件id
        HelloKittyMsgData::PlayerEventType m_type;//事件类型
        QWORD m_InsId;//当前相关建筑
        DWORD m_EndTimer; //结束时间
        std::map<QWORD,DWORD> m_otherOpTimer;//最近操作玩家
        DWORD m_step;



};

class EventLoseBoy : public EventBase
{
    public:
        virtual bool OpRoad(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack);//操作该建筑
        virtual bool init(DWORD bindBuild = 0);//初始化
        virtual ~EventLoseBoy(){}
        virtual bool bindforother();

        EventLoseBoy(EventManager &rManager,DWORD Id,DWORD intraltimer);


};

class EventGoldHill : public EventBase
{
    public:
        virtual bool OpRoad(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack);//操作该建筑
        virtual bool init(DWORD bindBuild = 0);//初始化
        virtual ~EventGoldHill(){};
        virtual bool UseIncon();
        virtual DWORD getOpReserverTimer(QWORD PlayerId);
        EventGoldHill(EventManager &rManager,DWORD Id,DWORD intraltimer);

};

class EventFixBuild : public EventBase
{
    public:

        virtual bool OpRoad(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack);//操作该建筑
        virtual bool init(DWORD bindBuild = 0);//初始化
        virtual ~EventFixBuild(){};
        EventFixBuild(EventManager &rManager,DWORD Id);
        virtual bool canDel();
        virtual DWORD getOpReserverTimer(QWORD PlayerId);


};

class EventPlant : public EventBase
{
    public:
        enum ePlanteState
        {
            ePlanteStateNone = 0,//未播种
            ePlanteStatePlant = 1,//已种下
            ePlanteStatemature = 2,//以成熟
            ePlanteStatecool = 3,//已收割，待冷却

        };
        virtual bool OpRoad(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack);//操作该建筑
        virtual bool init(DWORD bindBuild = 0);//初始化
        virtual ~EventPlant(){};
        EventPlant(EventManager &rManager,DWORD Id);
        virtual bool isOverTimer(DWORD nowTimer);
        virtual bool canDel();
        virtual bool needpushevent();
        virtual DWORD getTotalReserverTimer(); 
        virtual bool needdestroy();
        virtual bool canOp(QWORD PlayerId);



};


class SceneUser;
class EventManager
{
    public:
        enum SendType
        {
            DOSNEDNOne = 0,
            DOSNED = 1,
        };
        EventManager(SceneUser& rUser);
        SceneUser& getUser();
        void timerCheck();
        void OpRoad(QWORD PlayerId,const HelloKittyMsgData::Builditype &rBuild,HelloKittyMsgData::AckopBuilding &ack);
        void OnlineSend(QWORD PlayerId,HelloKittyMsgData::Evententer*info);
        bool load(const HelloKittyMsgData::Serialize& binary);
        bool save(HelloKittyMsgData::Serialize& binary);
        bool canDel();
        ~EventManager();
        void fullMessage(QWORD PlayerId,HelloKittyMsgData::Evententer *info);
    public:
        void Gmopenevent(DWORD eventid);
    private:
        bool checkCanCreateByBuild(const pb::Conf_t_event* pconf,bool isGm ,DWORD dwchecktimer);

        void DoAward(QWORD PlayerId,HelloKittyMsgData::AckopBuilding &ack,EventBase *pEvent);
        void createEvent(DWORD EventID,const pb::Conf_t_event* pconf); 
    private:
        SceneUser& m_rUser;
        std::map<DWORD,DWORD> m_lastCreateTime;//上次创建时间
        std::vector<EventBase *> m_vecEvent;//
};



#endif// EVENTMANAGER_H__
