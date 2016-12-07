#ifndef PLAYERACTIVEMANAGER_H__
#define PLAYERACTIVEMANAGER_H__
#include <set>
#include <vector> 
#include <map>
#include "zType.h"
#include "playeractive.pb.h"
#include "serialize.pb.h"

class SceneUser;

class PlayerActiveManager
{
    public:
        void timerCheck();
        ~PlayerActiveManager();
        PlayerActiveManager(SceneUser& rUser);
        SceneUser& getUser() {return m_rUser;}
        void load(const HelloKittyMsgData::Serialize& binary);
        void save(HelloKittyMsgData::Serialize& binary);
        HelloKittyMsgData::PlayerActiveSave * getSaveInfo(DWORD activeid);
        DWORD getselfparam(const DWORD activeID,HelloKittyMsgData::ActiveConditionType type);
        void doaction(HelloKittyMsgData::ActiveConditionType type,DWORD param,DWORD activeID = 0,DWORD subCondition = 0,DWORD subType = 0); 
    public:
        void ReqgetActiveAward(const HelloKittyMsgData::ReqgetActiveAward* cmd);
        void ReqgetPlayerActiveList(const HelloKittyMsgData::ReqgetPlayerActiveList* cmd = NULL);
        void update(const std::set<DWORD> &updateSet);
    private:
        SceneUser& m_rUser;
        std::map<DWORD,HelloKittyMsgData::PlayerActiveSave> m_mapActive;
};
#endif// PLAYERACTIVEMANAGER_H__
