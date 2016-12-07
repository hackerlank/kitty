#include "taskRegister.h"
#include "SceneUser.h"
#include "zMemDBPool.h"
#include "zMemDB.h"
#include "RecordFamily.h"

TargetRetType checkBuildTarger(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg)
{
    TargetRetType ret = Target_Ret_None;
    if(!taskConf || !owner || (!arg.initFlg && (arg.attrID < Attr_Build || arg.attrID > Attr_Add_Build_or_Level)))
    {
        return ret;
    }
    const std::map<DWORD,pb::TaskTarget>& targetMap = taskConf->getTargetMap();
    if(!arg.initFlg)
    {
        auto iter = targetMap.find(arg.key);
        if(iter == targetMap.end())
        {
            return ret;
        }
    }
    
    DWORD process = 0,total = 0;
    for(auto iter = targetMap.begin();iter != targetMap.end();++iter)
    {
        const pb::TaskTarget& taskTarget = iter->second;
        total += taskTarget.para1;
        DWORD num = 0;
        if(iter->first == Attr_Add_Build_or_Level)
        {
            HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
            if(arg.initFlg)
            {
                dailyData->set_buildorlevel(0);
            }
            num = dailyData->buildorlevel();
        }
        else
        {
            num = owner->m_buildManager.getBuildLevel(iter->first);
        }
        process += num > taskTarget.para1 ? taskTarget.para1 : num;
    }
    if(!task->total())
    {
        task->set_total(total);
        ret = Target_Ret_Update;
    }
    if(task->current() != process)
    {
        task->set_current(process);
        ret = Target_Ret_Update;
    }
    if(task->current() >= task->total())
    {
        ret = Target_Ret_Finish;
    }
    return ret;
}

TargetRetType checkInterActiveTarger(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg)
{
    TargetRetType ret = Target_Ret_None;
    if(!taskConf || !owner || (!arg.initFlg && (arg.attrID < Attr_Frined || arg.attrID > Attr_Add_Finish_Burst_Event)))
    {
        return ret;
    }
    const std::map<DWORD,pb::TaskTarget>& targetMap = taskConf->getTargetMap();
    if(!arg.initFlg)
    {
        auto iter = targetMap.find(arg.key);
        if(iter == targetMap.end())
        {
            return ret;
        }
    }
    DWORD process = 0,total = 0;
    for(auto iter = targetMap.begin();iter != targetMap.end();++iter)
    {
        const pb::TaskTarget& taskTarget = iter->second;
        total += taskTarget.para1;
        DWORD num = 0;
        switch(iter->first)
        {
            case Attr_Enter_Garden:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(dailyData)
                    {
                        num = dailyData->vistorother();
                    }
                }
                break;
            case Attr_Add_Friend:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(dailyData)
                    {
                        num = dailyData->addfriend();
                    }
                }
                break;
            case Attr_Frined:
                {
                    num = owner->getFriendManager().GetFriendSize(); 
#if 0
                    std::set<QWORD> friendSet;
                    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
                    if(!handle)
                    {
                        break;
                    }
                    handle->getSet("rolerelation", owner->charid, "friend", friendSet);
                    num = friendSet.size();
#endif
                }
                break;
            case Attr_Add_Finish_Burst_Event:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(dailyData)
                    {
                        num = dailyData->finishburstevent();
                    }
                }
                break;
            case Attr_Family:
                {
                    num = RecordFamily::getMe().getFamilyID(owner->charid) ? 1 : 0;
                }
                break;
            default:
                {
                }
                break;
        }
        process += num > taskTarget.para1 ? taskTarget.para1 : num;
    }
    if(!task->total())
    {
        task->set_total(total);
        ret = Target_Ret_Update;
    }
    if(task->current() != process)
    {
        task->set_current(process);
        ret = Target_Ret_Update;
    }
    if(task->current() >= task->total())
    {
        ret = Target_Ret_Finish;
    }
    return ret;
}

