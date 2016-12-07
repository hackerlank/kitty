#include "achieveRegister.h"
#include "SceneUser.h"
#include "zMemDBPool.h"
#include "zMemDB.h"

TargetRetType checkTargerHave(SceneUser *owner,const pb::Conf_t_Achievement *achieveConf,HelloKittyMsgData::AchieveMent *achieve,const AchieveArg &arg)
{
    TargetRetType ret = Target_Ret_None;
    if(!achieveConf || !owner || (!arg.initFlg && (arg.subType <= Achieve_Sub_Default || arg.subType >= Achieve_Sub_Money_Exchange)))
    {
        return ret;
    }
    const std::map<DWORD,pb::TaskTarget>& targetMap = achieveConf->getTargetMap();
    if(!arg.initFlg)
    {
        auto iter = targetMap.find(arg.key);
        if(iter == targetMap.end())
        {
            return ret;
        }
    }

    DWORD process = 0;
    for(auto iter = targetMap.begin();iter != targetMap.end();++iter)
    {
        switch(arg.subType)
        {
            case Achieve_Sub_Buid_Level:
                {
                    process += owner->m_buildManager.getBuildLevel(iter->first);
                }
                break;
            case Achieve_Sub_Build_Num:
                {
                    process += owner->m_buildManager.getBuildLevelNum(iter->first);
                }
                break;
            case Achieve_Sub_Sorce_Day:
                {
                    process += owner->getHappyFequence(iter->first);
                }
                break;
            case Achieve_Sub_Sorce_Num:
                {
                    if(iter->first == Attr_Open_Avartar || iter->first == Attr_Avartar)
                    {
                        process += owner->m_atlasManager.getAtlasNum(); 
                    }
                    else if(iter->first == Attr_Frined)
                    {
                        std::set<QWORD> friendSet;
                        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
                        if(!handle)
                        {
                            break;
                        }
                        handle->getSet("rolerelation", owner->charid, "friend", friendSet);
                        process += friendSet.size();
                    }
                    else if(iter->first == Attr_Main_Task)
                    {
                        process += owner->m_taskManager.getTaskTypeNum(Task_Type_Main,HelloKittyMsgData::Task_Finish);
                    }
                    else if(iter->first == Attr_Day_Task)
                    {
                        process += owner->m_taskManager.getTaskTypeNum(Task_Type_Day,HelloKittyMsgData::Task_Finish);
                    }
                    else if(iter->first == Attr_Total_Task)
                    {
                        process += owner->m_taskManager.getTaskNum();
                    }
                    else if(iter->first == Attr_KittyGraden_Process)
                    {
                        process += 0;
                    }
                    else if(iter->first == Attr_Money_Gold)
                    {
                        process += owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Gold);
                    }
                    else if(iter->first == Attr_Popular_Now)
                    {
                        process += owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Now);
                    }
                    else if(iter->first == Attr_Open_Grid_Val)
                    {
                        process += owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Open_Grid_Time);
                    }
                    else
                    {
                        Fir::logger->error("%s(%u,%u,%lu,%s)", __PRETTY_FUNCTION__,arg.subType,iter->first,owner->charid,owner->charbase.nickname);
                    }
                }
                break;
            default:
                {
                    Fir::logger->error("%s(%u,%lu,%s)", __PRETTY_FUNCTION__,arg.subType,owner->charid,owner->charbase.nickname);
                    break;
                }
        }
    }
    
    if(achieve->total() != achieveConf->achievement->process())
    {
        achieve->set_total(achieveConf->achievement->process());
    }
    if(achieve->current() != process)
    {
        achieve->set_current(process);
        ret = Target_Ret_Update;
    }
    if(achieve->current() >= achieve->total())
    {
        ret = Target_Ret_Finish;
    }
    return ret;
}

TargetRetType checkTargerExchange(SceneUser *owner,const pb::Conf_t_Achievement *achieveConf,HelloKittyMsgData::AchieveMent *achieve,const AchieveArg &arg)
{
    TargetRetType ret = Target_Ret_None;
    if(!achieveConf || !owner || (!arg.initFlg && (arg.subType < Achieve_Sub_Money_Exchange)))
    {
        return ret;
    }
    const std::map<DWORD,pb::TaskTarget>& targetMap = achieveConf->getTargetMap();
    if(!arg.initFlg)
    {
        auto iter = targetMap.find(arg.key);
        if(iter == targetMap.end())
        {
            return ret;
        }
    }
    for(auto iter = targetMap.begin();iter != targetMap.end();++iter)
    {
        DWORD process = 0;
        switch(arg.subType)
        {
            case Achieve_Sub_Money_Exchange:
                {
                    if(iter->first == Attr_Money_Gold)
                    {
                        process = 0;
                    }
                }
                break;
            default:
                {
                    Fir::logger->error("%s(%u,%lu,%s)", __PRETTY_FUNCTION__,arg.subType,owner->charid,owner->charbase.nickname);
                    break;
                }
        }
        if(achieve->total() != achieveConf->achievement->process())
        {
            achieve->set_total(achieveConf->achievement->process());
        }
        if(achieve->current() != process)
        {
            achieve->set_current(process);
            ret = Target_Ret_Update;
        }
        if(achieve->current() >= achieve->total())
        {
            ret = Target_Ret_Finish;
        }
    }
    return ret;
}
