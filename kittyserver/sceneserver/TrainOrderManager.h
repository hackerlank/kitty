#ifndef TRAINORDER_MANAGER_H__
#define TRAINORDER_MANAGER_H__
#include <set>
#include <map>
#include "zType.h"
#include "trainorder.pb.h"
#include "serialize.pb.h"
#include "login.pb.h"
#include "Misc.h"

class SceneUser;
const DWORD MaxTrain = 3;
class TrainOrderManager
{
    public:
        void timerCheck();
        ~TrainOrderManager();
        TrainOrderManager(SceneUser& rUser);
        SceneUser& getUser() {return m_rUser;}
        void load(const HelloKittyMsgData::Serialize& binary);
        void save(HelloKittyMsgData::Serialize& binary);
        bool fullMessage(HelloKittyMsgData::UserBaseInfo &binary);
        bool fullMessage(HelloKittyMsgData::EnterGardenInfo &binary);
        HelloKittyMsgData::Train* getTrainByID(DWORD trainID);
        void loadselftrain(HelloKittyMsgData::Train* ptrain,HelloKittyMsgData::CarriageLoad* pselectload);//设置车厢满状态，设置帮助信息接受状态，应答玩家，检查能否开拔 ：设置开拔状态，设置运行到站时间，设置地名.
        void loadothertrain(QWORD charid,DWORD trainID);//设置 厢满状态，设置帮助信息接受状态，应答玩家
        bool checkrun(HelloKittyMsgData::Train *ptrain);
        void sethelpinfo(HelloKittyMsgData::Train* ptrain,HelloKittyMsgData::CarriageLoad* pselectload);
        void answerHelp(HelloKittyMsgData::Train *ptrain);
        void setTrainawardget(HelloKittyMsgData::Train *ptrain, HelloKittyMsgData::CarriageAward* pselectaward);
        void setTrainReserverTimer(HelloKittyMsgData::Train *ptrain);
        void clearTrainCD(HelloKittyMsgData::Train *ptrain);
        eParam getParamByTrain(DWORD TrainId); 
        void checkgetAwardForLoad(HelloKittyMsgData::vecAward &rvecAward,const HelloKittyMsgData::Award& rneeditem,bool bself);
        bool upGrade(DWORD trainid,DWORD effectId);
        void checkEffectGold(DWORD trainNo,HelloKittyMsgData::vecAward &rvecAward);
        void checkEffectCD(HelloKittyMsgData::Train* ptrain);

    public:
        void updatetrainToCli(const HelloKittyMsgData::Train &rTrain);
        bool checkflushTrain(HelloKittyMsgData::Train* ptrain );
        bool newTrain(DWORD trainID);
        bool checkNewTrain();
    private:
        void getClientInfo(const HelloKittyMsgData::Train &rTrain,HelloKittyMsgData::TrainForClient *pOrder,bool bSelf);
        void setClientInfoAll(HelloKittyMsgData::TrainForClient *pOrder,const HelloKittyMsgData::Train &rTrain);
        void setClientShowLoadAward(HelloKittyMsgData::TrainForClient *pOrder,bool bSelf);
    private:
        bool checkFirstTrain(DWORD trainID);
        bool DoInitTrain(DWORD trainID,const std::vector<DWORD> &rloadvec,const std::vector<DWORD> &rawardvec,const std::vector<DWORD>* ploadnumvec = NULL ,const std::vector<DWORD>*pawardnumvec = NULL,DWORD bfirstCD = 0);
        bool updateEffect(const DWORD trainID);

    private:
        SceneUser& m_rUser;
        std::map<DWORD,HelloKittyMsgData::Train> m_mapOrder;
        std::map<DWORD,HelloKittyMsgData::BuildEffect> m_mapeffect;
};
#endif// TRAINORDER_MANAGER_H__
