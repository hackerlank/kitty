#include "PlayerActiveManager.h"
#include "TimeTick.h"
#include "SceneUser.h"
#include "PlayerActiveConfig.h"
#include "dataManager.h"
#include "RedisMgr.h"
#include "RecordFamily.h"

PlayerActiveManager::PlayerActiveManager(SceneUser& rUser):m_rUser(rUser)
{

}

PlayerActiveManager::~PlayerActiveManager()
{

}

void PlayerActiveManager::timerCheck()
{
    std::set<DWORD> setDel;
    for(auto it = m_mapActive.begin() ;it != m_mapActive.end() ;it++)
    {
        //活动已经删除的
        const HelloKittyMsgData::PlayerActive * pinfo = PlayerActiveConfig::getMe().getinfo(it->first);
        if(pinfo == NULL)
        {
            setDel.insert(it->first);
            continue;
        }
        //定时清理的
        bool bdel = false;
        switch(pinfo->f_condition())
        {
            case HelloKittyMsgData::ActiveConditionType_Charm_Current_Month:
            case HelloKittyMsgData::ActiveConditionType_Contribution_Current_Month:
                {
                    DWORD now = SceneTimeTick::currentTime.sec();
                    struct tm tm_old,tm_now;
                    zRTime::getLocalTime(tm_now, now);
                    zRTime::getLocalTime(tm_old, it->second.lastchecktimer());
                    if(tm_old.tm_year != tm_now.tm_year || tm_old.tm_mon != tm_now.tm_mon)
                    {
                        bdel = true;
                    }

                }
                break;
            case HelloKittyMsgData::ActiveConditionType_Charm_Current_Week:
            case HelloKittyMsgData::ActiveConditionType_Contribution_Current_Week:
                {
                    if(it->second.lastchecktimer() == 0)
                    {
                        bdel = true; 
                    }
                    else
                    {
                        DWORD now = SceneTimeTick::currentTime.sec();
                        struct tm tm_old,tm_now;
                        zRTime::getLocalTime(tm_now, now);
                        zRTime::getLocalTime(tm_old, it->second.lastchecktimer());
                        if(now - it->second.lastchecktimer() >= 7*24*3600 || tm_now.tm_wday < tm_old.tm_wday)
                        {
                            bdel = true;
                        }


                    }

                }
                break;
            case HelloKittyMsgData::ActiveConditionType_Daily_Charge:
            case HelloKittyMsgData::ActiveConditionType_Daily_Charge_Sum:
                {
                    DWORD now = SceneTimeTick::currentTime.sec();
                    struct tm tm_old,tm_now;
                    zRTime::getLocalTime(tm_now, now);
                    zRTime::getLocalTime(tm_old, it->second.lastchecktimer());
                    if(tm_old.tm_year != tm_now.tm_year || tm_old.tm_mon != tm_now.tm_mon || tm_old.tm_mday != tm_now.tm_mday)
                    {
                        bdel = true;
                    }


                }
                break;
            default:
                break;
        }
        if(bdel)
        {
            setDel.insert(it->first);
        }
    }
    for(auto it = setDel.begin(); it != setDel.end();it++)
    {
        m_mapActive.erase(*it);
    }
    update(setDel);
}


void PlayerActiveManager::load(const HelloKittyMsgData::Serialize& binary)
{
    for (int i = 0; i < binary.activeinfo_size(); i++) {
        m_mapActive[binary.activeinfo(i).f_id()] = binary.activeinfo(i);
    }

}

void PlayerActiveManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto it = m_mapActive.begin(); it != m_mapActive.end() ;it++)
    {
        HelloKittyMsgData::PlayerActiveSave *psave = binary.add_activeinfo();
        if(psave)
        {
            *psave = it->second;
        }
    }
}

HelloKittyMsgData::PlayerActiveSave * PlayerActiveManager::getSaveInfo(DWORD activeid)
{
    auto it = m_mapActive.find(activeid);
    return it== m_mapActive.end() ? NULL : &(it->second);
}

