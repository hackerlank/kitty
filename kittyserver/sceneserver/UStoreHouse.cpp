#include "UStoreHouse.h"
#include "tbx.h"
#include "SceneUser.h"
#include "TradeCommand.h"
#include "extractProtoMsg.h"
#include "warehouse.pb.h"
#include "atlasManager.h"
#include "itemFunRegister.h"
#include "Misc.h"
#include "key.h"
#include "TimeTick.h"

std::map<DWORD,UStoreHouse::ITEM_FUN> UStoreHouse::s_itemFumMap;
bool UStoreHouse::s_initItemFunFlg = false;

UStoreHouse::UStoreHouse(SceneUser* u)
{
    reset();
	m_owner = u;
    initItemFun();
}

UStoreHouse::~UStoreHouse()
{

}

void UStoreHouse::reset()
{
    m_attrMap.clear();
    m_itemMap.clear();
}

bool UStoreHouse::init(const std::map<DWORD,DWORD> &itemMap)
{
    addOrConsumeItem(HelloKittyMsgData::Attr_Ware_Grid_Val,ParamManager::getMe().GetSingleParam(eParam_Init_Ware_House_Grid),"新建角色",true);
    addOrConsumeItem(itemMap,"新建角色",true);
    if(!getAttr(HelloKittyMsgData::Attr_Ware_Level))
    {
        addOrConsumeItem(HelloKittyMsgData::Attr_Ware_Level,1,"新建角色",true);
    }
    DWORD level = getAttr(HelloKittyMsgData::Attr_Ware_Level);
    const pb::Conf_t_WareHouseGrid *gridConf = tbx::WareHouseGrid().get_base(level);
    DWORD nowGrid = getAttr(HelloKittyMsgData::Attr_Ware_Grid_Val);
    if(gridConf->gridInfo->grid() > nowGrid)
    {
        addOrConsumeItem(HelloKittyMsgData::Attr_Ware_Grid_Val,gridConf->gridInfo->grid() - nowGrid,"新建角色",true);
    }
    else
    {
        addOrConsumeItem(HelloKittyMsgData::Attr_Ware_Grid_Val,nowGrid - gridConf->gridInfo->grid(),"新建角色",false);
    }
    setAttrVal(HelloKittyMsgData::Attr_Level,m_owner->charbase.level);
    return true;
}


void UStoreHouse::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_attrMap.begin(); iter != m_attrMap.end(); iter++)
    {
        HelloKittyMsgData::PbStoreItem* item = binary.add_store_items();
        if(item)
        {
            item->set_itemid(iter->first);
            item->set_itemcount(iter->second);
        }
    }
	for(auto iter = m_itemMap.begin(); iter != m_itemMap.end(); iter++)
    {
        HelloKittyMsgData::PbStoreItem* item = binary.add_store_items();
        *item = iter->second;
    }
}

void UStoreHouse::load(const HelloKittyMsgData::Serialize& binary)
{
    reset();
    for(int index = 0; index < binary.store_items_size(); index++)
    {
        const HelloKittyMsgData::PbStoreItem &item = binary.store_items(index);
        if(item.itemid() > HelloKittyMsgData::Attr_Max)
        {
            m_itemMap.insert(std::make_pair(item.itemid(),item));
        }
        else
        {
            if(item.itemid() == HelloKittyMsgData::Attr_Worker)
            {
                if(item.itemcount() > 6)
                {
                    m_attrMap.insert(std::pair<DWORD,DWORD>(item.itemid(),6));
                    continue;
                }
            }
            int val = item.itemcount();
            if(val < 0)
            {
                m_attrMap.insert(std::pair<DWORD,DWORD>(item.itemid(),0));
            }
            else
            {
                m_attrMap.insert(std::pair<DWORD,DWORD>(item.itemid(),item.itemcount()));
            }
        }
    }
}

