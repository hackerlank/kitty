#include "MarketManager.h"
#include "tbx.h"
#include "zMisc.h"
#include "TimeTick.h"
#include "SceneUser.h"
#include "Misc.h"
#include "zMemDBPool.h"
#include "RedisMgr.h"
#include "SceneMail.h"



marketManager::marketManager(SceneUser& rUser):m_rUser(rUser)
{

}

marketManager::~marketManager()
{

}


bool marketManager::timerCheck()
{
    DWORD nowTimer = SceneTimeTick::currentTime.sec();
    bool bUpdateservant = false;
    switch(m_servant.state())
    {
        case HelloKittyMsgData::ServantState_Active:
            {
                if(nowTimer > m_servant.servanttime())
                {
                    setservant(HelloKittyMsgData::ServantState_None);
                    bUpdateservant = true;
                }
            }
            break;
        case HelloKittyMsgData::ServantState_FindCoolDwown:
            {
                if(nowTimer > m_servant.servanttime())
                {
                    setservant(HelloKittyMsgData::ServantState_None);
                    bUpdateservant = true;
                }
                else if(nowTimer > m_servant.cooltime())
                {
                    setservant(HelloKittyMsgData::ServantState_Active);
                    bUpdateservant = true;
                }
            }
            break;
        case HelloKittyMsgData::ServantState_AutoCoolDwown:
            {
                if(nowTimer > m_servant.cooltime())
                {
                    setservant(HelloKittyMsgData::ServantState_AutoOK); 
                    bUpdateservant = true;
                }
            }
            break;
        default:
            break;

    }
    if(bUpdateservant)
        UpdateStateServant();
    DWORD cooldwonhour = ParamManager::getMe().GetSingleParam(eParam_Market_FlushTime);
    if(cooldwonhour == 0)
    {
        cooldwonhour = 4;
    }

    if(m_market.lastflushtimer() > 0 &&  nowTimer > cooldwonhour*3600 + m_market.lastflushtimer() )
    {
        initmarket();

    }
#if 0
    struct tm tv;
    zRTime::getLocalTime(tv,nowTimer);
    bool bupdateMarket =false;
    std::vector<DWORD> rClock = ParamManager::getMe().GetVecParam(eParam_Market_FlushTime);
    for(auto it = rClock.begin();it != rClock.end();it++)
    {
        DWORD hclock = *it;
        if(DWORD(tv.tm_hour) == hclock)
        {
            bupdateMarket = true;
            break;
        }
    }
    if(!bupdateMarket)
    {
        return false;
    }
    initmarket(true,true);
#endif

    return true;
}

void marketManager::init()
{
    m_market.set_lastflushtimer(0);
    //initmarket(true,false);
    setservant(HelloKittyMsgData::ServantState_None);

}
bool marketManager::maketisopen()
{
    return m_market.lastflushtimer() > 0;
}


void marketManager::load(const HelloKittyMsgData::Serialize& binary)
{
    if(binary.maketdata_size() != 0)
    {
        m_market = binary.maketdata(0);
    }
    if(binary.servant_size() != 0)
    {
        m_servant = binary.servant(0);
    }

}

void marketManager::save(HelloKittyMsgData::Serialize& binary)
{
    HelloKittyMsgData::MarketData *pmarket = binary.add_maketdata();
    if(pmarket)
    {
        *pmarket = m_market;
    }
    HelloKittyMsgData::ServantData *pservant = binary.add_servant();
    if(pservant)
    {
        *pservant = m_servant;
    }

}

