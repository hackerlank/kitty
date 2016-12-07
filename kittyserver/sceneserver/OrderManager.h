#ifndef ORDER_MANAGER_H__
#define ORDER_MANAGER_H__
#include <set>
#include <vector> 
#include <map>
#include "zType.h"
#include "order.pb.h"
#include "serialize.pb.h"
#include "login.pb.h"

class SceneUser;

class OrderManager
{
    public:
        void timerCheck();
        ~OrderManager();
        OrderManager(SceneUser& rUser);
        SceneUser& getUser() {return m_rUser;}
        void getOrderList(HelloKittyMsgData::AckOrderList& ack);
        void ReqFinishOrder(HelloKittyMsgData::AckFinishOrder& ack,DWORD colid,bool useMoney);
        void ReqFlushOrder(HelloKittyMsgData::AckFlushOrder &ack,DWORD colid);
        void load(const HelloKittyMsgData::Serialize& binary);
        void save(HelloKittyMsgData::Serialize& binary);
        void fullInfo(HelloKittyMsgData::AckReconnectInfo &info);
        bool checkbuff(HelloKittyMsgData::vecAward& awarditem);
        void ReqClearCD(HelloKittyMsgData::AckClearCD &ack,DWORD colid);
    private:
        void getawardbyItemBase(HelloKittyMsgData::OrderItem & ritem);
        void getawardbyItemExtra(HelloKittyMsgData::OrderItem & ritem);
        void InitOrderItem(HelloKittyMsgData::OrderItem & ritem, bool bfirst = false);
        void getcoolTimerForCli(HelloKittyMsgData::OrderItem & ritem);
        void GetAward(const HelloKittyMsgData::OrderItem & ritem);
        QWORD randBuildID();
        DWORD getOrderNum();

    private:
        void getRandOrderId(DWORD num,const std::set<QWORD>& setExp,std::vector<QWORD>& sel );
        void sendUpadate(const HelloKittyMsgData::OrderItem &rItem);
        bool hasBuild();
        SceneUser& m_rUser;
        std::vector<HelloKittyMsgData::OrderItem> m_vecOrder;
        DWORD m_cdEndTime;
};
#endif// ORDER_MANAGER_H__
