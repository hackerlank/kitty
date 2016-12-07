#ifndef MARKETMANAGER_MANAGER_H__
#define MARKETMANAGER_MANAGER_H__
#include <set>
#include <map>
#include "zType.h"
#include "market.pb.h"
#include "serialize.pb.h"

class SceneUser;
class marketManager
{
    public:
        bool timerCheck();
        ~marketManager();
        marketManager(SceneUser& rUser);
        SceneUser& getUser() {return m_rUser;}
        void load(const HelloKittyMsgData::Serialize& binary);
        void save(HelloKittyMsgData::Serialize& binary);
        HelloKittyMsgData::MarketData & getMaketData();
        HelloKittyMsgData::ServantData& getservant();
        void UpdateMarket(const HelloKittyMsgData::MarketItem* pSel);
        void initmarket();
        void UpdateStateServant();
        void SetStateServantForCli(HelloKittyMsgData::ServantData *pservant);
        void setservant(HelloKittyMsgData::ServantState state);
        void init();
        bool maketisopen();
    private:
        SceneUser& m_rUser;
        HelloKittyMsgData::MarketData m_market;
        HelloKittyMsgData::ServantData m_servant;
};
#endif// MARKETMANAGER_MANAGER_H__
