#ifndef ORDERSYSTEMMANAGER_MANAGER_H__
#define ORDERSYSTEMMANAGER_MANAGER_H__
#include <set>
#include <map>
#include "zType.h"
#include "ordersystem.pb.h"
#include "serialize.pb.h"
#include "login.pb.h"

class SceneUser;
class ordersystemManager
{
    public:
        void timerCheck();
        ~ordersystemManager();
        ordersystemManager(SceneUser& rUser);
        SceneUser& getUser() {return m_rUser;}
        void load(const HelloKittyMsgData::Serialize& binary);
        void save(HelloKittyMsgData::Serialize& binary);
        bool fullMessage(HelloKittyMsgData::UserBaseInfo &binary);
        void UpdateOrderSystemItem(const HelloKittyMsgData::OrderSystemItem &rItem);
        void ClearOrderSystemCD(HelloKittyMsgData::OrderSystemItem *pItem);
        HelloKittyMsgData::OrderSystemItem *getOrderByID(DWORD colidID);
        void Initcolid(DWORD colidID);
        void Runcolid(HelloKittyMsgData::OrderSystemItem *pItem,DWORD ItemId,DWORD timer);
        bool fullMessage(HelloKittyMsgData::AckReconnectInfo &binary);
    private:
        SceneUser& m_rUser;
        std::map<DWORD,HelloKittyMsgData::OrderSystemItem> m_mapOrder;
};
#endif// ORDERSYSTEMMANAGER_MANAGER_H__