bool UStoreHouse::flushWareHouse()
{
    HelloKittyMsgData::AckFlushWareHouse message;
    for (auto iter = m_itemMap.begin(); iter != m_itemMap.end(); ++iter)
    {
        HelloKittyMsgData::PbStoreItem *item = message.add_store_items();
        *item = iter->second; 
    }
    std::string ret;
    encodeMessage(&message,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool UStoreHouse::flushItem(const DWORD itemID)
{   
    HelloKittyMsgData::AckUpdateItem updateItem;
    HelloKittyMsgData::PbStoreItem* item = updateItem.mutable_item();
    item->set_itemid(itemID);
    auto iter = m_itemMap.find(itemID);
	if(iter == m_itemMap.end())
	{
        item->set_itemcount(0);
	}
    else
    {
        item->set_itemcount(iter->second.itemcount());
    }
    
    std::string ret;
    encodeMessage(&updateItem,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool UStoreHouse::addOrConsumeItem(const std::map<DWORD,DWORD> itemMap,const char *reMark,const bool opAdd,const bool judgeFull,const bool logFlg)
{
    if(judgeFull && opAdd)
    {
        //先检测一遍
        for(auto iter = itemMap.begin();iter != itemMap.end();++iter)
        {
            bool ret = hasEnoughSpace(iter->first,iter->second);
            if(!ret)
            {
                return false;
            }
        }
    }

    for(auto iter = itemMap.begin();iter != itemMap.end();++iter)
    {
        bool ret = addOrConsumeItem(iter->first,iter->second,reMark,opAdd,judgeFull,logFlg);
        if(!ret)
        {
            return false;
        }
    }
    return true;
}

bool UStoreHouse::addOrConsumeItem(const DWORD itemID,const DWORD num,const char *reMark,const bool opAdd,const bool judgeFull,const bool logFlg)
{
    //建筑
    if(itemID > Build_Base_ID && itemID < Item_Base_ID)
    {
        QWORD key = hashKey(itemID,1);
        const pb::Conf_t_building *tempConf = tbx::building().get_base(key);
        if(!tempConf)
        {
            Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__,itemID,num);
            return false;
        }
        for(int cnt = 0;DWORD(cnt) < num;++cnt)
        {
            m_owner->m_buildManager.giveBuildInWare(itemID,1);
        }
        return true;
    }
    const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(itemID);
	if(NULL == confBase)
    {
        Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__,itemID,num);
		return false;
    }
    bool ret = false;
    DWORD oldNum = 0,newNum = num;
    const std::map<DWORD,DWORD>&atlasMap = confBase->getAtlasMap();
    if(itemID < HelloKittyMsgData::Attr_Max)
    {
        oldNum = m_attrMap.find(itemID) == m_attrMap.end() ? 0 : m_attrMap[itemID];
        ret = opAttrVal(HelloKittyMsgData::AttrType(itemID),num,opAdd);
        newNum = m_attrMap.find(itemID) == m_attrMap.end() ? 0 : m_attrMap[itemID];
        if(ret)
        {
            m_owner->updateAttrVal(HelloKittyMsgData::AttrType(itemID),m_attrMap[itemID]);
        }
    }
    else if(confBase->itemInfo->kind() == Item_Type_Dress)
    {
        if(opAdd)
        {
            ret = m_owner->m_dressManager.addDressByItem(itemID);
        }
    }
    else if(confBase->itemInfo->kind() == Item_Type_Paper)
    {
        if(opAdd)
        {
            ret = m_owner->m_paperManager.addPaperByItem(itemID);
        }
    }
    //赠送兑换物品
    else if(confBase->itemInfo->kind() == Item_Type_Exchange)
    {
        if(opAdd)
        {
            //填充
            HelloKittyMsgData::GiftInfo giftInfo;
            giftInfo.set_type(itemID);
            giftInfo.set_status(HelloKittyMsgData::GS_InWare);
            giftInfo.set_endtime(0);
            giftInfo.set_num(num);
            m_owner->m_giftPackage.addGift(giftInfo);
        }
    }
    //赠送实物
    else if(confBase->itemInfo->kind() == Item_Type_Forbid)
    {
        if(opAdd)
        {
            HelloKittyMsgData::GiftCashInfo giftInfo;
            HelloKittyMsgData::GiftInfo *giftTemp = giftInfo.mutable_gift();
            if(giftTemp)
            {
                giftTemp->set_type(itemID);
                giftTemp->set_num(1);
                giftTemp->set_status(HelloKittyMsgData::GS_InWare);
                giftTemp->set_endtime(confBase->itemInfo->time() * 24 * 3600 + SceneTimeTick::currentTime.sec());
            }
            HelloKittyMsgData::GiftOrder *order = giftInfo.mutable_order();
            if(order)
            {
                order->set_id(0);
                order->set_deliverycompany("");
                order->set_deliverynum("");
            }
            m_owner->m_giftPackage.addGiftCash(giftInfo);
        }
    }
    else
    {
        oldNum = m_itemMap.find(itemID) == m_itemMap.end() ? 0 : m_itemMap[itemID].itemcount();
        ret = addOrConsumeItem(itemID,num,opAdd,judgeFull);
        newNum = m_itemMap.find(itemID) == m_itemMap.end() ? 0 : m_itemMap[itemID].itemcount();
        if(ret)
        {
            flushItem(itemID);
        }
        //给图鉴
        if(!atlasMap.empty()) 
        {
            if(opAdd)
            {
                m_owner->m_atlasManager.addAtlasByItem(itemID);
            }
        }
    }

    bool isAttr = itemID < HelloKittyMsgData::Attr_Max ? true : false;

    std::string now = SceneTimeTick::currentTime.toString();
    if(ret && logFlg)
    {
        if(isAttr)
        {
            bool isMoney = false;
            if(itemID == HelloKittyMsgData::Attr_Gold || itemID == HelloKittyMsgData::Attr_Gem || itemID == HelloKittyMsgData::Attr_Coupons || itemID == HelloKittyMsgData::Attr_Token)
            {
                isMoney = true;
            }
            Fir::logger->info("[%s][t_currency][f_time=%s][f_acc_id=%s][f_char_id=%lu][f_type=%u][f_before_count=%u][f_after_count=%u][f_level=%u][f_source=%s]",now.c_str(),now.c_str(),m_owner->charbase.account,m_owner->charid,itemID,oldNum,newNum,m_owner->charbase.level,reMark);
        }
        else
        {
            Fir::logger->info("[%s][t_game_prop][f_time=%s][f_prop_type=%u][f_prop_id=%u][f_prop_name=%s][f_prop_mark=%s][f_prop_source=%s][f_prop_count=%u]",now.c_str(),now.c_str(),itemID,itemID,confBase->itemInfo->item().c_str(),opAdd ? "获取" : "消耗" ,reMark,num);
        }
        if(opAdd)
        {
            if(itemID == 3003 || itemID == 3004 || itemID == 3005)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Expand_Materials,num);
            }
            if(itemID == 3001 || itemID == 3002 || itemID == 3009)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Warehouse_Material,num);
            }
            if(itemID == 3006 || itemID == 3007 || itemID == 3008)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Activate_Material,num);
            }
            if(itemID == 3010 || itemID == 3011 || itemID == 3012 || itemID == 3013)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Star_Upgrade_aterial,num);
            }
            if(itemID >= 3014 && itemID <= 3099)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Pal_Materials,num);
            }
            if(itemID == HelloKittyMsgData::Attr_Gold)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Coin_Total,num);
            }
            if(itemID == HelloKittyMsgData::Attr_Gem)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Diamond_Total,num);
            }
            if(itemID == HelloKittyMsgData::Attr_Token)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Token_Total,num);
            }
            if(itemID == HelloKittyMsgData::Attr_Coupons)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Ticket_Total,num);
            }
            if(itemID == HelloKittyMsgData::Attr_Friend_Val)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Friendship_Point_Total,num);
            }
        }
        else
        {
            if(itemID == HelloKittyMsgData::Attr_Gold)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Coin_Consumption_Total,num);
            }
            if(itemID == HelloKittyMsgData::Attr_Gem)
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Diamond_Consumption_Total,num);
            }
        }
    }

    return ret;
}

