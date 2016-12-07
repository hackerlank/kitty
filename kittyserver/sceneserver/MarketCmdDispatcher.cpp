#include "MarketCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "extractProtoMsg.h"
#include "tbx.h"  
#include "Misc.h"
#include "TimeTick.h"
#include "dataManager.h"

bool marketCmdHandle::ReqMarketAndServantInfo(SceneUser* User,const HelloKittyMsgData::ReqMarketAndServantInfo *message)
{
    HelloKittyMsgData::AckMarketAndServantInfo ack;

    HelloKittyMsgData::MarketData &rData = User->m_market.getMaketData();
    HelloKittyMsgData::MarketData *pmarket = ack.mutable_maket();
    if(pmarket)
    {
        *pmarket = rData;
    }
    HelloKittyMsgData::ServantData* pservant = ack.mutable_servant();
    User->m_market.SetStateServantForCli(pservant);
    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool marketCmdHandle::ReqBuyMarketItem(SceneUser* User,const HelloKittyMsgData::ReqBuyMarketItem *message)
{
    HelloKittyMsgData::AckBuyMarketItem ack;
    ack.set_rowid(message->rowid());
    ack.set_lineid(message->lineid());
    HelloKittyMsgData::MarketResult result = HelloKittyMsgData::MarketResult_Suc;
    do{
        HelloKittyMsgData::MarketData &rData = User->m_market.getMaketData();
        HelloKittyMsgData::MarketItem* pSel = NULL;
        for(int i = 0 ; i != rData.submaketitem_size();i++)
        {
            HelloKittyMsgData::MarketItem* pItem = rData.mutable_submaketitem(i);
            if(pItem->lineid() == message->lineid() && pItem->rowid() == message->rowid())
            {
                pSel = pItem;
                break;
            }
        }
        if(pSel == NULL)
        {
            result = HelloKittyMsgData::MarketResult_OtherErr;
            break;
        }
        //已售
        if(pSel->state() == HelloKittyMsgData::MarketItem_Buy)
        {
            result = HelloKittyMsgData::MarketResult_BuyDone;
            break;
        }
        //如果金币购买，判断包裹
        HelloKittyMsgData::vecAward rvecAward;
        HelloKittyMsgData::Award* pItem = rvecAward.add_award();
        if(pItem)
        {
            pItem->set_awardtype(pSel->itemid());
            pItem->set_awardval(pSel->itemnum());
        }
        if(pSel->moneytype() == HelloKittyMsgData::Attr_Gold)
        {
            if(!User->checkPush(rvecAward))
            {
                result = HelloKittyMsgData::MarketResult_Parketfull;
                break;
            }

        }
        //判断钱
        if(!User->m_store_house.addOrConsumeItem(pSel->moneytype(),pSel->moneynum(),"购买黑市商品",false))
        {
            result = HelloKittyMsgData::MarketResult_MoneyLimit;
            break;

        }
        //给物品
        User->pushItemWithoutCheck(rvecAward,"购买黑市商品");
        pSel->set_state(HelloKittyMsgData::MarketItem_Buy);
        User->m_market.UpdateMarket(pSel);

    }while(0);
    std::string ret;
    ack.set_result(result);
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());

    return true;
}

