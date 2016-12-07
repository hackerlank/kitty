#ifndef PLAYERACTIVECONFIG_H__
#define PLAYERACTIVECONFIG_H__
#include <map> 
#include <set>
#include "zType.h"
#include "gmtool.pb.h"
#include "zSingleton.h"
#include "common.h"

enum active_state
{
    state_None = 0,
    state_wait = 1,
    state_run = 2,
    state_finish = 3,
    state_del = 4,
};
class PlayerActiveConfig :public Singleton<PlayerActiveConfig>
{
    public:
        void timerCheck();
        bool init();
        bool add(const HelloKittyMsgData::PlayerActive &rinfo);
        bool del(DWORD activeid);
        const HelloKittyMsgData::PlayerActive * getinfo(DWORD activeid) const;
        const std::map<DWORD,HelloKittyMsgData::PlayerActive>& getAllinfo() const;
    public:
        //查询一个事件被那些活动监听
        const std::set<DWORD> * getListen(HelloKittyMsgData::ActiveConditionType type) const;
        //判定一个活动是增量还是存量
        bool isSaveType(HelloKittyMsgData::ActiveConditionType type) const;
        const std::set<DWORD> * getlistBystate(active_state state) const;
        bool isNeedDel(DWORD now,DWORD endtime,HelloKittyMsgData::ActiveConditionType type);
        bool isRank(HelloKittyMsgData::ActiveConditionType type) const; //排行榜数据，小为胜，0除外
        active_state getState(DWORD activeID);
        //更新限时活动奖励的数量
        bool updateDB(const DWORD activeID);

    private:
        bool readactive(DWORD fID,activedata &m_base);
    private:
        void setState(DWORD activeID,active_state new_state);

        std::map<active_state,std::set<DWORD> > m_mapState;
        std::map<DWORD,active_state> m_mapID;
        std::map<HelloKittyMsgData::ActiveConditionType,std::set<DWORD> > m_maplisten;

    private:
        std::map<DWORD,HelloKittyMsgData::PlayerActive> m_mapActive;

};
#endif// PLAYERACTIVECONFIG_H__
