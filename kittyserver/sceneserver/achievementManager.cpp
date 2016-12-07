#include "achievementManager.h"
#include "SceneUser.h"
#include "key.h"
#include "tbx.h"
#include "achieveRegister.h"
#include "TimeTick.h"

bool AchievementManager::s_initTargetCheckFlg = false;
std::map<DWORD,AchievementManager::Achieve_Target_Check> AchievementManager::s_tragetCheckMap;

AchievementManager::AchievementManager(SceneUser *owner) : m_owner(owner)
{
    reset();
    initCheckTragerMap();
}

bool AchievementManager::load(const HelloKittyMsgData::Serialize& binary)
{
    reset();
    for(int index = 0;index < binary.achievement_size();++index)
    {
        const HelloKittyMsgData::AchieveMent &achievement = binary.achievement(index);
        QWORD key = hashKey(achievement.id(),achievement.status() == HelloKittyMsgData::Task_Accept ? achievement.stars() + 1 : achievement.stars());
        const pb::Conf_t_Achievement *achievementConf = tbx::Achievement().get_base(key);
        if(!achievementConf)
        {
            continue;
        }
        if(m_achievementMap.find(achievement.id()) != m_achievementMap.end())
        {
            Fir::logger->debug("[加载成就数据出错]:成就id重复(%lu)",achievement.id());
            continue;
        }
        m_achievementMap.insert(std::pair<DWORD,HelloKittyMsgData::AchieveMent>(achievement.id(),achievement));
        if(achievement.status() == HelloKittyMsgData::Task_Accept)
        {
            opTypeMap(achievement.id(),true);
        }
    }
    return true;
}

bool AchievementManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_achievementMap.begin();iter != m_achievementMap.end();++iter)
    {
        HelloKittyMsgData::AchieveMent *achievement = binary.add_achievement();
        if(!achievement)
        {
            continue;
        }
        *achievement = iter->second;
    }
    return true;
}

void AchievementManager::reset()
{
    m_achievementMap.clear();
    m_achieveTypeMap.clear();
}

bool AchievementManager::init()
{
    const std::unordered_map<unsigned int, const pb::Conf_t_Achievement*> &tbxMap = tbx::Achievement().getTbxMap();
    for(auto iter = tbxMap.begin();iter != tbxMap.end();++iter)
    {
        const pb::Conf_t_Achievement* achieveConf = iter->second;
        openAchieve(achieveConf->achievement->id(),0,true);
    }
    return true;
}

bool AchievementManager::openAchieve(const QWORD achieveID,const DWORD stars,const bool initFlg)
{
    HelloKittyMsgData::AchieveMent* achieve = getAchieve(achieveID);
    if(initFlg && achieve)
    {
        return false;
    }
    const pb::Conf_t_Achievement *achieveConf = tbx::Achievement().get_base(hashKey(achieveID,stars+1));
    if(!achieveConf)
    {
        return false;
    }
    
    AchieveArg arg((AchieveTargetType)achieveConf->achievement->targettype(),(AchieveSubTargetType)achieveConf->achievement->targetsubtype());
    arg.initFlg = true;
    auto fun = s_tragetCheckMap.find(arg.targerType);
    if(fun == s_tragetCheckMap.end())
    {
        Fir::logger->debug("[成就]:开启成就失败,成就类型没有对应的注册函数(%lu,%s,%lu,%u)",m_owner->charid,m_owner->charbase.nickname,achieveID,arg.targerType);
        return false;
    }
    
    acceptAchieve(achieveID,stars);
    achieve = getAchieve(achieveID);
    if(!achieve)
    {
        return false;
    }
     
    TargetRetType targetVal = fun->second(m_owner,achieveConf,achieve,arg);
    //矫正一些错误的数值
    if(!achieve->total())
    {
        achieve->set_total(1);
        targetVal = Target_Ret_Update;
    }
    if(achieve->current() > achieve->total())
    {
        achieve->set_current(achieve->total());
        targetVal = Target_Ret_Finish;
    }
    
    if(targetVal == Target_Ret_Update)
    {
        updateAchieve(achieveID);
    }
    else if(targetVal == Target_Ret_Finish)
    {
        finishAchieve(achieveID);
    }
    else
    {
        updateAchieve(achieveID);
    }
    return true;
}