TargetRetType checkSourceTarger(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg)
{
    TargetRetType ret = Target_Ret_None;
    if(!taskConf || !owner || (!arg.initFlg && (arg.attrID < Attr_Money_Gold || arg.attrID > Attr_Produce_Get)))
    {
        return ret;
    }
    const std::map<DWORD,pb::TaskTarget>& targetMap = taskConf->getTargetMap();
    if(!arg.initFlg)
    {
        auto iter = targetMap.find(arg.key);
        if(iter == targetMap.end())
        {
            return ret;
        }
    }
    
    DWORD process = 0,total = 0;
    for(auto iter = targetMap.begin();iter != targetMap.end();++iter)
    {
        const pb::TaskTarget& taskTarget = iter->second;
        total += taskTarget.para1;
        DWORD num = 0;
        switch(iter->first)
        {
            case Attr_Composite_Get:
                {
                    num = owner->getItemResourNum(Item_Composite,taskTarget.para2);
                }
                break;
            case Attr_Produce_Get:
                {
                    num = owner->getItemResourNum(Item_Produce,taskTarget.para2);
                }
                break;
            case Attr_SystemOrder_Get:
                {
                    num = owner->getItemResourNum(Item_SysOrder,taskTarget.para2);
                }
                break;
            case Attr_Visitor:
                {
                    num = owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Exp);
                }
                break;
            case Attr_Happy_val:
                {
                    num = owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Happy_Val);
                }
                break;
            case Attr_Money_Gold:
                {
                    num = owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Gold);
                }
                break;
            case Attr_Money_Gem:
                {
                    num = owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Gem);
                }
                break;
            case Attr_Add_Money_Gold:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_addgold(0);
                    }
                    num = dailyData->addgold();
                }
                break;
            case Attr_Add_Money_Gem:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_addgem(0);
                    }
                    num = dailyData->addgem();
                }
                break;
            case Attr_Add_Visitor:
                {   
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_addexp(0);
                    }
                    num = dailyData->addexp();
                }
                break;
            case Attr_Add_Happy_Val:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_addhappy(0);
                    }
                    num = dailyData->addhappy();
                }
                break;
            case Attr_Open_Grid_Val:
                {
                    num = owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Open_Grid_Time);
                }
                break;
            case Attr_Finish_Order:
                {
                    num = owner->charbin.finisnorder();
                }
                break;
            case Attr_Carnival_Num:
                {
                    num = owner->charbin.carnivalnum();
                }
                break;
            case Attr_Popular_Now:
                {
                    num = owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Now);
                }
                break;
            case Attr_Popular_Max:
                {
                    num = owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Max);
                }
                break;
            case Attr_Cost_Gem:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_costgem(0);
                    }
                    num = dailyData->costgem();
                }
                break;
            case Attr_Finish_Task:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_finishtask(0);
                    }
                    num = dailyData->finishtask();
                }
                break;
            case Attr_Composite_Num:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_compositeitem(0);
                    }
                    num = dailyData->compositeitem();
                }
                break;
            case Attr_Finish_Daily_Task:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_finishdailytask(0);
                    }
                    num = dailyData->finishdailytask();
                }
                break;
            case Attr_Trade_Num:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_tradenum(0);
                    }
                    num = dailyData->tradenum();
                }
                break;
            case Attr_Order_Num:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_ordervalue(0);
                    }
                    num = dailyData->ordervalue();
                }
                break;
            case Attr_Train_Get:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_trainget(0);
                    }
                    num = dailyData->trainget();
                }
                break;
            case Attr_Help_Train:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_helptrain(0);
                    }
                    num = dailyData->helptrain();
                }
                break;
            case Attr_Acculate_Popular_Now:
                {
                    if(arg.initFlg)
                    {
                        owner->charbin.set_accpopularnow(0);
                    }
                    num = owner->charbin.accpopularnow();
                }
                break;
            case Attr_Order_Accept_Num:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_orderaccept(0);
                    }
                    num = dailyData->orderaccept();
                }
                break;
            default:
                {
                }
                break;
        }
        process += num > taskTarget.para1 ? taskTarget.para1 : num;
    }
    if(!task->total())
    {
        task->set_total(total);
        ret = Target_Ret_Update;
    }
    if(task->current() != process) 
    {
        task->set_current(process);
        ret = Target_Ret_Update;
    }
    if(task->current() >= task->total())
    {
        ret = Target_Ret_Finish;
    }
    return ret;
}

