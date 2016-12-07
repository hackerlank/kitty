#include "PlayerActiveConfig.h"
#include "SceneServer.h"
#include "TimeTick.h"
#define ThreeDay 3*24*3600
void PlayerActiveConfig::timerCheck()
{
    std::map<DWORD,active_state> newMap ;
    DWORD DwNow  =  SceneTimeTick::currentTime.sec();
    for(auto it = m_mapState.begin();it != m_mapState.end(); it++)
    {
        switch(it->first)
        {
            case state_wait:
                {
                    for(auto iter = it->second.begin();iter != it->second.end();iter++)
                    {
                        const HelloKittyMsgData::PlayerActive * pinfo =  getinfo(*iter);
                        if(pinfo)
                        {
                            if(DwNow >= pinfo->f_begintime())
                            {
                                newMap[*iter] = state_run;
                            }
                        }
                        else
                        {
                            newMap[*iter] = state_del;
                        }

                    }
                };
                break;
            case state_run:
                {
                    for(auto iter = it->second.begin();iter != it->second.end();iter++)
                    {
                        const HelloKittyMsgData::PlayerActive * pinfo =  getinfo(*iter);
                        if(pinfo)
                        {
                            if(isNeedDel(DwNow, pinfo->f_endtime() ,pinfo->f_condition()))
                            {
                                newMap[*iter] = state_del;
                            }
#if 0
                            if(!isNeedDel(DwNow, pinfo->f_endtime() ,pinfo->f_condition()))
                            {
                                newMap[*iter] = state_finish;
                            }
                            else
                            {
                                newMap[*iter] = state_del;
                            }
#endif
                        }
                        else
                        {
                            newMap[*iter] = state_del;
                        }
                    }
                }
                break;
            case state_finish:
                {
                    for(auto iter = it->second.begin();iter != it->second.end();iter++)
                    {
                        const HelloKittyMsgData::PlayerActive * pinfo =  getinfo(*iter);
                        if(pinfo)
                        {
                            if(isNeedDel(DwNow, pinfo->f_endtime() ,pinfo->f_condition()))
                            {
                                newMap[*iter] = state_del;
                            }
                        }
                        else
                        {
                            newMap[*iter] = state_del;
                        }
                    }
                }
                break;
            default:
                break;

        }
    }
    for(auto it = newMap.begin() ;it != newMap.end();it++)
    {
        setState(it->first,it->second);
    }


}
const std::set<DWORD>* PlayerActiveConfig::getListen(HelloKittyMsgData::ActiveConditionType type) const
{
    auto it = m_maplisten.find(type);
    return it == m_maplisten.end() ? NULL : &(it->second);

}

active_state PlayerActiveConfig::getState(DWORD activeID)
{
    auto iter = m_mapID.find(activeID);
    return iter != m_mapID.end() ? state_None : iter->second; 
}

void PlayerActiveConfig::setState(DWORD activeID,active_state new_state)
{
    auto it = m_mapID.find(activeID);
    active_state oldstate = state_None;
    if(it != m_mapID.end())
    {
        oldstate = it->second;
    }
    if(oldstate == new_state)
        return ;

    const HelloKittyMsgData::PlayerActive * pinfo = getinfo(activeID);
    if(pinfo)
    {
        if(oldstate == state_run)
        {
            m_maplisten[pinfo->f_condition()].erase(activeID);
        }
        if(new_state == state_run)
        {
            m_maplisten[pinfo->f_condition()].insert(activeID);
        }
    }
    if(new_state != state_del)
    {
        if(oldstate == state_None)
        {
            m_mapID[activeID] = new_state;
            m_mapState[new_state].insert(activeID);
        }
        else
        {
            m_mapState[oldstate].erase(activeID);
            m_mapState[new_state].insert(activeID);
            m_mapID[activeID] = new_state;
        }
    }
    else
    {
        if(oldstate != state_None)
        {
            m_mapState[oldstate].erase(activeID);
            m_mapID.erase(activeID);
        }
        auto iter = m_mapActive.find(activeID);
        if(iter != m_mapActive.end())
        {
            const HelloKittyMsgData::PlayerActive &active = iter->second;
            if(active.f_rewardmaxcnt())
            {
                zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(activeID);
                if(redis)
                {

                    redis->delInt("limitactive",activeID,"rewardlimitmax");
                    redis->delInt("limitactive",activeID,"rewardlimitcur");
                }
            }
        }
        m_mapActive.erase(activeID);
    }
}