bool marketCmdHandle::ReqFlushMarket(SceneUser* User,const HelloKittyMsgData::ReqFlushMarket *message)
{
    HelloKittyMsgData::AckFlushMarket ack;
    HelloKittyMsgData::MarketResult result = HelloKittyMsgData::MarketResult_Suc;
    do{
        //里刷新时间不足1小时，不可刷新
        //11:00以后 或 23:00以后不可刷
#if 0
        DWORD nowTimer = SceneTimeTick::currentTime.sec();
        struct tm tv;
        zRTime::getLocalTime(tv,nowTimer);
        std::vector<DWORD> rClock = ParamManager::getMe().GetVecParam(eParam_Market_FlushTime);
        for(auto it = rClock.begin();it != rClock.end();it++)
        {
            DWORD hclock = *it;
            if(hclock == 0)
            {
                hclock = 23;
            }
            else
            {
                hclock--;
            }
            if(DWORD(tv.tm_hour) == hclock)
            {
                result = HelloKittyMsgData::MarketResult_FluseTimeClose;
                break;
            }
        }
        
#endif
        if(User->m_market.maketisopen() == false)
        {
            result = HelloKittyMsgData::MarketResult_NoOpenMarket;
            break;
        }
        if(!User->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,ParamManager::getMe().GetSingleParam(eParam_Market_FlushCost),"黑市刷新",false))
        {
            result = HelloKittyMsgData::MarketResult_MoneyLimit;
            break;
        }
        //刷新
        User->m_market.initmarket();
    }while(0);
    std::string ret;
    ack.set_result(result);
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool marketCmdHandle::ReqBuyServant(SceneUser* User,const HelloKittyMsgData::ReqBuyServant *message)
{
    HelloKittyMsgData::AckBuyServant ack;
    HelloKittyMsgData::MarketResult result = HelloKittyMsgData::MarketResult_Suc;
    do{
        //检查状态
        HelloKittyMsgData::ServantData& rdata = User->m_market.getservant();
        if(rdata.state() != HelloKittyMsgData::ServantState_None)
        {
            result = HelloKittyMsgData::MarketResult_ServantActive;
            break;
        }
        //检查眼镜
        const pb::Conf_t_Classes *confClasses = tbx::Classes().get_base(message->glassid());
        if(confClasses == NULL)
        {
            result = HelloKittyMsgData::MarketResult_ServantNoClass;
            break;
        }
        //检查钱
        if(!rdata.firstflg() && !User->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,confClasses->classes->cost(),"黑市刷新",false))
        {
            result = HelloKittyMsgData::MarketResult_MoneyLimit;
            break;
        }
        if(rdata.firstflg())
        {
            rdata.set_firstflg(false);
        }
        //开通
        rdata.set_servanttime(SceneTimeTick::currentTime.sec() + confClasses->classes->time()*24*3600);
        rdata.set_classid(message->glassid());
        User->m_market.setservant(HelloKittyMsgData::ServantState_Active);
        User->m_market.UpdateStateServant();
        User->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Employment_Number_All,1);
        User->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Employment_Consumption_All,confClasses->classes->cost());

    }while(0);
    std::string ret;
    ack.set_result(result);
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool marketCmdHandle::ReqBuyServantItem(SceneUser* User,const HelloKittyMsgData::ReqBuyServantItem *message)
{
    HelloKittyMsgData::AckBuyServantItem ack;
    ack.set_itemid(message->itemid());
    ack.set_servantid(message->servantid());
    HelloKittyMsgData::MarketResult result = HelloKittyMsgData::MarketResult_Suc;
    do{
        //检查状态
        HelloKittyMsgData::ServantData& rdata = User->m_market.getservant();
        if(rdata.state() != HelloKittyMsgData::ServantState_Active)
        {
            result = HelloKittyMsgData::MarketResult_ServantCool;
            break;
        }
        //检查男仆id
        const pb::Conf_t_Manservant *confManservant = tbx::Manservant().get_base(message->servantid());
        if(confManservant ==NULL)
        {
            result = HelloKittyMsgData::MarketResult_ServantIdOrItemErr;
            break;

        }
        const pb::Conf_t_itemInfo *pitemconf = tbx::itemInfo().get_base(message->itemid());
        if(pitemconf == NULL)
        {
            result =  HelloKittyMsgData::MarketResult_ServantIdOrItemErr;
            break;
        } 
        const pb::itemInfo::t_itemInfo *pitemInfo = pitemconf->itemInfo;
        switch(confManservant->manservant->classes())
        {

            case 1:
                {
                    if(pitemInfo->reward() != 1 ||  pitemInfo->itemlevel() <= 1)
                    {
                        result =  HelloKittyMsgData::MarketResult_ServantIdOrItemErr;
                        break;
                    }

                }
                break;
            case 2:
                {
                    if(pitemInfo->reward() != 2 ||  pitemInfo->itemlevel() != 0)
                    {
                        result =  HelloKittyMsgData::MarketResult_ServantIdOrItemErr;
                        break;
                    }

                }
                break;
            case 3:
                {
                    if(pitemInfo->reward() != 3 ||  pitemInfo->itemlevel() != 0)
                    {
                        result =  HelloKittyMsgData::MarketResult_ServantIdOrItemErr;
                        break;
                    }
                }
                break;
            default:
                result =  HelloKittyMsgData::MarketResult_ServantIdOrItemErr;
                break;

        }
        if(result != HelloKittyMsgData::MarketResult_Suc)
        {
            break;
        }
        DWORD dwNow =SceneTimeTick::currentTime.sec(); 
        //检查钱
        //若立即获得，检查金币
        if(confManservant->manservant->type() == 0)
        {
            //先检查包裹
            ////如果金币购买，判断包裹
            HelloKittyMsgData::vecAward rvecAward;
            HelloKittyMsgData::Award* pItem = rvecAward.add_award();
            if(pItem)
            {
                pItem->set_awardtype(message->itemid());
                pItem->set_awardval(confManservant->manservant->num());
            }
            if(!User->checkPush(rvecAward))
            {
                result = HelloKittyMsgData::MarketResult_Parketfull;
                break;
            }

            DWORD gold = confManservant->manservant->gold() * pitemInfo->price() * confManservant->manservant->num() / float(100); 
            //判断钱
            if(!User->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,gold,"购买男仆物品",false))
            {
                result = HelloKittyMsgData::MarketResult_MoneyLimit;
                break;

            }
            User->pushItemWithoutCheck(rvecAward,"购买男仆物品");
            User->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Find_Number_All,1);
            rdata.set_state(HelloKittyMsgData::ServantState_FindCoolDwown);

        }
        else
        {
            //可能取包裹时，已经过期
            if(rdata.servanttime() < dwNow )
            {
                result = HelloKittyMsgData::MarketResult_ServantTimerOut;
                break;
            }
            //判断钱
            if(!User->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,confManservant->manservant->gen(),"男仆挂机扣钻石",false))
            {
                result = HelloKittyMsgData::MarketResult_MoneyLimit;
                break;

            }
            User->m_market.setservant(HelloKittyMsgData::ServantState_AutoCoolDwown);
            User->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Hangup_Number_All,1);

        }
        rdata.set_cooltime(dwNow + confManservant->manservant->time()*60);
        rdata.set_servantid(message->servantid());
        rdata.set_itemid(message->itemid());
        DWORD BoxNum = confManservant->getBoxNum();
        for(DWORD i = 0 ; i != BoxNum ;i++)
        {
            rdata.add_openbox(false);
            DWORD boxID = pb::Conf_t_ManservantBox::randBox();
            rdata.add_boxid(boxID);
        }
        User->m_market.UpdateStateServant();
    }while(0);
    std::string ret;
    ack.set_result(result);
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool marketCmdHandle::ReqGetServantAutoItem(SceneUser* User,const HelloKittyMsgData::ReqGetServantAutoItem *message)
{
    HelloKittyMsgData::AckGetServantAutoItem ack;
    HelloKittyMsgData::MarketResult result = HelloKittyMsgData::MarketResult_Suc;
    do{
        //检查状态
        HelloKittyMsgData::ServantData& rdata = User->m_market.getservant();
        if(rdata.state() != HelloKittyMsgData::ServantState_AutoOK)
        {
            result = HelloKittyMsgData::MarketResult_ServantNoAutoOK;
            break;
        }
        //检查男仆id
        const pb::Conf_t_Manservant *confManservant = tbx::Manservant().get_base(rdata.servantid());
        if(confManservant ==NULL)
        {
            result = HelloKittyMsgData::MarketResult_ServantIdOrItemErr;
            break;

        }
        //先检查包裹
        ////如果金币购买，判断包裹
        HelloKittyMsgData::vecAward rvecAward;
        HelloKittyMsgData::Award* pItem = rvecAward.add_award();
        if(pItem)
        {
            pItem->set_awardtype(rdata.itemid());
            pItem->set_awardval(confManservant->manservant->num());
        }
        if(!User->checkPush(rvecAward))
        {
            result = HelloKittyMsgData::MarketResult_Parketfull;
            break;
        }

        const pb::Conf_t_itemInfo *pitemconf = tbx::itemInfo().get_base(rdata.itemid());
        if(pitemconf == NULL)
        {
            result =  HelloKittyMsgData::MarketResult_ServantIdOrItemErr;
            break;
        }
        const pb::itemInfo::t_itemInfo *pitemInfo = pitemconf->itemInfo;
        DWORD gold = confManservant->manservant->gold() * pitemInfo->price() * confManservant->manservant->num() / float(100); 
        //判断钱
        if(!User->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,gold,"购买男仆物品",false))
        {
            result = HelloKittyMsgData::MarketResult_MoneyLimit;
            break;

        }
        User->pushItemWithoutCheck(rvecAward,"购买男仆物品");
        //如果眼镜已经过期
        if(SceneTimeTick::currentTime.sec() > rdata.servanttime())
        {
            User->m_market.setservant(HelloKittyMsgData::ServantState_None);
        }
        else
        {
            User->m_market.setservant(HelloKittyMsgData::ServantState_Active);
        }
        User->m_market.UpdateStateServant();


    }while(0);
    std::string ret;
    ack.set_result(result);
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool marketCmdHandle::ReqOpenServantBox(SceneUser* User,const HelloKittyMsgData::ReqOpenServantBox *message)
{
    HelloKittyMsgData::AckOpenServantBox ack;
    HelloKittyMsgData::MarketResult result = HelloKittyMsgData::MarketResult_Suc;
    do{
        HelloKittyMsgData::vecAward *pAward = ack.mutable_award();
        if(pAward ==NULL)
        {
            result = HelloKittyMsgData::MarketResult_OtherErr;
            break;
        }
        //检查状态
        HelloKittyMsgData::ServantData& rdata = User->m_market.getservant();
        if(rdata.state() != HelloKittyMsgData::ServantState_FindCoolDwown && rdata.state() != HelloKittyMsgData::ServantState_AutoCoolDwown &&  rdata.state() != HelloKittyMsgData::ServantState_AutoOK)
        {
            result = HelloKittyMsgData::MarketResult_NoBox;
            break;
        }
        //检查男仆id
        const pb::Conf_t_Manservant *confManservant = tbx::Manservant().get_base(rdata.servantid());
        if(confManservant ==NULL)
        {
            result = HelloKittyMsgData::MarketResult_ServantIdOrItemErr;
            break;

        }
        //检查箱子数目
        if(message->index() >= QWORD(rdata.openbox_size()))
        {
            result = HelloKittyMsgData::MarketResult_NoBox;
            break;
        }
        if(rdata.openbox(message->index()))
        {
            result = HelloKittyMsgData::MarketResult_BoxHasOpen;
            break;
        }
#if 0
        //判断钱
        if(!User->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,ParamManager::getMe().GetSingleParam(eParam_SerVant_OpenBox),"开男仆宝箱",false))
        {
            result = HelloKittyMsgData::MarketResult_MoneyLimit;
            break;

        }
#endif
        HelloKittyMsgData::vecAward& rvecAward = *pAward;
        DWORD boxID = rdata.boxid(message->index()); 
        pb::Conf_t_ManservantBox *box = const_cast<pb::Conf_t_ManservantBox*>(tbx::ManservantBox().get_base(boxID));
        if(box)
        {
            //判断钱
            if(!User->m_store_house.addOrConsumeItem(box->getPrice(),"开男仆宝箱",false))
            {
                result = HelloKittyMsgData::MarketResult_MoneyLimit;
                break;

            }
            std::map<DWORD,DWORD> itemMap;
            if(!box->randItemMap(itemMap))
            {
                result = HelloKittyMsgData::MarketResult_NoBox;
                break;
            }
            for(auto iter = itemMap.begin();iter != itemMap.end();++iter)
            {
                HelloKittyMsgData::Award* pItem = rvecAward.add_award();
                if(pItem)
                {
                    pItem->set_awardtype(iter->first);
                    pItem->set_awardval(iter->second);
                }
            }
        }

#if 0
            const std::unordered_map<unsigned int, const pb::Conf_t_itemInfo *> &tbxMap = tbx::itemInfo().getTbxMap();
            std::vector<DWORD> sel;
            for(auto it = tbxMap.begin();it != tbxMap.end();it++)   
            {
                const pb::Conf_t_itemInfo *pitemInfo = it->second;
                if(pitemInfo == NULL)
                    continue;
                switch(confManservant->manservant->box())
                {
                    case 0:
                        {
                            if(pitemInfo->itemInfo->reward() == 1 ||pitemInfo->itemInfo->reward() == 2 || pitemInfo->itemInfo->reward() ==3)
                            {
                                sel.push_back(it->first);
                            }
                        }
                        break;
                    case 1:
                        {
                            if(pitemInfo->itemInfo->reward() == 1 )
                            {
                                sel.push_back(it->first);
                            }
                        }
                        break;
                    case 2:
                        {
                            if(pitemInfo->itemInfo->reward() == 2 )
                            {
                                sel.push_back(it->first);
                            }
                        }
                        break;
                    case 3:
                        {
                            if(pitemInfo->itemInfo->reward() == 3 )
                            {
                                sel.push_back(it->first);
                            }
                        }
                        break;
                    default:
                        break;

                }

            }
            if(sel.empty())
            {
                result = HelloKittyMsgData::MarketResult_OtherErr;
                break;
            }
            DWORD randIndex = zMisc::randBetween(0,sel.size() -1 );
            pItem->set_awardtype(sel[randIndex]);
            pItem->set_awardval(confManservant->getBoxOpenGetNum());

        }
        //判断钱
        if(!User->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,ParamManager::getMe().GetSingleParam(eParam_SerVant_OpenBox),"开男仆宝箱",false))
        {
            result = HelloKittyMsgData::MarketResult_MoneyLimit;
            break;

        }

#endif
        User->pushItemWithoutCheck(rvecAward,"开男仆宝箱");
        rdata.set_openbox(message->index(),true);
        User->m_market.UpdateStateServant();
    }while(0);
    std::string ret;
    ack.set_result(result);
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

