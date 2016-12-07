#include "RechargeCmdDispatcher.h"
#include "SceneUser.h"
#include "extractProtoMsg.h"
#include "tbxbase.h"
bool RechargeCmdHandle::ReqChangeMoney(SceneUser* u, const HelloKittyMsgData::ReqChangeMoney* cmd)
{
    HelloKittyMsgData::ReChargeResult result = HelloKittyMsgData::ReChargeResult_Suc;

    do{
        const pb::Conf_t_TopupMall* base = tbx::TopupMall().get_base(cmd->changeid());
        if(base == NULL)
        {
            result = HelloKittyMsgData::ReChargeResult_ErrId;
            break;
        }
        const pb::Conf_t_upgrade*  pUpdate = tbx::upgrade().get_base(u->charbase.level);
        if(pUpdate == NULL)
        {
            result = HelloKittyMsgData::ReChargeResult_ErrId;
            break;

        }
        if(!u->m_store_house.addOrConsumeItem(base->getPriceMap(),"兑换金币",false))
        {
            result = HelloKittyMsgData::ReChargeResult_LowSource;
            break;
        }
        while((DWORD)u->charbin.recharge_size() < cmd->changeid())
        {
            u->charbin.add_recharge(false);
        }
        std::map<DWORD,DWORD> tempMap;
        if(u->charbin.recharge(cmd->changeid()-1))
        {
            tempMap = base->getMap();
            for(auto iter = tempMap.begin();iter != tempMap.end();++iter)
            {
                iter->second = iter->second * pUpdate->upgrade->changegoldparam() * base->mall->proportion() / 100; 
            }
        }
        else
        {
            tempMap = base->getFirstMap();
        }
        u->m_store_house.addOrConsumeItem(tempMap,"兑换金币",true);

    }while(0);
    HelloKittyMsgData::AckChangeMoney  ack;
    ack.set_changeid(cmd->changeid());
    ack.set_result(result);
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool RechargeCmdHandle::ReqChangeMoneyList(SceneUser* u, const HelloKittyMsgData::ReqChangeMoneyList* cmd)
{
    const std::unordered_map<unsigned int, const pb::Conf_t_TopupMall*> &tbxMap = tbx::TopupMall().getTbxMap();
    HelloKittyMsgData::AckChangeMoneyList ack;
    for(auto it = tbxMap.begin();it != tbxMap.end();it++)
    {
        HelloKittyMsgData::ItemChangeMoneyList *pItem = ack.add_item();
        if(pItem == NULL)
            continue;
        const pb::Conf_t_TopupMall* base = it->second;
        pItem->set_changeid(base->getKey());
        pItem->set_name(base->mall->itemname());
        pItem->set_icon(base->mall->itemtable());
        HelloKittyMsgData::Key32Val32Pair *pair = pItem->mutable_price();
        if(pair)
        {
            const std::map<DWORD,DWORD> &priceMap = base->getPriceMap();
            for(auto iter = priceMap.begin();iter != priceMap.end();++iter)
            {
                pair->set_key(iter->first);
                pair->set_val(iter->second);
            }
        }
        pair = pItem->mutable_firstnum();
        if(pair)
        {
            const std::map<DWORD,DWORD> &tempMap = base->getFirstMap();
            for(auto iter = tempMap.begin();iter != tempMap.end();++iter)
            {
                pair->set_key(iter->first);
                pair->set_val(iter->second);
            }
        }
        pair = pItem->mutable_gainnum();
        if(pair)
        {
            const std::map<DWORD,DWORD> &tempMap = base->getMap();
            for(auto iter = tempMap.begin();iter != tempMap.end();++iter)
            {
                pair->set_key(iter->first);
                pair->set_val(iter->second);
            }
        }
        pair = pItem->mutable_givenum();
        if(pair)
        {
            const std::map<DWORD,DWORD> &tempMap = base->getRewardMap();
            for(auto iter = tempMap.begin();iter != tempMap.end();++iter)
            {
                pair->set_key(iter->first);
                pair->set_val(iter->second);
            }
        }
        pItem->set_proportion(base->mall->proportion());
        while((DWORD)u->charbin.recharge_size() < base->getKey())
        {
            u->charbin.add_recharge(false);
        }
        pItem->set_first(u->charbin.recharge(base->getKey()-1) ? false : true);
    }
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}
