#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneToOtherManager.h"
#include "SceneCommand.h"
#include "tbx.h"

bool SceneUser::deductPrice(const CMD::SCENE::t_UserPurchasePrice *cmd)
{
    CMD::SCENE::t_UserPurchaseUnlockeItem unLockItem;
    unLockItem.reqcharid = cmd->reqcharid;
    unLockItem.ackcharid = cmd->ackcharid;
    strncpy(unLockItem.name,charbase.account,sizeof(unLockItem.name));
    unLockItem.cellID = cmd->cellID;
    unLockItem.deductFlg = true;
    if(!m_store_house.checkAttr(HelloKittyMsgData::Attr_Gold,cmd->price))
    {
        opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gold);
        unLockItem.deductFlg = false;
    }
    if(!m_store_house.hasEnoughSpace(cmd->item,cmd->num))
    {
        opErrorReturn(HelloKittyMsgData::WareHouse_Is_Full);
        unLockItem.deductFlg = false;
    }
    
    if(unLockItem.deductFlg)
    {
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"购买道具扣除(%lu,%u,%u)",cmd->ackcharid,cmd->price,cmd->num);
        m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,cmd->price,temp,false);

        HelloKittyMsgData::DailyData *dailyData = charbin.mutable_dailydata();
        dailyData->set_tradenum(dailyData->tradenum() + 1);
        TaskArgue arg(Target_Add_Source,Attr_Trade_Num,Attr_Trade_Num,dailyData->tradenum());
        m_taskManager.target(arg);

    }
    SceneUser* sceneUser = SceneUserManager::getMe().getUserByID(cmd->ackcharid);
    if(sceneUser)
    {
        sceneUser->m_trade.purchaseUnLockItem(&unLockItem);
    }
    else
    {
        SceneClientToOtherManager::getMe().SendMsgToOtherSceneCharID(cmd->ackcharid,&unLockItem,sizeof(unLockItem));
    }
    return true;
}

bool SceneUser::purchaseAddItem(const CMD::SCENE::t_UserPurchaseShiftItem *cmd)
{
    bool ret = true;
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"交易获得");
    std::vector<HelloKittyMsgData::ReplaceWord> argVec;
    if(!m_store_house.addOrConsumeItem(cmd->itemID,cmd->itemNum,temp,true))
    {
        std::map<DWORD,DWORD> itemMap;
        itemMap.insert(std::pair<DWORD,DWORD>(cmd->itemID,cmd->itemNum));
        const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_WareFull_ID);
        if(emailConf)
        {
            EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,itemMap);
        }
        ret = false;
    }
    m_trade.buyPaperItemSuccess(cmd->ackcharid,cmd->cellID);
    Fir::logger->debug("[购买道具] %s (%u,%u)",ret ? "包裹" : "邮件",cmd->itemID,cmd->itemNum);
    return ret;
}

bool SceneUser::purchaseItem(const HelloKittyMsgData::ReqPurchaseItem *cmd)
{
    std::map<DWORD,DWORD>itemInfoMap;
    DWORD price = 0;
    for(int cnt = 0;cnt < cmd->iteminfo_size();++cnt)
    {
        const HelloKittyMsgData::PbStoreItem &itemInfo = cmd->iteminfo(cnt);
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(itemInfo.itemid());
        if(!confBase || confBase->getKey() <= HelloKittyMsgData::Attr_Max)
        {
            return false;
        }
        if(itemInfoMap.find(itemInfo.itemid()) == itemInfoMap.end())
        {
            itemInfoMap.insert(std::pair<DWORD,DWORD>(itemInfo.itemid(),itemInfo.itemcount()));
        }
        else
        {
            itemInfoMap[itemInfo.itemid()] += itemInfo.itemcount();
        }
        price += confBase->itemInfo->shopprice() * itemInfo.itemcount();
    }
#if 0
    //不要检测好了
    if(!m_store_house.hasEnoughSpace(itemInfoMap))
    {
        return opErrorReturn(HelloKittyMsgData::WareHouse_Is_Full);
    }
#endif
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"批量购买商城道具(%lu)",itemInfoMap.size());
    if(!m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,price,temp,false))
    {
        return opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
    }
    bzero(temp,sizeof(temp));
    snprintf(temp,sizeof(temp),"批量商城购买");
    return m_store_house.addOrConsumeItem(itemInfoMap,temp,true,false);
}