bool PlayerActiveConfig::isNeedDel(DWORD now,DWORD endtime,HelloKittyMsgData::ActiveConditionType type)
{
    if(endtime == 0)
        return false;
    if(isSaveType(type))
    {
        if(now > endtime)
            return true;
    }
    else
    {
        if(now > endtime +ThreeDay)
            return true;
    }
    return false;

}

bool PlayerActiveConfig::init()
{
    FieldSet* activebase = SceneService::metaData->getFields("t_playeractive");

    if(NULL == activebase)
    {
        Fir::logger->error("找不到t_playeractive的FieldSet");
        return false;
    }

    connHandleID handle = SceneService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }

    RecordSet* recordset = NULL;
    Record col,where;

    col.put("f_id");
    char buf[255];
    sprintf(buf," f_id != 0 and f_open > 0");
    where.put("f_id",buf);

    recordset = SceneService::dbConnPool->exeSelect(handle, activebase, &col, &where);//得到所有活动
    SceneService::dbConnPool->putHandle(handle);
    if(recordset != NULL) 
    {
        for(unsigned int i = 0; i<recordset->size(); i++)
        {   
            Record *rec = recordset->get(i);
            QWORD fID = rec->get("f_id");
            activedata m_base; 
            if(!readactive(fID,m_base))
            {
                continue;
            }
            if(isNeedDel(SceneTimeTick::currentTime.sec(),m_base.f_endtime,static_cast<HelloKittyMsgData::ActiveConditionType>(m_base.f_condition)))
            {
                continue;
            }
            HelloKittyMsgData::PlayerActive rinfo;
            rinfo.set_f_type((HelloKittyMsgData::ActiveOpType)m_base.f_type);
            rinfo.set_f_subtype((HelloKittyMsgData::ActiveSubType)m_base.f_subtype);
            rinfo.set_f_id(m_base.f_id);
            rinfo.set_f_begintime(m_base.f_begintime);
            rinfo.set_f_endtime(m_base.f_endtime);
            rinfo.set_f_condition(static_cast<HelloKittyMsgData::ActiveConditionType>(m_base.f_condition));
            rinfo.set_f_conditionparam(m_base.f_conditionparam);
            rinfo.set_f_preactive(m_base.f_preactive);
            rinfo.set_f_award(m_base.f_award);
            rinfo.set_f_title(m_base.f_title);
            rinfo.set_f_desc(m_base.f_desc);
            rinfo.set_f_open(m_base.f_open);
            rinfo.set_f_platemsg(m_base.f_platemsg);
            rinfo.set_f_rewardmaxcnt(m_base.f_rewardmaxcnt);
            rinfo.set_f_rewardcurcnt(m_base.f_rewardcurcnt);
            add(rinfo);
        }

    }
    return recordset != NULL;


}

bool PlayerActiveConfig::add(const HelloKittyMsgData::PlayerActive &rinfo)
{
    DWORD DwNow  =  SceneTimeTick::currentTime.sec(); 
    active_state state = state_wait;
    if(DwNow >= rinfo.f_begintime())
    {
        state = state_run;
        if(rinfo.f_endtime() > 0)
        {
            if(DwNow > rinfo.f_endtime())
            {
                state = state_finish;
                if(isNeedDel(DwNow, rinfo.f_endtime(),rinfo.f_condition()))
                {
                    state = state_del;
                }
            }
        }

    }
    m_mapActive[rinfo.f_id()] = rinfo;
    setState(rinfo.f_id(),state);
    //有奖励限制的放redis里面去
    if(rinfo.f_rewardmaxcnt())
    {
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(rinfo.f_id());
        if(redis)
        {
            DWORD num = redis->getInt("limitactive",rinfo.f_id(),"rewardlimitmax");
            if(!num)
            {
                redis->setInt("limitactive",rinfo.f_id(),"rewardlimitmax",rinfo.f_rewardmaxcnt());
            }
            num = redis->getInt("limitactive",rinfo.f_id(),"rewardlimitcur");
            if(!num)
            {
                redis->setInt("limitactive",rinfo.f_id(),"rewardlimitcur",rinfo.f_rewardcurcnt());
            }
        }
    }
    return true;
}