bool AchievementManager::flushAllAchieve()
{
    HelloKittyMsgData::AckAllAchieve message;
    for(auto iter = m_achievementMap.begin();iter != m_achievementMap.end();++iter)
    {
        const HelloKittyMsgData::AchieveMent &achieve = iter->second;
        HelloKittyMsgData::AchieveMent *temp = message.add_achieve();
        if(temp)
        {
            *temp = achieve;
        }
    }

    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool AchievementManager::finishAchieve(const QWORD achieveID)
{
    HelloKittyMsgData::AchieveMent *achieve = getAchieve(achieveID);
    if(!achieve)
    {
        return false;
    }
    if(achieve->status() != HelloKittyMsgData::Task_Accept)
    {
        return false;
    }
    
    QWORD key = hashKey(achieve->id(),achieve->stars()+1);
    const pb::Conf_t_Achievement *achieveConf = tbx::Achievement().get_base(key);
    if(!achieveConf)
    {
        return false;
    }

    std::string now = SceneTimeTick::currentTime.toString();
    DWORD rewardID = 0,rewardNum = 0;
    const std::map<DWORD,DWORD>& tempMap = achieveConf->getRewardMap();
    if(!tempMap.empty())
    {
        rewardID = tempMap.begin()->first;
        rewardNum = tempMap.begin()->second;
    }
    Fir::logger->info("[%s][t_achieved][f_time=%s][f_char_id=%lu][f_achieve_name=%s][f_achieve_level=%u][f_achieve_award=%u][f_award_count=%u]",now.c_str(),now.c_str(),m_owner->charid,achieveConf->achievement->name().c_str(),achieve->stars(),rewardID,rewardNum);
    achieve->set_stars(achieve->stars()+1);
    achieve->set_status(HelloKittyMsgData::Task_Finish);
    achieve->set_current(achieve->total());
    updateAchieve(achieveID);
    return true;
}

bool AchievementManager::opTypeMap(const QWORD achieveID,bool opAdd)
{
    HelloKittyMsgData::AchieveMent *achieve = getAchieve(achieveID);
    if(!achieve)
    {
        return false;
    }
    QWORD key = hashKey(achieve->id(),achieve->stars()+1);
    const pb::Conf_t_Achievement *achieveConf = tbx::Achievement().get_base(key);
    if(!achieveConf)
    {
        return false;
    }
    
    auto iter = m_achieveTypeMap.find(achieveConf->achievement->targettype());
    if(iter == m_achieveTypeMap.end())
    {
        if(!opAdd)
        {
            return false;
        }
        std::set<QWORD> temp;
        temp.insert(achieveID);
        m_achieveTypeMap.insert(std::pair<DWORD,std::set<QWORD>>(achieveConf->achievement->targettype(),temp));
    }
    else
    {
        std::set<QWORD> &temp = const_cast<std::set<QWORD>&>(iter->second);
        if(opAdd)
        {
            temp.insert(achieveID);
        }
        else
        {
            temp.erase(achieveID);
        }
    }
    return true;
}


bool AchievementManager::updateAchieve(const QWORD achieveID)
{
    HelloKittyMsgData::AchieveMent *achieve = getAchieve(achieveID);
    if(!achieve)
    {
        return false;
    }
    
    HelloKittyMsgData::AckUpdateAchieve message;
    HelloKittyMsgData::AchieveMent *temp = message.mutable_achieve();
    if(temp)
    {
        *temp = *achieve;
    }

    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool AchievementManager::isAcceptAchieve(const QWORD achieveID)
{
    HelloKittyMsgData::AchieveMent *achieve = getAchieve(achieveID);
    if(!achieve)
    {
        return false;
    }
    return achieve->status() == HelloKittyMsgData::Task_Accept;
}

bool AchievementManager::acceptAchieve(const QWORD achieveID,const DWORD stars)
{
    if(m_achievementMap.find(achieveID) != m_achievementMap.end())
    {
        HelloKittyMsgData::AchieveMent &acheive = m_achievementMap[achieveID];
        acheive.set_status(HelloKittyMsgData::Task_Accept);
        acheive.set_stars(stars);
    }
    else
    {
        HelloKittyMsgData::AchieveMent acheive;
        acheive.set_id(achieveID);
        acheive.set_status(HelloKittyMsgData::Task_Accept);
        acheive.set_stars(stars);
        acheive.set_current(0);
        acheive.set_total(0);
        m_achievementMap.insert(std::pair<QWORD,HelloKittyMsgData::AchieveMent>(achieveID,acheive));
    }
    opTypeMap(achieveID,true);
    return true;
}

bool AchievementManager::rewardAchieve(const QWORD achieveID)
{
    HelloKittyMsgData::AchieveMent *achieve = getAchieve(achieveID);
    if(!achieve || achieve->status() != HelloKittyMsgData::Task_Finish)
    {
        return false;
    }
    
    QWORD key = hashKey(achieve->id(),achieve->stars());
    const pb::Conf_t_Achievement *achieveConf = tbx::Achievement().get_base(key);
    if(!achieveConf)
    {
        return false;
    }

    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"成就获得(%lu,%u)",achieve->id(),achieve->stars());
    if(!m_owner->m_store_house.hasEnoughSpace(achieveConf->getRewardMap()))
    {
        m_owner->opErrorReturn(HelloKittyMsgData::WareHouse_Is_Full);
        return false;
    }
    m_owner->m_store_house.addOrConsumeItem(achieveConf->getRewardMap(),temp,true);
    achieveConf = tbx::Achievement().get_base(hashKey(achieve->id(),achieve->stars()+1));
    //处于最高星级了
    if(!achieveConf)
    {
        achieve->set_status(HelloKittyMsgData::Task_Award);
        achieve->set_stars(achieve->stars());
        updateAchieve(achieveID);
    }
    else
    {
        openAchieve(achieve->id(),achieve->stars());
    }
    return true;
}

HelloKittyMsgData::AchieveMent* AchievementManager::getAchieve(const QWORD achieveID)
{
    HelloKittyMsgData::AchieveMent* achieve = NULL;
    auto iter = m_achievementMap.find(achieveID);
    if(iter == m_achievementMap.end())
    {
        return achieve;
    }
    achieve = const_cast<HelloKittyMsgData::AchieveMent*>(&iter->second);
    return achieve;
}

bool AchievementManager::target(const AchieveArg &arg)
{
    auto iter = m_achieveTypeMap.find(arg.targerType);
    if(iter == m_achieveTypeMap.end())
    {
        return false;
    }
    auto fun = s_tragetCheckMap.find(arg.targerType);
    if(fun == s_tragetCheckMap.end())
    {
        return false;
    }

    const std::set<QWORD> &tempSet = iter->second;
    for(auto temp = tempSet.begin();temp != tempSet.end();++temp)
    {
        HelloKittyMsgData::AchieveMent *achieve = getAchieve(*temp);
        if(!achieve || achieve->status() != HelloKittyMsgData::Task_Accept)
        {
            continue;
        }

        QWORD key = hashKey(achieve->id(),achieve->stars()+1);
        const pb::Conf_t_Achievement *achieveConf = tbx::Achievement().get_base(key);
        if(!achieveConf || (AchieveTargetType)(achieveConf->achievement->targettype()) != arg.targerType || (AchieveSubTargetType)(achieveConf->achievement->targetsubtype()) != arg.subType)
        {
            continue;
        }
        TargetRetType targetVal = fun->second(m_owner,achieveConf,achieve,arg);
        if(targetVal == Target_Ret_Update)
        {
            updateAchieve(*temp);
        }
        else if(targetVal == Target_Ret_Finish)
        {
            finishAchieve(*temp);
        }
    }
    return true;
}

void AchievementManager::initCheckTragerMap()
{
    if(s_initTargetCheckFlg)
    {
        return;
    }
    s_initTargetCheckFlg = true;
    s_tragetCheckMap.insert(std::pair<DWORD,Achieve_Target_Check>(Achieve_Target_Have,checkTargerHave));
    s_tragetCheckMap.insert(std::pair<DWORD,Achieve_Target_Check>(Achieve_Target_Exchange,checkTargerExchange));
    return;
}