bool UStoreHouse::checkAttr(const HelloKittyMsgData::AttrType &attrID,const DWORD value)
{
    if(m_attrMap.find(attrID) == m_attrMap.end() || m_attrMap[attrID] < value)
    {
        return false;
    }
    return true;
}

DWORD UStoreHouse::getAttr(const HelloKittyMsgData::AttrType &attrID)
{
    return m_attrMap.find(attrID) == m_attrMap.end() ? 0 : m_attrMap[attrID];
}

DWORD UStoreHouse::getNum(const DWORD itemID)
{
    if(itemID < HelloKittyMsgData::Attr_Max)
    {
        return getAttr(HelloKittyMsgData::AttrType(itemID));
    }
    return getItemNum(itemID);
}

void UStoreHouse::setAttrVal(const HelloKittyMsgData::AttrType &attrID,const DWORD value)
{
    DWORD oldVal = m_attrMap .find(attrID) == m_attrMap.end() ? 0 : m_attrMap[attrID];
    if(oldVal != value)
    {
        m_attrMap[attrID] = value;
        m_owner->updateAttrVal(attrID,value);
    }
}

bool UStoreHouse::opAttrVal(const HelloKittyMsgData::AttrType &attrID,const DWORD value,const bool opAdd)
{
    bool ret = true;
    DWORD oldExp = m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Exp);
    DWORD oldLevel = m_owner->charbase.level;
    if(opAdd)
    {
        DWORD oldVal = m_attrMap.find(attrID) == m_attrMap.end() ? 0 : m_attrMap[attrID];
        if(m_attrMap.find(attrID) == m_attrMap.end())
        {
            m_attrMap.insert(std::pair<DWORD,DWORD>(attrID,value));
        }
        else
        {
            m_attrMap[attrID] += value;
        }
        //矫正人气当前值不能大于最大值
        if(attrID == HelloKittyMsgData::Attr_Popular_Now)
        {
            /*
            if(getAttr(HelloKittyMsgData::Attr_Popular_Now) > getAttr(HelloKittyMsgData::Attr_Popular_Max))
            {
                m_attrMap[attrID] = m_attrMap[HelloKittyMsgData::Attr_Popular_Max];
            }
            */
            m_owner->charbin.set_accpopularnow(m_attrMap[attrID] - oldVal + m_owner->charbin.accpopularnow());
            TaskArgue arg(Target_Add_Source,Attr_Acculate_Popular_Now,Attr_Acculate_Popular_Now,m_owner->charbin.accpopularnow());
            m_owner->m_taskManager.target(arg);
        }
        targetTaskOrAchieve(attrID,m_attrMap[attrID]);
    }
    else
    {
        if(!checkAttr(attrID,value))
        {
            ret = false;
        }
        else
        {
            m_attrMap[attrID] -= value;
            targetTaskOrAchieveReduce(attrID,value);
        }
    }

    switch(attrID)
    {
        case HelloKittyMsgData::Attr_Exp:
            {
                m_owner->charbase.exp = m_attrMap[attrID];
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(m_owner->charbase.charid);
                if(handle)
                {
                    handle->setInt("rolebaseinfo",m_owner->charbase.charid,"exp",m_owner->charbase.exp);
                }
                if(!opAdd)
                {
                    m_owner->adjustLevel();
                }
                setAttrVal(HelloKittyMsgData::Attr_Level,m_owner->charbase.level);
                m_owner->infoLevel(oldExp,oldLevel);
            }
            break;
        case HelloKittyMsgData::Attr_Charisma:
            {
                DWORD val = m_attrMap[attrID];
                m_owner->charbin.set_charismaweek(m_owner->charbin.charismaweek() + value);
                m_owner->charbin.set_charismamonth(m_owner->charbin.charismamonth() + value);
                m_owner->charbin.set_charis(val);
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(m_owner->charbase.charid);
                if(handle)
                {
                    handle->setInt("rolebaseinfo",m_owner->charid,"charisma",val);
                    handle->setInt("rolebaseinfo",m_owner->charid,"charismaweek",m_owner->charbin.charismaweek());
                    handle->setInt("rolebaseinfo",m_owner->charid,"charismamonth",m_owner->charbin.charismamonth());
                }
                handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Charisma);
                if(handle)
                {
                    handle->setSortSet("charismarank",m_owner->charid,"charisma",val);
                    handle->setSortSet("charismarank",m_owner->charid,"week",m_owner->charbin.charismaweek());
                    handle->setSortSet("charismarank",m_owner->charid,"month",m_owner->charbin.charismamonth());
                }
            }
            break;
        case HelloKittyMsgData::Attr_Verify_Level:
            {
                DWORD val = m_attrMap[attrID];
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(m_owner->charbase.charid);
                if(handle)
                {
                    handle->setInt("rolebaseinfo",m_owner->charbase.charid,"verifylevel",val);
                }
                bool isMan = m_owner->charbase.sex == HelloKittyMsgData::Male ? true : false;
                handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_VerifyLevel);
                if(handle)
                {
                    handle->setSortSet("verifyrank",m_owner->charid,"verifylevel",val);
                    handle->setSortSet("verifyrank",m_owner->charid,isMan ? "man" : "female",val);
                }
            }
            break;
        case HelloKittyMsgData::Attr_Coupons:
            {
                if(opAdd)
                {
                    m_owner->charbin.set_accucoupons(m_owner->charbin.accucoupons() + value);
                }
                m_owner->charbase.coupons = m_attrMap[attrID];
            }
        case HelloKittyMsgData::Attr_Contribute:
            {
                if(opAdd)
                {
                    m_owner->charbin.set_contribute(m_owner->charbin.contribute() + value);
                    m_owner->charbin.set_contributeweek(m_owner->charbin.contributeweek() + value);
                    m_owner->charbin.set_contributemonth(m_owner->charbin.contributemonth() + value);
                    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(m_owner->charbase.charid);
                    if(handle)
                    {
                        handle->setInt("rolebaseinfo",m_owner->charid,"contribute",m_owner->charbin.contribute());
                        handle->setInt("rolebaseinfo", m_owner->charid, "contributeweek", m_owner->charbin.contributeweek());
                        handle->setInt("rolebaseinfo", m_owner->charid, "contributemonth",m_owner->charbin.contributemonth());
                    }
                    handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Contribute);
                    if(handle)
                    {
                        handle->setSortSet("contributerank",m_owner->charid,"contribute",m_owner->charbin.contribute());
                        handle->setSortSet("contributerank",m_owner->charid,"contributeweek",m_owner->charbin.contributeweek());
                        handle->setSortSet("contributerank",m_owner->charid,"contributemonth",m_owner->charbin.contributemonth());
                    }
                }
            }
            break;
        case HelloKittyMsgData::Attr_Popular_Now:
            {
                if(opAdd)
                {
                    m_owner->charbin.set_popularnowweek(m_owner->charbin.popularnowweek() + value);
                    m_owner->charbin.set_popularnowmonth(m_owner->charbin.popularnowmonth() + value);
                    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(m_owner->charbase.charid);
                    if(handle)
                    {
                        handle->setInt("rolebaseinfo",m_owner->charid,"popularnow",m_attrMap[attrID]);
                        handle->setInt("rolebaseinfo", m_owner->charid, "popularnowweek", m_owner->charbin.popularnowweek());
                        handle->setInt("rolebaseinfo", m_owner->charid, "popularnowmonth",m_owner->charbin.popularnowmonth());
                    }
                    handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Popular_Now);
                    if(handle)
                    {
                        handle->setSortSet("popularnowrank",m_owner->charid,"popularnow",m_attrMap[attrID]);
                        handle->setSortSet("popularnowrank",m_owner->charid,"week",m_owner->charbin.popularnowweek());
                        handle->setSortSet("popularnowrank",m_owner->charid,"month",m_owner->charbin.popularnowmonth());
                    }
                }
                m_owner->charbin.set_popularnow(m_attrMap[attrID]);
            }
            break;
        case HelloKittyMsgData::Attr_Popular_Max:
            {
                m_owner->charbin.set_popularmax(m_attrMap[attrID]);
            }
            break;
        case HelloKittyMsgData::Attr_Gem:
            {
                m_owner->charbase.gem = m_attrMap[attrID];
            }
            break;
        case HelloKittyMsgData::Attr_Token:
            {
                m_owner->charbase.token = m_attrMap[attrID];
            }
        case HelloKittyMsgData::Attr_Ware_Level:
            {
                m_owner->adjusetGrid();
            }
            break;
        default:
            {
                break;
            }
    }
    return ret;
}