void PlayerActiveManager::ReqgetActiveAward(const HelloKittyMsgData::ReqgetActiveAward* cmd)
{
    DWORD max = 0,cur = 0;
    HelloKittyMsgData::ActiveResult result = HelloKittyMsgData::ActiveResult_Suc;
    do{
        const HelloKittyMsgData::PlayerActive * pinfo = PlayerActiveConfig::getMe().getinfo(cmd->f_id());
        if(pinfo ==NULL)
        {
            result = HelloKittyMsgData::ActiveResult_NotFind;
            break;
        }
        HelloKittyMsgData::PlayerActiveSave * pSave = getSaveInfo(cmd->f_id());
        if(pSave != NULL && pSave->getaward())
        {
            result = HelloKittyMsgData::ActiveResult_HasGetAward;
            break;
        }
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(cmd->f_id());
        if(redis && pinfo->f_type() == HelloKittyMsgData::AT_TimeLimit)
        {
            max = redis->getInt("limitactive",cmd->f_id(),"rewardlimitmax");
            if(max)
            {
                cur = redis->getInt("limitactive",cmd->f_id(),"rewardlimitcur");
                if(max && cur >= max)
                {
                    result = HelloKittyMsgData::ActiveResult_AwardOut;
                    break;
                }
            }
        }
        if(PlayerActiveConfig::getMe().isSaveType(pinfo->f_condition()))
        {
            if(!PlayerActiveConfig::getMe().isRank(pinfo->f_condition()))
            {
                if(getselfparam(pinfo->f_id(),pinfo->f_condition()) < pinfo->f_conditionparam())
                {
                    result = HelloKittyMsgData::ActiveResult_CaNNotGetAward;
                    break;
                }

            }
            else
            {
                DWORD rankdata = getselfparam(pinfo->f_id(),pinfo->f_condition());
                if(rankdata == 0 || rankdata > pinfo->f_conditionparam())
                {
                    result = HelloKittyMsgData::ActiveResult_CaNNotGetAward; 
                    break;
                }
            }

        }
        else
        {
            if(pSave ==NULL || pSave->currentparam() < pinfo->f_conditionparam())
            {
                result = HelloKittyMsgData::ActiveResult_CaNNotGetAward;
                break;
            }
        }
        //push
        std::map<DWORD,DWORD> resultMap;
        pb::parseDWORDToDWORDMap(pinfo->f_award(),resultMap);
        if(!m_rUser.m_store_house.addOrConsumeItem(resultMap,"get active award"))
        {
            result = HelloKittyMsgData::ActiveResult_PacketFull;
            break;
        }
        DWORD now = SceneTimeTick::currentTime.sec();
        if(pSave != NULL)
        {
            pSave->set_getaward(true);
            pSave->set_lastchecktimer(now);

        }
        else
        {
            HelloKittyMsgData::PlayerActiveSave rSave;
            rSave.set_f_id(cmd->f_id());
            rSave.set_getaward(true); 
            rSave.set_lastchecktimer(now);
            rSave.set_currentparam(0);
            m_mapActive[cmd->f_id()] = rSave;
        }
        redis = zMemDBPool::getMe().getMemDBHandle(cmd->f_id());
        if(redis && pinfo->f_type() == HelloKittyMsgData::AT_TimeLimit)
        {
            if(max)
            {
                DWORD status = redis->getInt("limitactive",cmd->f_id(),"status");
                if(!status)
                {
                    redis->setInt("limitactive",cmd->f_id(),"status",1);
                    cur += 1;
                    redis->setInt("limitactive",cmd->f_id(),"rewardlimitcur",cur);
                    redis->setInt("limitactive",cmd->f_id(),"status",0);
                    PlayerActiveConfig::getMe().updateDB(cmd->f_id());
                }
            }
        }
        std::set<DWORD> updateSet;
        updateSet.insert(cmd->f_id());
        update(updateSet);
    }while(0);
    std::string ret;
    HelloKittyMsgData::AckgetActiveAward ack;
    ack.set_result(result);
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());
}