TargetRetType checkRoleLevelTarget(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg)
{
    TargetRetType ret = Target_Ret_None;
    if(!taskConf || !owner || (!arg.initFlg && arg.attrID != Attr_Role_Level))
    {
        return ret;
    }
    const std::map<DWORD,pb::TaskTarget>& targetMap = taskConf->getTargetMap();
    if(!arg.initFlg)
    {
        auto iter = targetMap.find(arg.key);
        if(iter == targetMap.end())
        {
            return ret;
        }
    }
    DWORD process = 0,total = 0;
    for(auto iter = targetMap.begin();iter != targetMap.end();++iter)
    {
        const pb::TaskTarget& taskTarget = iter->second;
        total += taskTarget.para1;
        DWORD num = 0;
        switch(iter->first)
        {
            case Attr_Role_Level:
                {
                    num = owner->charbase.level;
                }
                break;
            default:
                {
                }
                break;
        }
        process += num > taskTarget.para1 ? taskTarget.para1 : num;
    }
    if(!task->total())
    {
        task->set_total(total);
        ret = Target_Ret_Update;
    }
    if(task->current() != process)
    {
        task->set_current(process);
        ret = Target_Ret_Update;
    }
    if(task->current() >= task->total())
    {
        ret = Target_Ret_Finish;
    }
    return ret;
}

TargetRetType checkAtlasTarget(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg)
{
    TargetRetType ret = Target_Ret_None;
    if(!taskConf || !owner || (!arg.initFlg && (arg.attrID < Attr_Open_Avartar || arg.attrID > Attr_Add_Atlas)))
    {
        return ret;
    }
    const std::map<DWORD,pb::TaskTarget>& targetMap = taskConf->getTargetMap();
    if(!arg.initFlg)
    {
        auto iter = targetMap.find(arg.key);
        if(iter == targetMap.end())
        {
            return ret;
        }
    }
    DWORD process = 0,total = 0;
    for(auto iter = targetMap.begin();iter != targetMap.end();++iter)
    {
        const pb::TaskTarget& taskTarget = iter->second;
        total += taskTarget.para1;
        DWORD num = 0;
        switch(iter->first)
        {
            case Attr_Open_Avartar:
            case Attr_Avartar:
                {
                    num = owner->m_atlasManager.getAtlasNum();
                }
                break;
            case Attr_Add_Atlas:
                {
                    HelloKittyMsgData::DailyData *dailyData = owner->charbin.mutable_dailydata();
                    if(arg.initFlg)
                    {
                        dailyData->set_addatlas(0);
                    }
                    num = dailyData->addatlas();
                }
                break;
            default:
                {
                }
                break;
        }
        process += num > taskTarget.para1 ? taskTarget.para1 : num;
    }
    if(!task->total())
    {
        task->set_total(total);
        ret = Target_Ret_Update;
    }
    if(task->current() != process) 
    {
        task->set_current(process);
        ret = Target_Ret_Update;
    }
    if(task->current() >= task->total())
    {
        ret = Target_Ret_Finish;
    }
    return ret;
}