bool UStoreHouse::addOrConsumeItem(const DWORD itemID,const DWORD num,const bool opAdd,const bool judgeFull)
{
    bool ret = true;
	auto iter = m_itemMap.find(itemID);
    if(opAdd)
    {
        if(judgeFull && !hasEnoughSpace(itemID,num))
        {
            return false;
        }
        //建筑
        if(itemID > Build_Base_ID && itemID < Item_Base_ID)
        {
            for(int cnt = 0;DWORD(cnt) < num;++cnt)
            {
                m_owner->m_buildManager.giveBuildInWare(itemID,1);
            }
            return true;
        }
        if(iter == m_itemMap.end())
        {
            HelloKittyMsgData::PbStoreItem temp;
            temp.set_itemid(itemID);
            temp.set_itemcount(num);
            m_itemMap.insert(std::make_pair(itemID,temp));
        }
        else
        {
            iter->second.set_itemcount(iter->second.itemcount() + num);
        }
        /*
        //矫正人气当前值不能大于最大值
        if(itemID == HelloKittyMsgData::Attr_Popular_Now)
        {
            if(getAttr(HelloKittyMsgData::Attr_Popular_Now) > getAttr(HelloKittyMsgData::Attr_Popular_Max))
            {
                m_attrMap[itemID] = m_attrMap[HelloKittyMsgData::Attr_Popular_Max];
            }
        }
        */
        

    }
    else
    {
        //建筑
        if(itemID > Build_Base_ID && itemID < Item_Base_ID)
        {
            return false;
        }
        if(iter == m_itemMap.end() || iter->second.itemcount() < num)
        {
            ret = false;
        }
        else
        {
            iter->second.set_itemcount(iter->second.itemcount() - num);
        }
        if(!iter->second.itemcount())
        {
            m_itemMap.erase(iter);
        }
    }
    return ret;
}

