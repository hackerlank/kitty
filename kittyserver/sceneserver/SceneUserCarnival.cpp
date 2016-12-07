#include "SceneUser.h"
#include "zMetaData.h"
#include <stdarg.h>
#include "SceneServer.h"
#include "zMetaData.h"
#include "TimeTick.h"
#include "SceneUserManager.h"
#include <zlib.h>
#include <bitset>
#include "RecordClient.h"
#include "LoginUserCommand.h"
#include "xmlconfig.h"
#include <limits>
#include "ResType.h"
#include "RedisMgr.h"
#include "json/json.h"
#include "login.pb.h"
#include "extractProtoMsg.h"
#include "dataManager.h"
#include "tbx.h"
#include "taskAttr.h"
#include "Misc.h"

bool SceneUser::checkOpenCarnival()
{
    const pb::Conf_t_CarnivalData *carnival = tbx::CarnivalData().get_base(1);
    return carnival && m_store_house.getAttr(HelloKittyMsgData::Attr_Carnival_Val) >= carnival->carnival->state1uplimit();
}

bool SceneUser::openCarnival()
{
    const pb::Conf_t_CarnivalData *carnival = tbx::CarnivalData().get_base(1);
    if(!carnival || !checkOpenCarnival())
    {
        return false;
    }
    DWORD num = m_store_house.getAttr(HelloKittyMsgData::Attr_Carnival_Val);
    m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Carnival_Val,num,"开启嘉年华",false);
    Fir::logger->debug("[开启嘉年华嘉数值清空]:%s,%lu,%u,%u,%u",charbase.nickname,charid,num,num,m_store_house.getAttr(HelloKittyMsgData::Attr_Carnival_Val));

    DWORD rewardMoney = carnival->carnival->rewardpoint() + num * carnival->carnival->rewardgold();
    m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,rewardMoney,"嘉年华奖励",true);
    randCarnivalBox(num == 200);

    charbin.set_carnivalnum(charbin.carnivalnum() + 1);
    TaskArgue arg(Target_Add_Source,Attr_Carnival_Num,Attr_Carnival_Num,charbin.carnivalnum());
    m_taskManager.target(arg);
    return true;
}

void SceneUser::initCarnivalShopData()
{
    m_carnivalShopData.Clear();
    m_carnivalShopData.set_begintime(0);
    m_carnivalShopData.set_item(0);
    m_carnivalShopData.set_price(0);
}

DWORD SceneUser::countPrice(const std::map<DWORD,DWORD> &materialMap)
{
    DWORD sumPrice = 0;
    for(auto iter = materialMap.begin();iter != materialMap.end();++iter)
    {
        const pb::Conf_t_itemInfo *itemInfo = tbx::itemInfo().get_base(iter->first);
        if(itemInfo)
        {
            sumPrice += itemInfo->itemInfo->shopprice() * iter->second;
        }
    }
    return sumPrice;
}


bool SceneUser::randCarnivalShop()
{
    const pb::Conf_t_CarnivalShop *carnivalShop = tbx::CarnivalShop().get_base(1);
    if(!carnivalShop || m_carnivalShopData.item())
    {
        return false;
    }
    
    DWORD uniqueID =  pb::Conf_t_ItemPool::randID(carnivalShop->carnivalShop->pooid());
    if(!uniqueID)
    {
        return false;
    }
    const pb::Conf_t_ItemPool *pool = tbx::ItemPool().get_base(uniqueID);
    if(!pool)
    {
        return false;
    }
    m_carnivalShopData.add_randitem(pool->pool->itemid());
    m_carnivalShopData.set_item(pool->pool->itemid());
    const pb::Conf_t_itemInfo *itemInfo = tbx::itemInfo().get_base(pool->pool->itemid());
    if(!itemInfo)
    {
        return false;
    }
    DWORD sumPrice = itemInfo->itemInfo->shopprice();
    sumPrice *= carnivalShop->carnivalShop->discount() * 1.0 / 100;
    sumPrice = sumPrice ? sumPrice : 1;
    m_carnivalShopData.set_price(sumPrice);

    HelloKittyMsgData::AckClickCarnicalBox ackMessage;
    ackMessage.set_begintime(m_carnivalShopData.begintime());
    ackMessage.set_price(m_carnivalShopData.price());
    ackMessage.set_good(m_carnivalShopData.good());
    for(int index = 0;index < m_carnivalShopData.randitem_size();++index)
    {
        ackMessage.add_randitem(m_carnivalShopData.randitem(index));
    }

    std::string ret; 
    encodeMessage(&ackMessage,ret);
    return sendCmdToMe(ret.c_str(),ret.size());
        
}
    
