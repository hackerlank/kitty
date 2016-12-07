//此文件主要放user的一些检测函数

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
#include "serialize.pb.h"
#include "dataManager.h"
#include "tbx.h"
#include "taskAttr.h"
#include "Misc.h"
#include "guide.pb.h"


bool SceneUser::checkLevel(const DWORD num)
{
    return charbase.level >= num;
}

void SceneUser::changeHappyData(HelloKittyMsgData::HappyData *happyData,const DWORD value,const DWORD judegeVal,const DWORD sec,const DWORD days,const bool gmFlg)
{
    if(value >= judegeVal && happyData)
    {
        if(happyData && (gmFlg || (!happyData->time() || sec - happyData->time() >= 24 * 36000)))
        {
            happyData->set_time(sec);
            happyData->set_frequency(happyData->frequency() + days);

            AchieveArg arg(Achieve_Target_Have,Achieve_Sub_Sorce_Day,happyData->grade(),happyData->frequency());
            m_achievementManager.target(arg);
        }
    }
}

void SceneUser::changeHappyData()
{
    DWORD happyVal = m_store_house.getAttr(HelloKittyMsgData::Attr_Happy_Val);
    if(happyVal < HAPPY_LOW)
    {
        return;
    }
    DWORD nowSec = SceneTimeTick::currentTime.sec();
    HelloKittyMsgData::HappyData *happyData = charbin.mutable_happy_hight();
    changeHappyData(happyData,happyVal,HAPPY_HIGHT,nowSec);

    happyData = charbin.mutable_happy_mid();
    changeHappyData(happyData,happyVal,HAPPY_MID,nowSec);

    happyData = charbin.mutable_happy_low();
    changeHappyData(happyData,happyVal,HAPPY_LOW,nowSec);
}

bool SceneUser::changeHappyDataGm(const DWORD happyVal,const DWORD day)
{
    if(happyVal < HAPPY_LOW)
    {
        return false;
    }
    DWORD nowSec = SceneTimeTick::currentTime.sec();
    HelloKittyMsgData::HappyData *happyData = charbin.mutable_happy_hight();
    changeHappyData(happyData,happyVal,HAPPY_HIGHT,nowSec,day,true);

    happyData = charbin.mutable_happy_mid();
    changeHappyData(happyData,happyVal,HAPPY_MID,nowSec,day,true);

    happyData = charbin.mutable_happy_low();
    changeHappyData(happyData,happyVal,HAPPY_LOW,nowSec,day,true);
    return true;
}

DWORD SceneUser::getHappyFequence(const DWORD happyVal)
{
    if(happyVal < HAPPY_LOW)
    {
        return 0;
    }
    HelloKittyMsgData::HappyData *happyData  = NULL;
    if(happyVal >= HAPPY_HIGHT)
    {
        happyData = charbin.mutable_happy_hight();
    }
    else if(happyVal >= HAPPY_MID)
    {
        happyData = charbin.mutable_happy_mid();
    }
    else if(happyVal >= HAPPY_LOW)
    {
        happyData = charbin.mutable_happy_low();
    }
    return happyData ? happyData->frequency() : 0;
}

bool SceneUser::adjustLevel()
{
    DWORD oldLevel = charbase.level;
    const std::unordered_map<unsigned int, const pb::Conf_t_upgrade*> &tbxMap = tbx::upgrade().getTbxMap();
    for(auto iter = tbxMap.begin();iter != tbxMap.end();++iter)
    {
        const pb::Conf_t_upgrade* levelConf = iter->second;
        if(levelConf->upgrade->exp() > m_store_house.getAttr(HelloKittyMsgData::Attr_Exp))
        {
            charbase.level = levelConf->getKey();
        }
    }
    if(oldLevel != charbase.level)
    {
        if(charbase.level > oldLevel)
        {
            m_taskManager.upLevel();
            TaskArgue arg(Target_Role_Level,Attr_Role_Level,Attr_Role_Level,charbase.level);
            m_taskManager.target(arg); 
        }

        //等级排名
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Level);
        if(redishandle)
        {

            redishandle->setSortSet("rank",charid,"level",charbase.level);
        }
        redishandle = zMemDBPool::getMe().getMemDBHandle(charbase.areaType);
        if(redishandle)
        {
            char temp[100] = {0};
            snprintf(temp,sizeof(temp),"arealevel_%u",charbase.areaType);
            redishandle->setSortSet("rank",charid,temp,charbase.level);
        }
    }
    return true;
}

void SceneUser::infoLevel(const DWORD oldExp,const DWORD oldLevel)
{
    if(oldLevel != charbase.level)
    {
        std::string now = SceneTimeTick::currentTime.toString();
        Fir::logger->info("[%s][t_player_level][f_time=%s][f_acc_id=%s][f_char_id=%lu][f_before_exp=%u][f_after_exp=%u][f_before_level=%u][f_after_level=%u]",now.c_str(),now.c_str(),charbase.account,charid,oldExp,m_store_house.getAttr(HelloKittyMsgData::Attr_Exp),oldLevel,charbase.level);
        m_managertrain.checkNewTrain();
        triggerTaskGuid(pb::Conf_t_Guide::TIGGERTYPE_LEVEL,charbase.level);
        checkFunOpen(pb::Conf_t_openfun::eOpen_Level,charbase.level);


    }


}
void SceneUser::triggerTaskGuid(DWORD type,DWORD param)
{
    const  pb::st_Guide* pinfo= pb::Conf_t_Guide::getGuideByType(type,param);
    if(pinfo)
    {
        charbin.set_taskguidid(pinfo->m_ID);
        charbin.set_taskguidstep(0);
        HelloKittyMsgData::Acknewsettaskguide Ack;
        Ack.set_taskguideid(pinfo->m_ID);
        Ack.set_taskguidestep(pinfo->getNextstep(0,false));
        std::string ret;
        encodeMessage(&Ack,ret);
        sendCmdToMe(ret.c_str(),ret.size());
    }
}