TargetRetType checkAvartarTarget(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg)
{
    TargetRetType ret = Target_Ret_None;
    if(!taskConf || !owner || (!arg.initFlg && (arg.attrID < Attr_Fashion_Dress || arg.attrID > Attr_Fashion_Get)))
    {
        return ret;
    }
    const std::map<DWORD,pb::TaskTarget>& targetMap = taskConf->getTargetMap();
    if(!arg.initFlg)
    {
        auto iter = targetMap.find(arg.key);
        if(iter == targetMap.end())
        {
            return ret;
        }
    }
    DWORD process = 0,total = 0;
    for(auto iter = targetMap.begin();iter != targetMap.end();++iter)
    {
        const pb::TaskTarget& taskTarget = iter->second;
        total += taskTarget.para1;
        DWORD num = 0;
        switch(iter->first)
        {
            case Attr_Fashion_Dress:
                {
                    num = owner->charbin.dress().id() ? 1 : 0;
                }
                break;
            case Attr_Fashion_Has:
                {
                    num = owner->m_dressManager.findLevel(taskTarget.para2);
                }
                break;
            case Attr_Fashion_Change:
                {
                    num = owner->charbin.dress().id() ? 1 : 0;
                }
                break;
            case Attr_Fashion_Level:
                {
                    num = owner->m_dressManager.findHigestLevel();
                }
                break;
            case Attr_Fashion_Get:
                {
                    num = owner->m_dressManager.getDressNum();
                }
                break;
            default:
                {
                }
                break;
        }
        process += num > taskTarget.para1 ? taskTarget.para1 : num;
    }
    if(!task->total())
    {
        task->set_total(total);
        ret = Target_Ret_Update;
    }
    if(task->current() != process) 
    {
        task->set_current(process);
        ret = Target_Ret_Update;
    }
    if(task->current() >= task->total())
    {
        ret = Target_Ret_Finish;
    }
    return ret;
}

TargetRetType checkDivineTarget(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg)
{
    TargetRetType ret = Target_Ret_None;
    if(!taskConf || !owner || (!arg.initFlg && (arg.attrID != Attr_Divine)))
    {
        return ret;
    }
    const std::map<DWORD,pb::TaskTarget>& targetMap = taskConf->getTargetMap();
    if(!arg.initFlg)
    {
        auto iter = targetMap.find(arg.key);
        if(iter == targetMap.end())
        {
            return ret;
        }
    }
    DWORD process = 0,total = 0;
    for(auto iter = targetMap.begin();iter != targetMap.end();++iter)
    {
        const pb::TaskTarget& taskTarget = iter->second;
        total += taskTarget.para1;
        DWORD num = 0;
        switch(iter->first)
        {
            case Attr_Divine:
                {
                    num = owner->charbin.dailydata().divine().randtime();
                }
                break;
            default:
                {
                }
                break;
        }
        process += num > taskTarget.para1 ? taskTarget.para1 : num;
    }
    if(!task->total())
    {
        task->set_total(total);
        ret = Target_Ret_Update;
    }
    if(task->current() != process) 
    {
        task->set_current(process);
        ret = Target_Ret_Update;
    }
    if(task->current() >= task->total())
    {
        ret = Target_Ret_Finish;
    }
    return ret;
}

TargetRetType checkToyTarget(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg)
{
    TargetRetType ret = Target_Ret_None;
    if(!taskConf || !owner || (!arg.initFlg && (arg.attrID != Attr_Toy)))
    {
        return ret;
    }
    const std::map<DWORD,pb::TaskTarget>& targetMap = taskConf->getTargetMap();
    if(!arg.initFlg)
    {
        auto iter = targetMap.find(arg.key);
        if(iter == targetMap.end())
        {
            return ret;
        }
    }
    DWORD process = 0,total = 0;
    for(auto iter = targetMap.begin();iter != targetMap.end();++iter)
    {
        const pb::TaskTarget& taskTarget = iter->second;
        total += taskTarget.para1;
        DWORD num = 0;
        switch(iter->first)
        {
            case Attr_Toy:
                {
                    num = owner->charbin.dailydata().randtoy();
                }
                break;
            default:
                {
                }
                break;
        }
        process += num > taskTarget.para1 ? taskTarget.para1 : num;
    }
    if(!task->total())
    {
        task->set_total(total);
        ret = Target_Ret_Update;
    }
    if(task->current() != process) 
    {
        task->set_current(process);
        ret = Target_Ret_Update;
    }
    if(task->current() >= task->total())
    {
        ret = Target_Ret_Finish;
    }
    return ret;
}


