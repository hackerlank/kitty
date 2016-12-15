#ifndef BURST_EVENT_MANAGER_H
#define BURST_EVENT_MANAGER_H 
#include <set>
#include "zType.h"
#include "serialize.pb.h"
#include "login.pb.h"

class SceneUser;
class BurstEventManager 
{
    public:
        BurstEventManager(SceneUser *owner);
        ~BurstEventManager();
        //加载
        bool load(const HelloKittyMsgData::Serialize& binary);
        //保存
        bool save(HelloKittyMsgData::Serialize& binary);
        //loop
        bool loop(const bool online = false);
        //更新
        bool updateBurstEvent(const QWORD eventID);
        //全更
        bool flushEvent();
        //操作事件
        bool opEvent(const HelloKittyMsgData::ReqOpBurstEvent *cmd);
        //删除事件
        bool delEvent(const QWORD tempid,const HelloKittyMsgData::BurstEventStatus &delType = HelloKittyMsgData::BES_Del_Other);
        //填充messag
        bool fullMessage(HelloKittyMsgData::UserBaseInfo &binary);
        //重置
        void reset();
        bool delEvent(const DWORD colID);
        bool newEvent(const DWORD colID);
    private:
        //检查事件是否达到目标
        bool checkTarget(const QWORD tempid);
        //获得事件实例指针
        HelloKittyMsgData::BurstEvent* getEvent(const QWORD tempid);
    private:
        SceneUser *m_owner;
        std::map<QWORD,HelloKittyMsgData::BurstEvent> m_eventMap;
        std::map<DWORD,QWORD> m_colIDMap;
        std::set<DWORD> m_npcKeySet;
};


#endif