DWORD UStoreHouse::getItemCount()
{
	DWORD itemcount = 0;
	for(auto iter = m_itemMap.begin(); iter != m_itemMap.end(); ++iter)
	{
        itemcount += iter->second.itemcount();
	}
	return itemcount;
}

bool UStoreHouse::hasEnoughSpace(const DWORD itemID,const DWORD itemcount)
{
    //建筑
    if(itemID > Build_Base_ID && itemID < Item_Base_ID)
    {
        QWORD key = hashKey(itemID,1);
        const pb::Conf_t_building *tempConf = tbx::building().get_base(key);
        if(!tempConf)
        {
            Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__,itemID,itemcount);
            return false;
        }
        return true;
    }
    const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(itemID);
	if(NULL == confBase)
    {
        Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__,itemID,itemcount);
		return false;
    }
    if(itemID <= HelloKittyMsgData::Attr_Max || capSpace(confBase->itemInfo->kind()))
    {
        return true;
    }
	DWORD store_limit = m_owner->getStoreLimit();
    DWORD cur_item_count = getItemCount();
    DWORD remain_space = store_limit > cur_item_count ? store_limit - cur_item_count : 0;
    return remain_space >= itemcount;
}

bool UStoreHouse::hasEnoughItem(const DWORD itemid, const DWORD itemcount)
{
    //不管建筑
    if(itemid > Build_Base_ID && itemid < Item_Base_ID)
    {
        return false;
    }
	auto iter = m_itemMap.find(itemid);
	if(iter == m_itemMap.end())
	{
		return false;
	}
    return iter->second.itemcount() >= itemcount;
}