bool SceneUser::buyCarnivalBox()
{
    if(!m_carnivalShopData.item() || !m_carnivalShopData.price())
    {
        return false;
    }

    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"购买嘉年华商店物品(%u)",m_carnivalShopData.item());
    if(!m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,m_carnivalShopData.price(),temp,false))
    {
        return opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
    }
    std::vector<HelloKittyMsgData::ReplaceWord> argVec;
    if(!m_store_house.addOrConsumeItem(m_carnivalShopData.item(),1,"嘉年华购买",true))
    {
        std::map<DWORD,DWORD> itemMap;
        itemMap.insert(std::pair<DWORD,DWORD>(m_carnivalShopData.item(),1));
        const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_WareFull_ID);
        if(emailConf)
        {
            EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,itemMap);
        }
    }
    initCarnivalShopData();

    HelloKittyMsgData::AckUpdateCarnicalBox ackDisapper;
    ackDisapper.set_begintime(0);
    ackDisapper.set_disapper(true);
    std::string ret;
    encodeMessage(&ackDisapper,ret);
    sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

void SceneUser::onLineCarnivalBox()
{
    const pb::Conf_t_CarnivalShop *carnivalShop = tbx::CarnivalShop().get_base(1);
    if(!carnivalShop)
    {
        return;
    }
    DWORD now = SceneTimeTick::currentTime.sec();
    if(now - m_carnivalShopData.begintime() > carnivalShop->carnivalShop->lasttime())
    {
        return;
    }
    if(m_carnivalShopData.price())
    {
    
        HelloKittyMsgData::AckClickCarnicalBox ackMessage;
        ackMessage.set_begintime(m_carnivalShopData.begintime());
        ackMessage.set_price(m_carnivalShopData.price());
        ackMessage.set_good(m_carnivalShopData.good());
        for(int index = 0;index < m_carnivalShopData.randitem_size();++index)
        {
            ackMessage.add_randitem(m_carnivalShopData.randitem(index));
        }

        std::string ret; 
        encodeMessage(&ackMessage,ret);
        sendCmdToMe(ret.c_str(),ret.size());
    }
    else
    {
        HelloKittyMsgData::AckUpdateCarnicalBox ackBox;
        ackBox.set_disapper(false);
        ackBox.set_begintime(m_carnivalShopData.begintime());

        std::string ret;
        encodeMessage(&ackBox,ret);
        sendCmdToMe(ret.c_str(),ret.size());
    }
    return;
}

bool SceneUser::randCarnivalBox(const bool good)
{
    initCarnivalShopData();
    m_carnivalShopData.set_begintime(SceneTimeTick::currentTime.sec());
    m_carnivalShopData.set_good(good);
    HelloKittyMsgData::AckUpdateCarnicalBox ackBox;
    ackBox.set_disapper(false);
    ackBox.set_begintime(m_carnivalShopData.begintime());
    ackBox.set_good(good);

    std::string ret;
    encodeMessage(&ackBox,ret);
    return sendCmdToMe(ret.c_str(),ret.size());
}

void SceneUser::loopCarnivalBox()
{
    if(!m_carnivalShopData.begintime())
    {
        return;
    }
    const pb::Conf_t_CarnivalShop *carnivalShop = tbx::CarnivalShop().get_base(1);
    if(!carnivalShop)
    {
        return;
    }
    DWORD now = SceneTimeTick::currentTime.sec();
    if(now - m_carnivalShopData.begintime() >= carnivalShop->carnivalShop->lasttime())
    {
        HelloKittyMsgData::AckUpdateCarnicalBox ackDisapper;
        ackDisapper.set_begintime(0);
        ackDisapper.set_disapper(true);
        std::string ret;
        encodeMessage(&ackDisapper,ret);
        sendCmdToMe(ret.c_str(),ret.size());
        initCarnivalShopData();
    }
}

            




    