void SceneUser::checkFunOpen(pb::Conf_t_openfun::eOpenSource type,DWORD param)
{

    std::set<DWORD> Oldset;
    for(int i =0 ;i != charbin.funiconid_size();i++)
    {
        Oldset.insert(charbin.funiconid(i));

    }
    std::set<DWORD> newSet;
    if(pb::Conf_t_openfun::getNewIconByType(type,param,Oldset,newSet))
    {
        for(auto it = newSet.begin();it != newSet.end();it++)
        {
            charbin.add_funiconid(*it);
            HelloKittyMsgData::AcknewIcon ackNewIcon;
            ackNewIcon.set_funiconid(*it);
            std::string ret;
            encodeMessage(&ackNewIcon,ret);
            sendCmdToMe(ret.c_str(),ret.size());
            if(pb::Conf_t_openfun::FunMarket == *it)
            {
                m_market.initmarket();

            }

        }


    }


}


bool SceneUser::upgrade()
{
    bool changeFlg = false;
    while(true)
    {
        const pb::Conf_t_upgrade* levelConf = tbx::upgrade().get_base(charbase.level);
        if(!levelConf || levelConf->upgrade->exp() > m_store_house.getAttr(HelloKittyMsgData::Attr_Exp))
        {
            break;
        }
        char reMark[100] = {0};
        snprintf(reMark,sizeof(reMark),"升级奖励(%u)",charbase.level);
        addItempOrEmail(levelConf->getRewardMap(),reMark);

        charbase.level += 1;
        updateAttrVal(HelloKittyMsgData::Attr_Level,charbase.level);
        changeFlg = true;
    }
    if(changeFlg)
    {
        m_taskManager.upLevel();
        TaskArgue arg(Target_Role_Level,Attr_Role_Level,Attr_Role_Level,charbase.level);
        m_taskManager.target(arg); 

        //等级排名
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Level);
        if(redishandle)
        {
            redishandle->setSortSet("rank",charid,"level",charbase.level);
        }
        redishandle = zMemDBPool::getMe().getMemDBHandle(charbase.areaType);
        if(redishandle)
        {
            char temp[100] = {0};
            snprintf(temp,sizeof(temp),"arealevel_%u",charbase.areaType);
            redishandle->setSortSet("rank",charid,temp,charbase.level);
        }
        m_trade.openNpcStall();
        m_active.doaction(HelloKittyMsgData::ActiveConditionType_Park_level,1,0,0,HelloKittyMsgData::AST_TimeLimit_Level);
    }
    return true;
}

bool SceneUser::opErrorReturn(const HelloKittyMsgData::ErrorCodeType &errorCode,const DWORD itemID)
{
    HelloKittyMsgData::AckReturnError error;
    error.set_reason(errorCode);
    error.set_itemid(itemID);

    std::string ret;
    encodeMessage(&error,ret);
    return sendCmdToMe(ret.c_str(),ret.size());
}

bool SceneUser::opSuccessReturn(const HelloKittyMsgData::SuccessCodeType &code)
{
    HelloKittyMsgData::AckReturnSuccess success;
    success.set_code(code);

    std::string ret;
    encodeMessage(&success,ret);
    return sendCmdToMe(ret.c_str(),ret.size());
}

bool SceneUser::updateAttrVal(const HelloKittyMsgData::AttrType &attrType,const DWORD value)
{
    HelloKittyMsgData::AckUpdateAttrVal updateAttr;
    updateAttr.set_attrtype(attrType);
    if(attrType == HelloKittyMsgData::Attr_Popular_Now)
    {
        DWORD max = m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Max);
        updateAttr.set_attrval(value < max ? value : max);
    }
    else
    {
        updateAttr.set_attrval(value);
    }
    updateAttr.set_updatechar(charid);
    std::string ret;
    encodeMessage(&updateAttr,ret);
    broadcastMsgInMap(ret.c_str(),ret.size());
    return true;
}

bool SceneUser::addVertiseOther(const DWORD num)
{
    HelloKittyMsgData::DailyData *dailyData = charbin.mutable_dailydata();
    if(!dailyData)
    {
        return false;
    }
    dailyData->set_vistorother(dailyData->vistorother() + num);

    TaskArgue arg(Target_InterActive,Attr_Enter_Garden,Attr_Enter_Garden,dailyData->vistorother());
    m_taskManager.target(arg); 
    return true;
}

bool SceneUser::addFriendNum(const DWORD num)
{
    HelloKittyMsgData::DailyData *dailyData = charbin.mutable_dailydata();
    if(!dailyData)
    {
        return false;
    }
    dailyData->set_addfriend(dailyData->addfriend() + num);

    TaskArgue arg(Target_InterActive,Attr_Frined,Attr_Frined,0);
    m_taskManager.target(arg); 

    TaskArgue arg1(Target_InterActive,Attr_Add_Friend,Attr_Add_Friend,dailyData->addfriend());
    m_taskManager.target(arg1); 

    AchieveArg arg2(Achieve_Target_Have,Achieve_Sub_Sorce_Num,Attr_Frined,0);
    m_achievementManager.target(arg2);
    return true;
}