void PlayerActiveManager::ReqgetPlayerActiveList(const HelloKittyMsgData::ReqgetPlayerActiveList* cmd)
{
    HelloKittyMsgData::Ackgetplayeractivelist ack;
    ack.set_init(true);

    //等待的活动
    const std::set<DWORD> * pwaitset = PlayerActiveConfig::getMe().getlistBystate(state_wait);
    if(pwaitset != NULL)
    {
        for(auto it = pwaitset->begin();it != pwaitset->end() ;it++)
        {
            const HelloKittyMsgData::PlayerActive * pinfo = PlayerActiveConfig::getMe().getinfo(*it);
            if(pinfo == NULL)
                continue;
            //前置不符合，pass 
            if(pinfo->f_preactive() > 0)
            {
                HelloKittyMsgData::PlayerActiveSave * pPre = getSaveInfo(pinfo->f_preactive());
                if(pPre ==NULL)
                {
                    continue;
                }
                if(pPre->getaward() == false)
                {
                    continue;
                }
            }
            HelloKittyMsgData::PlayerActiveForClient *pcli = ack.add_info();
            if(pcli ==NULL)
            {
                continue;
            }
            HelloKittyMsgData::PlayerActive* pbase = pcli->mutable_baseinfo();
            *pbase = *pinfo;
            pcli->set_state(HelloKittyMsgData::ActiveState_NoBegin);
            HelloKittyMsgData::PlayerActiveSave * psave = getSaveInfo(*it);
            if(psave == NULL)
            {
                pcli->set_getaward(false);
            }
            else
            {
                pcli->set_getaward(psave->getaward());
            }
            if(PlayerActiveConfig::getMe().isSaveType(pinfo->f_condition()))
            {
                pcli->set_currentparam(getselfparam(pinfo->f_id(),pinfo->f_condition()));
            }
            else
            {
                if(psave == NULL)
                {
                    pcli->set_currentparam(0);
                }
                else
                {
                    pcli->set_currentparam(psave->currentparam());
                }
            }
            zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(*it);
            if(redis)
            {
                DWORD num = redis->getInt("limitactive",*it,"rewardlimitmax");
                pbase->set_f_rewardmaxcnt(num);
                num = redis->getInt("limitactive",*it,"rewardlimitcur");
                pbase->set_f_rewardcurcnt(num);
            }

        }
    }
    //已经开始的活动
    const std::set<DWORD> * prunset = PlayerActiveConfig::getMe().getlistBystate(state_run);
    if(prunset != NULL)
    {
        for(auto it = prunset->begin();it != prunset->end() ;it++)
        {
            const HelloKittyMsgData::PlayerActive * pinfo = PlayerActiveConfig::getMe().getinfo(*it);
            if(pinfo == NULL)
                continue;
            //前置不符合，pass 
            if(pinfo->f_preactive() > 0)
            {
                HelloKittyMsgData::PlayerActiveSave * pPre = getSaveInfo(pinfo->f_preactive());
                if(pPre ==NULL)
                {
                    continue;
                }
                if(pPre->getaward() == false)
                {
                    continue;
                }
            }
            HelloKittyMsgData::PlayerActiveForClient *pcli = ack.add_info();
            if(pcli ==NULL)
            {
                continue;
            }
            HelloKittyMsgData::PlayerActive* pbase = pcli->mutable_baseinfo();
            *pbase = *pinfo;
            pcli->set_state(HelloKittyMsgData::ActiveState_Start);
            HelloKittyMsgData::PlayerActiveSave * psave = getSaveInfo(*it);
            if(psave == NULL)
            {
                pcli->set_getaward(false);
            }
            else
            {
                pcli->set_getaward(psave->getaward());
            }
            if(PlayerActiveConfig::getMe().isSaveType(pinfo->f_condition()))
            {
                pcli->set_currentparam(getselfparam(pinfo->f_id(),pinfo->f_condition()));
            }
            else
            {
                if(psave == NULL)
                {
                    pcli->set_currentparam(0);
                }
                else
                {
                    pcli->set_currentparam(psave->currentparam());
                }
            }
            zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(*it);
            if(redis)
            {
                DWORD num = redis->getInt("limitactive",*it,"rewardlimitmax");
                pbase->set_f_rewardmaxcnt(num);
                num = redis->getInt("limitactive",*it,"rewardlimitcur");
                pbase->set_f_rewardcurcnt(num);
            }

        }
    }
    //已经结束的活动
    const std::set<DWORD> * pfinish = PlayerActiveConfig::getMe().getlistBystate(state_finish);
    if(pfinish != NULL)
    {
        for(auto it = pfinish->begin();it != pfinish->end() ;it++)
        {
            const HelloKittyMsgData::PlayerActive * pinfo = PlayerActiveConfig::getMe().getinfo(*it);
            if(pinfo == NULL)
                continue;
            if(pinfo->f_preactive() > 0)
            {
                HelloKittyMsgData::PlayerActiveSave * pPre = getSaveInfo(pinfo->f_preactive());
                if(pPre ==NULL)
                {
                    continue;
                }
                if(pPre->getaward() == false)
                {
                    continue;
                }
            }
            HelloKittyMsgData::PlayerActiveForClient *pcli = ack.add_info();
            if(pcli ==NULL)
            {
                continue;
            }
            HelloKittyMsgData::PlayerActive* pbase = pcli->mutable_baseinfo();
            *pbase = *pinfo;
            pcli->set_state(HelloKittyMsgData::ActiveState_Finish);
            HelloKittyMsgData::PlayerActiveSave * psave = getSaveInfo(*it);
            if(psave == NULL)
            {
                pcli->set_getaward(false);
            }
            else
            {
                pcli->set_getaward(psave->getaward());
            }
            if(psave == NULL)
            {
                pcli->set_currentparam(0);
            }
            else
            {
                pcli->set_currentparam(psave->currentparam());
            }
            zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(*it);
            if(redis)
            {
                DWORD num = redis->getInt("limitactive",*it,"rewardlimitmax");
                pbase->set_f_rewardmaxcnt(num);
                num = redis->getInt("limitactive",*it,"rewardlimitcur");
                pbase->set_f_rewardcurcnt(num);
            }
        }
    }
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());


}