bool PlayerActiveConfig::del(DWORD activeid)
{
    setState(activeid,state_del);
    return true;

}

const HelloKittyMsgData::PlayerActive * PlayerActiveConfig::getinfo(DWORD activeid) const
{
    auto it = m_mapActive.find(activeid);
    return it== m_mapActive.end() ? NULL :&(it->second);
}

const std::map<DWORD,HelloKittyMsgData::PlayerActive>& PlayerActiveConfig::getAllinfo() const
{
    return m_mapActive;
}

bool PlayerActiveConfig::readactive(DWORD fID,activedata &m_base)
{
    connHandleID handle = SceneService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        return false;
    }
    char where[128]={0};
    snprintf(where, sizeof(where) - 1, "f_id=%u", fID);
    unsigned int retcode = SceneService::dbConnPool->exeSelectLimit(handle, "t_playeractive", activedb, where, "f_id DESC", 1, (BYTE *)(&m_base));
    SceneService::dbConnPool->putHandle(handle);
    return 1 == retcode;
}

bool PlayerActiveConfig::isSaveType(HelloKittyMsgData::ActiveConditionType type) const
{
    switch(type)
    {
        case HelloKittyMsgData::ActiveConditionType_Composite_Number:
        case HelloKittyMsgData::ActiveConditionType_Fruit_Production_Number:
        case HelloKittyMsgData::ActiveConditionType_Family_Trust:
        case HelloKittyMsgData::ActiveConditionType_Family_Compete:
        case HelloKittyMsgData::ActiveConditionType_Family_Order:
        case HelloKittyMsgData::ActiveConditionType_Regular_Order_Number:
        case HelloKittyMsgData::ActiveConditionType_Helicopter_Order_Number:
        case HelloKittyMsgData::ActiveConditionType_Family_Order_Number:
        case HelloKittyMsgData::ActiveConditionType_Airship_Order_Number:
        case HelloKittyMsgData::ActiveConditionType_Train_Order_Number:
        case HelloKittyMsgData::ActiveConditionType_Expand_Materials:
        case HelloKittyMsgData::ActiveConditionType_Warehouse_Material:
        case HelloKittyMsgData::ActiveConditionType_Activate_Material:
        case HelloKittyMsgData::ActiveConditionType_Star_Upgrade_aterial:
        case HelloKittyMsgData::ActiveConditionType_Pal_Materials:
        case HelloKittyMsgData::ActiveConditionType_Purchase_All:
        case HelloKittyMsgData::ActiveConditionType_Coin_Total:
        case HelloKittyMsgData::ActiveConditionType_Diamond_Total:
        case HelloKittyMsgData::ActiveConditionType_Token_Total:
        case HelloKittyMsgData::ActiveConditionType_Ticket_Total:
        case HelloKittyMsgData::ActiveConditionType_Friendship_Point_Total:
        case HelloKittyMsgData::ActiveConditionType_Sushi_Game:
        case HelloKittyMsgData::ActiveConditionType_Constellation_Lines:
        case HelloKittyMsgData::ActiveConditionType_Gashapon_Diamond_Consumption_Total:
        case HelloKittyMsgData::ActiveConditionType_Gashapon_Friendship_Point_Consumption_Total:
        case HelloKittyMsgData::ActiveConditionType_Divination_Number:
        case HelloKittyMsgData::ActiveConditionType_Bidding_Join_Number:
        case HelloKittyMsgData::ActiveConditionType_Auction_Success_Number:
        case HelloKittyMsgData::ActiveConditionType_Coin_Consumption_Total:
        case HelloKittyMsgData::ActiveConditionType_Diamond_Consumption_Total:
        case HelloKittyMsgData::ActiveConditionType_Employment_Number_All:
        case HelloKittyMsgData::ActiveConditionType_Employment_Consumption_All:
        case HelloKittyMsgData::ActiveConditionType_Find_Number_All:
        case HelloKittyMsgData::ActiveConditionType_Hangup_Number_All:
        case HelloKittyMsgData::ActiveConditionType_Give_Gift_Worth:
        case HelloKittyMsgData::ActiveConditionType_Accept_Gift_Worth:
        case HelloKittyMsgData::ActiveConditionType_Give_Gift_Number:
        case HelloKittyMsgData::ActiveConditionType_Accept_Gift_Number:

            {
                return false;
            }

        case HelloKittyMsgData::ActiveConditionType_Charm_Current_Month:
        case HelloKittyMsgData::ActiveConditionType_Charm_Current_Week:
        case HelloKittyMsgData::ActiveConditionType_Contribution_Current_Month:
        case HelloKittyMsgData::ActiveConditionType_Contribution_Current_Week:
        case HelloKittyMsgData::ActiveConditionType_First_Charge:
        case HelloKittyMsgData::ActiveConditionType_Daily_Charge:
        case HelloKittyMsgData::ActiveConditionType_Daily_Charge_Sum:
        case HelloKittyMsgData::ActiveConditionType_All_Charge_Sum:
        case HelloKittyMsgData::ActiveConditionType_Active_Charge:
        case HelloKittyMsgData::ActiveConditionType_Popular_Building:
        case HelloKittyMsgData::ActiveConditionType_Decorations:
        case HelloKittyMsgData::ActiveConditionType_Max_Popular:
        case HelloKittyMsgData::ActiveConditionType_Current_Popular:
        case HelloKittyMsgData::ActiveConditionType_All_Popular:
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_Number:
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_Level_All:
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_Level6_Number:
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_Level10_Number:
        case HelloKittyMsgData::ActiveConditionType_Unite_Building__Attributes:
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_All_Current_Popular:
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_All_Max_Popular:
        case HelloKittyMsgData::ActiveConditionType_Park_level:
        case HelloKittyMsgData::ActiveConditionType_Friends_Number:
        case HelloKittyMsgData::ActiveConditionType_Fans_Number:
        case HelloKittyMsgData::ActiveConditionType_Pal_Number:
        case HelloKittyMsgData::ActiveConditionType_Pal_Attributes:
        case HelloKittyMsgData::ActiveConditionType_Pal_Level:
        case HelloKittyMsgData::ActiveConditionType_Pal_Grow:
        case HelloKittyMsgData::ActiveConditionType_Pal_Battle_Power:
        case HelloKittyMsgData::ActiveConditionType_4Star_Pal:
        case HelloKittyMsgData::ActiveConditionType_Charm_List:
        case HelloKittyMsgData::ActiveConditionType_Contribution_List:
        case HelloKittyMsgData::ActiveConditionType_Popular_list:
        case HelloKittyMsgData::ActiveConditionType_Level_List:
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_List:
        case HelloKittyMsgData::ActiveConditionType_Family_Level:
        case HelloKittyMsgData::ActiveConditionType_Family_Experience:
        case HelloKittyMsgData::ActiveConditionType_Charm_All:
        case HelloKittyMsgData::ActiveConditionType_Contribution_All:
            {
                return true;
            }
        default:
            break;


    }
    return false;

}