void marketManager::initmarket()
{
    m_market.set_lastflushtimer(SceneTimeTick::currentTime.sec());
    m_market.clear_submaketitem();
    const std::unordered_map<unsigned int, const pb::Conf_t_itemInfo *> &tbxMap = tbx::itemInfo().getTbxMap();
    std::vector<std::vector<DWORD> > sel;
    std::vector<DWORD> sub;
    for(int i =0 ; i != 4 ;i++)
    {
        sel.push_back(sub);
    }
    for(auto it = tbxMap.begin();it != tbxMap.end();it++)
    {
        const pb::Conf_t_itemInfo *pitemInfo = it->second;
        if(pitemInfo == NULL)
        {
            continue;
        }
        switch(pitemInfo->itemInfo->reward())
        {
            case 1:
                {
                    if(pitemInfo->itemInfo->level() > m_rUser.charbase.level)
                    {
                        continue;
                    }
                    if(pitemInfo->itemInfo->itemlevel() ==  1 )
                    {
                        sel[0].push_back(it->first);
                    }
                    else if(pitemInfo->itemInfo->itemlevel()  > 1)
                    {
                        sel[1].push_back(it->first);
                    }


                }
                break;
            case 2:
                {
                    sel[2].push_back(it->first);  
                }
                break;
            case 3:
                {
                    sel[3].push_back(it->first); 
                }
                break;
            default:
                break;
        }
    } 
    //第一列
    for(int i =0 ; i != 4; i++)
    {
        const pb::Conf_t_market *confmarket = tbx::market().get_base(i+1);
        if(confmarket)
        {
            std::vector<DWORD> &rvec = sel[i];
            for(DWORD j =0 ; j != confmarket->market->colnum() && !rvec.empty() ; j++)
            {
                HelloKittyMsgData::MarketItem *pItem = m_market.add_submaketitem();
                if(pItem == NULL)
                    continue;
                pItem->set_lineid(i+1);
                pItem->set_rowid(j+1);
                pItem->set_state(HelloKittyMsgData::MarketItem_None);
                DWORD randIndex = zMisc::randBetween(0,rvec.size() -1 );
                pItem->set_itemid(rvec[randIndex]);
                rvec.erase(rvec.begin() + randIndex);
                pItem->set_itemnum(confmarket->getnum());
                pItem->set_moneytype(static_cast<HelloKittyMsgData::AttrType>(confmarket->market->moneytype()));
                pItem->set_off(confmarket->getpricerate());
                DWORD money = 1;
                if(pItem->moneytype() == HelloKittyMsgData::Attr_Gold)
                {

                    const pb::Conf_t_OrderGoods *pGoods = NULL;
                    const std::unordered_map<unsigned int, const pb::Conf_t_OrderGoods *> &ordertbxMap = tbx::OrderGoods().getTbxMap();
                    for(auto subit = ordertbxMap.begin();subit != ordertbxMap.end();subit++)
                    {
                        if(subit->second->OrderGoods->itemid() == pItem->itemid())
                        {
                            pGoods = subit->second;
                            break;
                        }
                    }
                    if(pGoods != NULL)
                    {
                        money = pGoods->OrderGoods->gold();
                    }
                    else
                    {
                        const pb::Conf_t_itemInfo *pitemconf = tbx::itemInfo().get_base(pItem->itemid());
                        if(pitemconf)
                        {
                            money = pitemconf->itemInfo->money();
                        }
                    }

                }
                else if(pItem->moneytype() == HelloKittyMsgData::Attr_Gem)
                {
                    const pb::Conf_t_itemInfo *pitemconf = tbx::itemInfo().get_base(pItem->itemid());
                    if(pitemconf)
                    {
                        money = pitemconf->itemInfo->shopprice();
                    }
                }
                money = ceil(money*pItem->off()*pItem->itemnum()/float(100));
                money = money > 0 ? money : 1;
                pItem->set_moneynum(money);

            }
        }
    }
    UpdateMarket(NULL);


}

void marketManager::setservant(HelloKittyMsgData::ServantState state)
{
    m_servant.set_state(state);
    switch(state)
    {
        case HelloKittyMsgData::ServantState_None:
            {
                m_servant.set_servanttime(0);
                m_servant.set_cooltime(0);
                m_servant.set_servantid(0);
                m_servant.set_itemid(0);
                m_servant.clear_openbox();
                m_servant.set_classid(0);
                m_servant.clear_boxid();


            }
            break;
        case HelloKittyMsgData::ServantState_Active:
            {
                m_servant.set_cooltime(0);
                m_servant.set_servantid(0);
                m_servant.set_itemid(0);
                m_servant.clear_openbox();
                m_servant.clear_boxid();

            }
            break;
        case HelloKittyMsgData::ServantState_AutoOK: 
            {
                m_servant.set_cooltime(0); 
            }
            break;
        default:
            break;
    }

}

HelloKittyMsgData::MarketData & marketManager::getMaketData()
{
    return m_market;
}

HelloKittyMsgData::ServantData& marketManager::getservant()
{
    return m_servant;
}

void marketManager::UpdateMarket(const HelloKittyMsgData::MarketItem* pSel)
{
    if(pSel)
    {
        HelloKittyMsgData::AckUpdateMarketOne ack;
        HelloKittyMsgData::MarketItem* pupdate = ack.mutable_submaketitem();
        if(pupdate)
        {
            *pupdate = *pSel;
        }
        std::string ret;    
        encodeMessage(&ack,ret);    
        m_rUser.sendCmdToMe(ret.c_str(),ret.size());  
    }
    else
    {
        HelloKittyMsgData::AckUpdateMarketAll ack;
        HelloKittyMsgData::MarketData* pupdate = ack.mutable_maket();
        if(pupdate)
        {
            *pupdate = m_market;
        }

        std::string ret;
        encodeMessage(&ack,ret);
        m_rUser.sendCmdToMe(ret.c_str(),ret.size());

    }

}

void marketManager::UpdateStateServant()
{
    HelloKittyMsgData::AckUpdateServant ack;
    HelloKittyMsgData::ServantData *pservant = ack.mutable_servant();
    if(pservant == NULL)
        return ;
    SetStateServantForCli(pservant);
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());

}

void marketManager::SetStateServantForCli(HelloKittyMsgData::ServantData *pservant)
{
    *pservant = m_servant;

}