DWORD UStoreHouse::getItemNum(const DWORD itemid)
{	
    auto iter = m_itemMap.find(itemid);
	if(iter == m_itemMap.end())
	{
		return 0;
	}
    return iter->second.itemcount();

}

bool UStoreHouse::capSpace(const DWORD itemType)
{
    if(itemType == Item_Type_Money || itemType == Item_Type_Dress || itemType == Item_Type_Paper)
    {
        return true;
    }
    return false;
}

bool UStoreHouse::hasEnoughSpace(const std::map<DWORD,DWORD>&itemMap)
{
    DWORD needSpace = 0;
    for(auto iter = itemMap.begin();iter != itemMap.end();++iter)
    {
        //建筑
        if(iter->first > Build_Base_ID && iter->first < Item_Base_ID)
        {
            QWORD key = hashKey(iter->first,1);
            const pb::Conf_t_building *tempConf = tbx::building().get_base(key);
            if(!tempConf)
            {
                Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__,iter->first,iter->second);
                return false;
            }
            continue;
        }
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(iter->first);
        if(NULL == confBase)
        {
            Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__,iter->first,iter->second);
            return false;
        }
        if(iter->first <= HelloKittyMsgData::Attr_Max || capSpace(confBase->itemInfo->kind()))
        {
            continue;
        }
        needSpace += iter->second;
    }
	DWORD store_limit = m_owner->getStoreLimit();
    DWORD cur_item_count = getItemCount();
    DWORD remain_space = store_limit > cur_item_count ? store_limit - cur_item_count : 0;
    return remain_space >= needSpace;
}