bool PlayerActiveConfig::isRank(HelloKittyMsgData::ActiveConditionType type) const
{
    switch(type)
    {

        case HelloKittyMsgData::ActiveConditionType_Charm_List:
        case HelloKittyMsgData::ActiveConditionType_Contribution_List:
        case HelloKittyMsgData::ActiveConditionType_Popular_list:
        case HelloKittyMsgData::ActiveConditionType_Level_List:
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_List:
            {
                return true;
            }
        default:
            break;


    }
    return false;

}

const std::set<DWORD> * PlayerActiveConfig::getlistBystate(active_state state) const
{
    auto it =  m_mapState.find(state);
    if(it == m_mapState.end())
        return NULL;
    return &(it->second);
}

bool PlayerActiveConfig::updateDB(const DWORD activeID)
{
    FieldSet* activebase = SceneService::metaData->getFields("t_playeractive");

    if(NULL == activebase)
    {
        Fir::logger->error("找不到t_playeractive的FieldSet");
        return false;
    }

    connHandleID handle = SceneService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }
    zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(activeID);
    if(!redis)
    {
        return false;
    }
    DWORD cur = redis->getInt("limitactive",activeID,"rewardlimitcur");
    Record col,where;
    where.put("f_id",activeID);
    col.put("f_rewardcurcnt",cur);

    //更新限时活动奖励数量
    SceneService::dbConnPool->exeUpdate(handle,activebase,&col, &where);
    SceneService::dbConnPool->putHandle(handle);
    return true;
}