DWORD PlayerActiveManager::getselfparam(const DWORD activeID,HelloKittyMsgData::ActiveConditionType type)
{
    DWORD param = 0;
    switch(type)
    {
        case HelloKittyMsgData::ActiveConditionType_Charm_Current_Month://本月魅力值
            {
                return m_rUser.charbin.charismamonth();

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Charm_Current_Week://本周魅力值
            {
                return m_rUser.charbin.charismaweek();
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Contribution_Current_Month://本月贡献值
            {
                return m_rUser.charbin.contributemonth();
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Contribution_Current_Week://本周贡献值
            {
                return m_rUser.charbin.contributeweek();
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_First_Charge://首冲
            {
                return m_rUser.charbin.firstrecharge();

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Daily_Charge://每日首值
            {
                return m_rUser.charbin.dailydata().rechargetodayfrist();

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Daily_Charge_Sum://每日充值
            {
                return m_rUser.charbin.dailydata().rechargetoday();
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_All_Charge_Sum://总计充值
            {
                return m_rUser.charbin.rechargetotal();
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Active_Charge: //活动充值
            {
                return m_rUser.getActiveRecharge(activeID);
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Popular_Building://人气建筑
            {
                return m_rUser.m_buildManager.getPopularBuildingNum();

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Decorations://装饰物
            {
                return m_rUser.m_buildManager.getDecorationsBuildingNum();
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Max_Popular://人气上限
            {
                return m_rUser.m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Max);
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Current_Popular://人气值
            {
                return m_rUser.m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Now);

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_All_Popular://人气总值,2,上限+人气
            {
                return m_rUser.m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Now) + m_rUser.m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Max);
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_Number://合建建筑总数
            {
                return m_rUser.m_buildManager.getUinityBuildnum();
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_Level_All://合建建筑总等级
            {
                return m_rUser.m_buildManager.getUinityBuildLevelTotal();

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_Level6_Number://合建建筑6级数量
            {
                return m_rUser.m_buildManager.getUinityBuildnumByLevel(6);

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_Level10_Number://合建建筑10级数量
            {
                return m_rUser.m_buildManager.getUinityBuildnumByLevel(10);
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Unite_Building__Attributes://合建建筑总属性
            {
                HelloKittyMsgData::MaxUnityBuild rscorebuild;
                rscorebuild.set_maxscore(0);
                rscorebuild.set_buildid(0);
                rscorebuild.set_buildlv(0); 
                rscorebuild.set_otherid(0);
                rscorebuild.set_totalpopular(0); 
                rscorebuild.set_totalmaxpopular(0);
                RedisMgr::getMe().get_unitybuildrankdata(m_rUser.charid,rscorebuild);
                return rscorebuild.maxscore();
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_All_Current_Popular://合建建筑人气总
            {
                HelloKittyMsgData::MaxUnityBuild rscorebuild;
                rscorebuild.set_maxscore(0);
                rscorebuild.set_buildid(0);
                rscorebuild.set_buildlv(0); 
                rscorebuild.set_otherid(0);
                rscorebuild.set_totalpopular(0); 
                rscorebuild.set_totalmaxpopular(0);
                RedisMgr::getMe().get_unitybuildrankdata(m_rUser.charid,rscorebuild);
                return rscorebuild.totalpopular();

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_All_Max_Popular://合建建筑人气上限总
            {
                HelloKittyMsgData::MaxUnityBuild rscorebuild;
                rscorebuild.set_maxscore(0);
                rscorebuild.set_buildid(0);
                rscorebuild.set_buildlv(0); 
                rscorebuild.set_otherid(0);
                rscorebuild.set_totalpopular(0); 
                rscorebuild.set_totalmaxpopular(0);
                RedisMgr::getMe().get_unitybuildrankdata(m_rUser.charid,rscorebuild);
                return rscorebuild.totalmaxpopular();

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Park_level://乐园等级
            {
                return m_rUser.charbase.level;
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Friends_Number://好友总数
            {
                return m_rUser.m_friend.GetFriendSize();
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Fans_Number://粉丝总数
            {
                return m_rUser.m_friend.GetFanSize();
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Pal_Number://小伙伴总数
            {
                //TODO
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Pal_Attributes://小伙伴总属性
            {
                //TODO
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Pal_Level://小伙伴等级
            {
                //TODO
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Pal_Grow://小伙伴成长值
            {
                //TODO
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Pal_Battle_Power://小伙伴战斗力
            {
                //TODO
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_4Star_Pal://四星小伙伴
            {
                //TODO
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Charm_List://魅力榜
            {
                zMemDB*  redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Charisma);
                if(redishandle)
                    return redishandle->getRevRank("charismarank","charisma",m_rUser.charid);
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Contribution_List://贡献榜
            {
                zMemDB*  redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Contribute);
                if(redishandle)
                    return redishandle->getRevRank("contributerank","contribute",m_rUser.charid);


            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Popular_list://人气榜
            {
                zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Popular_Now);
                if(redishandle) 
                    return redishandle->getRevRank("popularnowrank","popularnow",m_rUser.charid);
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Level_List://等级榜
            {
                zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_VerifyLevel);
                if(redishandle)
                    return redishandle->getRevRank("verifyrank","verifylevel",m_rUser.charid);
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Unite_Building_List://合建榜
            {
                zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_UnityBuild);
                if(redishandle)
                    return redishandle->getRevRank("unitbuildrank","totalscore",m_rUser.charid);
            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Family_Level://家族等级
            {
                QWORD familyID = RecordFamily::getMe().getFamilyID(m_rUser.charid);
                if(familyID != 0)
                {
                    CMD::RECORD::FamilyBase base;
                    if(RecordFamily::getMe().readFamily(familyID,base))
                    {
                        return base.m_level;
                    }
                }

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Family_Experience://家族经验
            {
                QWORD familyID = RecordFamily::getMe().getFamilyID(m_rUser.charid);
                if(familyID != 0)
                {
                    CMD::RECORD::FamilyBase base;
                    if(RecordFamily::getMe().readFamily(familyID,base))
                    {
                        return base.m_score;
                    }
                }

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Charm_All://魅力总值
            {
                return m_rUser.m_store_house.getAttr(HelloKittyMsgData::Attr_Charisma); 

            }
            break;
        case HelloKittyMsgData::ActiveConditionType_Contribution_All://贡献总值
            {
                return m_rUser.m_store_house.getAttr(HelloKittyMsgData::Attr_Contribute);

            }
            break;
        default:
            break;
    }
    return param;
}

void PlayerActiveManager::update(const std::set<DWORD> &updateSet)
{
    HelloKittyMsgData::Ackgetplayeractivelist ack;
    ack.set_init(false);
    for(auto it = updateSet.begin();it != updateSet.end() ;it++)
    {
        HelloKittyMsgData::PlayerActiveForClient *pcli = ack.add_info();
        if(pcli ==NULL)
        {
            continue;
        }
        HelloKittyMsgData::PlayerActiveSave * psave = getSaveInfo(*it);
        HelloKittyMsgData::PlayerActive* pbase = pcli->mutable_baseinfo();
        const HelloKittyMsgData::PlayerActive * pinfo = PlayerActiveConfig::getMe().getinfo(*it);
        if(pinfo)
        {
            *pbase = *pinfo;
        }
        else
        {
            pbase->set_f_type(HelloKittyMsgData::AT_None);
            pbase->set_f_subtype(HelloKittyMsgData::AST_None);
            pbase->set_f_id(*it);
            pbase->set_f_begintime(0);
            pbase->set_f_endtime(0);
            pbase->set_f_condition(HelloKittyMsgData::ActiveConditionType_None);
            pbase->set_f_conditionparam(0);
            pbase->set_f_preactive(0);
            pbase->set_f_award(0);
            pbase->set_f_title(0);
            pbase->set_f_desc("");
            pbase->set_f_open(true);
            pbase->set_f_platemsg("");
            pbase->set_f_rewardmaxcnt(0);
            pbase->set_f_rewardcurcnt(0);
        }
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(*it);
        if(redis && pinfo && pinfo->f_type() == HelloKittyMsgData::AT_TimeLimit)
        {
            DWORD num = redis->getInt("limitactive",*it,"rewardlimitmax");
            if(num)
            {
                pbase->set_f_rewardmaxcnt(num);
                num = redis->getInt("limitactive",*it,"rewardlimitcur");
                pbase->set_f_rewardcurcnt(num);
            }
        }

        HelloKittyMsgData::ActiveState activeState = HelloKittyMsgData::ActiveState_Del;
        active_state state = PlayerActiveConfig::getMe().getState(*it);
        if(state == state_wait)
        {
            activeState = HelloKittyMsgData::ActiveState_NoBegin;
        }
        else if(state == state_run)
        {
            activeState = HelloKittyMsgData::ActiveState_Start;
        }
        else if(state == state_finish)
        {
            activeState = HelloKittyMsgData::ActiveState_Finish;
        }
        pcli->set_getaward(psave ? psave->getaward() : false);
        pcli->set_state(psave ? activeState : HelloKittyMsgData::ActiveState_Del);
        pcli->set_currentparam(psave ? psave->currentparam() : 0);
    }
    if(updateSet.empty())
    {
        return;
    }
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());
}



void PlayerActiveManager::doaction(HelloKittyMsgData::ActiveConditionType type,DWORD param,DWORD activeID,DWORD subCondition,DWORD subType)
{
    std::set<DWORD> updateSet;

    const std::set<DWORD> *pconf = PlayerActiveConfig::getMe().getListen(type);
    if(pconf == NULL || pconf->empty())
        return ;
    if(PlayerActiveConfig::getMe().isSaveType(type))
    {
        for(auto it = pconf->begin();it != pconf->end();it++)
        {
            if(activeID && activeID != *it)
            {
                continue;
            }
            const HelloKittyMsgData::PlayerActive * pinfo = PlayerActiveConfig::getMe().getinfo(*it);
            if(pinfo == NULL)
            {
                continue;
            }
            if(subType && subType != (DWORD)pinfo->f_subtype())
            {
                continue;
            }
            if(pinfo->f_subcondition() && pinfo->f_subcondition() != subCondition)
            {
                continue;
            }
            HelloKittyMsgData::PlayerActiveSave *psave =  getSaveInfo(*it);
            if(psave)
            {
                psave->set_currentparam(psave->currentparam() + param);
            }
            else
            {   
                HelloKittyMsgData::PlayerActiveSave rSave;
                rSave.set_f_id(*it);
                rSave.set_getaward(false); 
                rSave.set_lastchecktimer(0);
                rSave.set_currentparam(param);
                m_mapActive[*it] = rSave;
            }
            updateSet.insert(*it);
        }
    }
    update(updateSet);
    /*
       case HelloKittyMsgData::ActiveConditionType_Composite_Number://合成总数,1,合成动作：除了庄园，其他11建筑的合成//done
       case HelloKittyMsgData::ActiveConditionType_Fruit_Production_Number://水果生产总数,1,庄园生产//done
       case HelloKittyMsgData::ActiveConditionType_Family_Trust://家族委托//done 
       case HelloKittyMsgData::ActiveConditionType_Family_Compete://家族比拼//'todo
       case HelloKittyMsgData::ActiveConditionType_Family_Order://家族订单 //todo
       case HelloKittyMsgData::ActiveConditionType_Regular_Order_Number://普通订单数//done
       case HelloKittyMsgData::ActiveConditionType_Helicopter_Order_Number://直升机订单数//todo
       case HelloKittyMsgData::ActiveConditionType_Family_Order_Number://家族订单数//done
       case HelloKittyMsgData::ActiveConditionType_Airship_Order_Number://飞艇订单数//todo
       case HelloKittyMsgData::ActiveConditionType_Train_Order_Number://火车订单数done
       case HelloKittyMsgData::ActiveConditionType_Expand_Materials://扩地材料done
       case HelloKittyMsgData::ActiveConditionType_Warehouse_Material://扩仓材料done
       case HelloKittyMsgData::ActiveConditionType_Activate_Material://激活材料done
       case HelloKittyMsgData::ActiveConditionType_Star_Upgrade_aterial://升星材料done
       case HelloKittyMsgData::ActiveConditionType_Pal_Materials://小伙伴材料done
       case HelloKittyMsgData::ActiveConditionType_Purchase_All://采购总量done
       case HelloKittyMsgData::ActiveConditionType_Coin_Total://金币总数done
       case HelloKittyMsgData::ActiveConditionType_Diamond_Total://钻石总数done
       case HelloKittyMsgData::ActiveConditionType_Token_Total://代币总数done
       case HelloKittyMsgData::ActiveConditionType_Ticket_Total://点券总数done
       case HelloKittyMsgData::ActiveConditionType_Friendship_Point_Total://友情点总数//done
       case HelloKittyMsgData::ActiveConditionType_Sushi_Game://寿司游戏,1,完成游戏的次数 //done
       case HelloKittyMsgData::ActiveConditionType_Constellation_Lines://星座连线,1,完成游戏的次数 //done
       case HelloKittyMsgData::ActiveConditionType_Gashapon_Diamond_Consumption_Total://扭蛋总钻石消耗,1//done
       case HelloKittyMsgData::ActiveConditionType_Gashapon_Friendship_Point_Consumption_Total://扭蛋总友情点消耗,1, //done
       case HelloKittyMsgData::ActiveConditionType_Divination_Number://占卜总数,1 //done
       case HelloKittyMsgData::ActiveConditionType_Bidding_Join_Number://竞拍参与次数,1 //done
       case HelloKittyMsgData::ActiveConditionType_Auction_Success_Number://竞拍成功次数,1//done
       case HelloKittyMsgData::ActiveConditionType_Coin_Consumption_Total://费总金币,1 done
       case HelloKittyMsgData::ActiveConditionType_Diamond_Consumption_Total://消费总钻石,1 done
       case HelloKittyMsgData::ActiveConditionType_Employment_Number_All://雇用总次数,1done
       case HelloKittyMsgData::ActiveConditionType_Employment_Consumption_All://雇用总花费,1done
       case HelloKittyMsgData::ActiveConditionType_Find_Number_All://寻物总次数,1done
       case HelloKittyMsgData::ActiveConditionType_Hangup_Number_All://挂机总次数,1done
       case HelloKittyMsgData::ActiveConditionType_Give_Gift_Worth://送礼总价值,2,物品总价值：限于代币 //done
       case HelloKittyMsgData::ActiveConditionType_Accept_Gift_Worth://收礼总价值,2,物品总价值：限于代币 //done
       case HelloKittyMsgData::ActiveConditionType_Give_Gift_Number://送礼总数量,2 //done
       case HelloKittyMsgData::ActiveConditionType_Accept_Gift_Number://收礼总数量,2 //done
       */

}