bool UStoreHouse::sallSystem(const DWORD itemid,const DWORD num)
{
    const pb::Conf_t_itemInfo* base = tbx::itemInfo().get_base(itemid);
    if(!base)
    {
        Fir::logger->debug("[出售道具错误] 道具不存在 %u",itemid);
        return false;
    }
    if(base->itemInfo->transaction() == ITT_Forbid_Recycle || base->itemInfo->transaction() == ITT_Forbid_Both)
    {
        return m_owner->opErrorReturn(HelloKittyMsgData::Item_Forbid_Recycle);
    }
    DWORD price = base->itemInfo->price() * num;
    //DWORD price = ParamManager::getMe().GetSingleParam(eParam_Item_System_Price) * num; 
    if(!addOrConsumeItem(itemid,num,"出售道具给系统",false))
    {
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"出售道具给系统(%u,%u)",itemid,num);
    return addOrConsumeItem(HelloKittyMsgData::Attr_Gold,price,temp,true);
}

bool UStoreHouse::targetTaskOrAchieve(const HelloKittyMsgData::AttrType &attrID,const DWORD value)
{
    HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
    if(!dailyData)
    {
        return false;
    }
    
    bool target = true,achieve = false;
    TaskArgue arg(Target_Add_Source),argDaily(Target_Add_Source);
    AchieveArg achieveArg;
    switch(attrID)
    {
        case HelloKittyMsgData::Attr_Gold:
            {
                arg.attrID = Attr_Money_Gold;
                arg.key = Attr_Money_Gold;
                arg.value = value; 

                argDaily.attrID = Attr_Money_Gold;
                argDaily.key = Attr_Add_Money_Gold;
                argDaily.value = dailyData->addgold();

                achieveArg.targerType = Achieve_Target_Have;
                achieveArg.subType = Achieve_Sub_Sorce_Num;
                achieveArg.key = Attr_Money_Gold;
                achieveArg.value = value;
                achieve = true;
            }
            break;
        case HelloKittyMsgData::Attr_Gem:
            {
                arg.attrID = Attr_Money_Gem;
                arg.key = Attr_Money_Gem;
                arg.value = value;

                argDaily.attrID = Attr_Add_Money_Gem;
                argDaily.key = Attr_Add_Money_Gem;
                argDaily.value = dailyData->addgem();
            }
            break;
        case HelloKittyMsgData::Attr_Exp:
            {
                arg.attrID = Attr_Visitor;
                arg.key = Attr_Visitor;
                arg.value = value;

                argDaily.attrID = Attr_Visitor;
                argDaily.key = Attr_Visitor;
                argDaily.value = dailyData->addexp();
                m_owner->upgrade();
            }
            break;
        case HelloKittyMsgData::Attr_Happy_Val:
            {
                arg.attrID = Attr_Happy_val;
                arg.key = Attr_Happy_val;
                arg.value = value;
                
                argDaily.attrID = Attr_Happy_val;
                argDaily.key = Attr_Add_Happy_Val;
                argDaily.key = dailyData->addhappy();
                m_owner->changeHappyData();
            }
            break;
        case HelloKittyMsgData::Attr_Popular_Now:
            {
                arg.attrID = Attr_Popular_Now;
                arg.key = Attr_Popular_Now;
                arg.value = value;

                achieveArg.targerType = Achieve_Target_Have;
                achieveArg.subType = Achieve_Sub_Sorce_Num;
                achieveArg.key = Attr_Popular_Now;
                achieveArg.value = value;
                achieve = true;
            }
            break;
        case HelloKittyMsgData::Attr_Popular_Max:
            {
                arg.attrID = Attr_Popular_Max;
                arg.key = Attr_Popular_Max;
                arg.value = value;
            }
            break;


        case HelloKittyMsgData::Attr_Friend_Val:
            {
                target = false;
            }
            break;
        case HelloKittyMsgData::Attr_Carnival_Val:
            {
                target = false;
            }
            break;
        case HelloKittyMsgData::Attr_Sweet_Val:
            {
                target = false;
            }
            break;
        case HelloKittyMsgData::Attr_Sall_Grid_Val:
            {
                target = false;
            }
        default:
            {
                target = false;
                break;
            }
    }
    if(target)
    {
        m_owner->m_taskManager.target(arg);
        m_owner->m_taskManager.target(argDaily);
    }
    if(achieve)
    {
         m_owner->m_achievementManager.target(achieveArg);
    }
        
    return true;
}

bool UStoreHouse::targetTaskOrAchieveReduce(const HelloKittyMsgData::AttrType &attrID,const DWORD value)
{
    HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
    if(!dailyData)
    {
        return false;
    }
    
    bool target = true,achieve = false;
    TaskArgue arg(Target_Add_Source),argDaily(Target_Add_Source);
    AchieveArg achieveArg;
    switch(attrID)
    {
        case HelloKittyMsgData::Attr_Gem:
            {
                dailyData->set_costgem(dailyData->costgem() + value);
                arg.attrID = Attr_Cost_Gem;
                arg.key = Attr_Cost_Gem;
                arg.value = dailyData->costgem();

                argDaily.attrID = Attr_Cost_Gem;
                argDaily.key = Attr_Cost_Gem;
                argDaily.value = dailyData->costgem();
            }
            break;
        default:
            {
                target = false;
                break;
            }
    }
    if(target)
    {
        m_owner->m_taskManager.target(arg);
        m_owner->m_taskManager.target(argDaily);
    }
    if(achieve)
    {
         m_owner->m_achievementManager.target(achieveArg);
    }
        
    return true;
}




