#ifndef UNITYBUILD_MANGER_H__
#define UNITYBUILD_MANGER_H__
#include <set>
#include <map>
#include "zType.h"
#include "unitbuild.pb.h"
#include "serialize.pb.h"
#include "login.pb.h"

class SceneUser;
class UnityManager
{
    public:
        ~UnityManager();
        UnityManager(SceneUser& rUser);
        SceneUser& getUser() {return m_rUser;}
        void timercheck();
        void load(const HelloKittyMsgData::Serialize& binary);
        void save(HelloKittyMsgData::Serialize& binary);
        HelloKittyMsgData::UnitPlayerColId *getColById(DWORD colid);
        //接口
        DWORD getUnityBuildNums(DWORD buildId);
        DWORD getUnityBuildNums(QWORD PlayerOther,DWORD buildId);
        const std::map<DWORD,HelloKittyMsgData::UnitPlayerColId> & getselfinfo(){return m_mapUnitColId;}
        //public
        //self
        void  addUnitPlayerCol(HelloKittyMsgData::UnitPlayerColId & rInfo,bool bSendCli);
        void  delUnitPlayerCol(DWORD colId);
        //Other
        static void AddBuildTimes(QWORD PlayerOther,DWORD colId);
        static void SendMailToInviteFalse(QWORD PlayerOther,QWORD PlayerOtherFriend,DWORD BuildId);
        void NoticeUpdateUnityForAll(HelloKittyMsgData::UnitRunInfo &info);
        void addUnitBuildTimes(DWORD colId);
        void pushunitbuild(HelloKittyMsgData::UnitRunInfo &info);
        void synunitbuildlevel(HelloKittyMsgData::UnitRunInfo &info);
        void sendmailforcancel(HelloKittyMsgData::UnitRunInfo &info);
        void sendmailforfinish(HelloKittyMsgData::UnitRunInfo &info);
        //new case 
        bool getCliColInfoByColId(HelloKittyMsgData::UnitColInfoForCli *pinfo,DWORD ColID);
        void updateCliColInfoByColId(DWORD ColID);
        //gift
        bool DoGift(DWORD ColID,QWORD FriendCharid,DWORD giftID);
        //
        bool fullMessage(HelloKittyMsgData::UserBaseInfo &binary);
    private:
        void getUnityBuildInfoByInfo(HelloKittyMsgData::UnitRunInfo &info,HelloKittyMsgData::UnitRunInfoForCli* pinfo);
        bool getUnityBuildInfo(QWORD unityID,HelloKittyMsgData::UnitRunInfoForCli* pinfo);
        void ReturnResult(HelloKittyMsgData::UnitBuildResult result);


    private:
        SceneUser& m_rUser;
        std::map<DWORD,HelloKittyMsgData::UnitPlayerColId> m_mapUnitColId;
};
#endif// UNITYBUILD_MANGER_H__
