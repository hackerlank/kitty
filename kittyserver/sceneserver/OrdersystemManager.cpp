#include "OrdersystemManager.h"
#include "tbx.h"
#include "zMisc.h"
#include "TimeTick.h"
#include "buildManager.h"
#include "SceneUser.h"
#include "Misc.h"
#include "zMemDBPool.h"
#include "RedisMgr.h"
#include "SceneMail.h"



ordersystemManager::ordersystemManager(SceneUser& rUser):m_rUser(rUser)
{

}

ordersystemManager::~ordersystemManager()
{

}
void ordersystemManager::UpdateOrderSystemItem(const HelloKittyMsgData::OrderSystemItem &rItem)
{
    HelloKittyMsgData::AckUpdateOrderSystem ack;
    HelloKittyMsgData::OrderSystemItem* pinfo = ack.mutable_subordersystem();
    if(pinfo)
    {
        *pinfo = rItem;

    }
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());
}


void ordersystemManager::timerCheck()
{
    //看是否有火车到站
    DWORD NowTimer = SceneTimeTick::currentTime.sec();
    for(auto it = m_mapOrder.begin();it != m_mapOrder.end();it++)
    {
        HelloKittyMsgData::OrderSystemItem &rItem = it->second;
        if(rItem.state() != HelloKittyMsgData::OrderSystemState_Running)
        {
            continue;
        }
        if(NowTimer < rItem.timer())
        {
            continue;

        }
        ClearOrderSystemCD(&rItem);
    }
}




void ordersystemManager::load(const HelloKittyMsgData::Serialize& binary)
{
    for (int i = 0; i < binary.subordersystem_size(); i++) {
        m_mapOrder[binary.subordersystem(i).colid()] = binary.subordersystem(i);
    }

}

void ordersystemManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto it = m_mapOrder.begin(); it != m_mapOrder.end() ;it++)
    {
        HelloKittyMsgData::OrderSystemItem *pOrder = binary.add_subordersystem();
        if(pOrder)
        {
            *pOrder = it->second;
        }
    }
}

bool ordersystemManager::fullMessage(HelloKittyMsgData::UserBaseInfo &binary)
{
    for(auto it = m_mapOrder.begin(); it != m_mapOrder.end() ;it++)
    {
        HelloKittyMsgData::OrderSystemItem *pOrder = binary.add_subordersystem();
        if(pOrder)
        {
            *pOrder = it->second;
        }
    }
    return true;
}

bool ordersystemManager::fullMessage(HelloKittyMsgData::AckReconnectInfo &binary)
{
    for(auto it = m_mapOrder.begin(); it != m_mapOrder.end() ;it++)
    {
        HelloKittyMsgData::OrderSystemItem *pOrder = binary.add_subordersystem();
        if(pOrder)
        {
            *pOrder = it->second;
        }
    }
    return true;
}



HelloKittyMsgData::OrderSystemItem* ordersystemManager::getOrderByID(DWORD colidID)
{
    auto iter = m_mapOrder.find(colidID);
    if(iter == m_mapOrder.end())
    {
        return NULL;
    }
    return &(iter->second);

}

void ordersystemManager::ClearOrderSystemCD(HelloKittyMsgData::OrderSystemItem *pItem)
{
    pItem->set_state(HelloKittyMsgData::OrderSystemState_Finish);
    pItem->set_timer(0);
    UpdateOrderSystemItem(*pItem);
}

void ordersystemManager::Initcolid(DWORD colidID)
{
    HelloKittyMsgData::OrderSystemItem& rItem =  m_mapOrder[colidID];
    rItem.set_colid(colidID);
    rItem.set_state(HelloKittyMsgData::OrderSystemState_Empty);
    rItem.set_itemid(0);
    rItem.set_timer(0);
    UpdateOrderSystemItem(rItem);
}

void ordersystemManager::Runcolid(HelloKittyMsgData::OrderSystemItem *pItem,DWORD ItemId,DWORD timer)
{
    pItem->set_state(HelloKittyMsgData::OrderSystemState_Running);
    pItem->set_itemid(ItemId);
    pItem->set_timer(SceneTimeTick::currentTime.sec()+timer);
    UpdateOrderSystemItem(*pItem);
}

