#include "SceneUser.h"
#include "divine.pb.h"
#include "tbx.h"
#include "key.h"
#include "common.pb.h"
#include "zMemDB.h"
#include "zMemDBPool.h"
#include "SceneCommand.h"
#include <string.h>
#include "SceneCommand.h"
#include "SceneTaskManager.h"
#include "CharBase.h"
#include "dataManager.h"
#include "RedisMgr.h"


bool SceneUser::reqRank(const HelloKittyMsgData::ReqRank *cmd)
{
    switch(cmd->id())
    {
        case HelloKittyMsgData::RT_Level:
            {
                return levelRank(cmd);
            }
            break;
        case HelloKittyMsgData::RT_Sushi:
            {
                return suShiRank();
            }
            break;
        case HelloKittyMsgData::RT_Star:
            {
                return starRank();
            }
            break;
        case HelloKittyMsgData::RT_Charisma:
            {
                return charismaRank(cmd);
            }
            break;
        case HelloKittyMsgData::RT_Contribute:
            {
                return contributeRank(cmd);
            }
            break;
        case HelloKittyMsgData::RT_UnityBuild:
            {
                return unitybuildRank(cmd);
            }
            break;
        case HelloKittyMsgData::RT_Popular_Now:
            {
                return popularNowRank(cmd);
            }
            break;
        default:
            {
                Fir::logger->debug("[排行榜] 请求类型出错(%lu,%u)",charid,cmd->id());
            }
    }
    return false;
}

bool SceneUser::levelRank(const HelloKittyMsgData::ReqRank *reqRank)
{
    bool ret = false;
    do
    {
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle((reqRank->area() ? charbase.areaType : HelloKittyMsgData::RT_Level));
        if(!handle)
        {
            break;
        }

        char areaKey[100] = {0};
        snprintf(areaKey,sizeof(areaKey),"arealevel_%u",charbase.areaType);
        std::set<RankData> rankSet;
        handle->getRevSortSet("rank",reqRank->area() ? areaKey : "level",rankSet);

        HelloKittyMsgData::AckRank ackRank;
        ackRank.set_id(reqRank->id());
        ackRank.set_area(reqRank->area());
        ackRank.set_rank(handle->getRevRank("rank",reqRank->area() ? areaKey : "level",charid));

        for(auto iter = rankSet.begin();iter != rankSet.end();++iter)
        {
            const RankData &rankData = *iter;
            QWORD charID = rankData.charID;
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(charID);
            if(!handleTemp)
            {
                continue;
            }
            HelloKittyMsgData::RankInfo *temp = ackRank.add_rankinfo();
            if(!temp)
            {
                continue;
            }
            HelloKittyMsgData::playerShowbase &rbase = *(temp->mutable_head());
            SceneUser::getplayershowbase(charID,rbase);
            temp->set_value(rankData.value);
            temp->set_rank(handle->getRevRank("rank",reqRank->area() ? areaKey : "level",charID));
            temp->set_city(handleTemp->get("rolebaseinfo",rankData.charID,"cityname"));
            temp->set_born(handleTemp->get("rolebaseinfo",rankData.charID,"birthday"));

        }

        std::string msg;
        encodeMessage(&ackRank,msg);
        sendCmdToMe(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::suShiRank()
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Sushi);
    if(!handle)
    {
        return false;
    }

    std::set<RankData> rankSet;
    handle->getRevSortSet("sushirank","sushihistory",rankSet);

    HelloKittyMsgData::AckRank ackRank;
    ackRank.set_id(HelloKittyMsgData::RT_Sushi);
    ackRank.set_area(false);
    ackRank.set_rank(handle->getRevRank("sushirank","sushihistory",charid));
    HelloKittyMsgData::SuShiData *suShi = charbin.mutable_dailydata()->mutable_sushidata();
    ackRank.set_value(suShi->history());

    for(auto iter = rankSet.begin();iter != rankSet.end();++iter)
    {
        const RankData &rankData = *iter;
        QWORD charID = rankData.charID;
        zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(charID);
        if(!handleTemp)
        {
            continue;
        }
        HelloKittyMsgData::RankInfo *temp = ackRank.add_rankinfo();
        if(!temp)
        {
            continue;
        }
        HelloKittyMsgData::playerShowbase &rbase = *(temp->mutable_head());
        SceneUser::getplayershowbase(charID,rbase);
        temp->set_value(rankData.value);
        temp->set_city(handleTemp->get("rolebaseinfo",rankData.charID,"cityname"));
        temp->set_born(handleTemp->get("rolebaseinfo",rankData.charID,"birthday"));
        temp->set_rank(handle->getRevRank("sushirank","sushihistory",charID));
    }

    std::string ret;
    encodeMessage(&ackRank,ret);
    return sendCmdToMe(ret.c_str(),ret.size());
}

bool SceneUser::starRank()
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Star);
    if(!handle)
    {
        return false;
    }

    std::set<RankData> rankSet;
    handle->getSortSet("starrank","starhistory",rankSet,0,99);

    HelloKittyMsgData::AckRank ackRank;
    ackRank.set_id(HelloKittyMsgData::RT_Star);
    ackRank.set_area(false);
    ackRank.set_rank(handle->getRank("starrank","starhistory",charid));

    for(auto iter = rankSet.rbegin();iter != rankSet.rend();++iter)
    {
        const RankData &rankData = *iter;
        QWORD charID = rankData.charID;
        HelloKittyMsgData::RankInfo *temp = ackRank.add_rankinfo();
        if(!temp)
        {
            continue;
        }
        zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(charID);
        if(!handleTemp)
        {
            continue;
        }
        HelloKittyMsgData::playerShowbase &rbase = *(temp->mutable_head());
        SceneUser::getplayershowbase(charID,rbase);
        temp->set_value(rankData.value);
        temp->set_city(handleTemp->get("rolebaseinfo",rankData.charID,"cityname"));
        temp->set_born(handleTemp->get("rolebaseinfo",rankData.charID,"birthday"));
        temp->set_rank(handle->getRank("starrank","starhistory",charID));

    }

    std::string ret;
    encodeMessage(&ackRank,ret);
    return sendCmdToMe(ret.c_str(),ret.size());
}

bool SceneUser::charismaRank(const HelloKittyMsgData::ReqRank *reqRank)
{
    bool ret = false;
    do
    {
        std::string rankKey = getTimeRankKey(reqRank->time());
        rankKey = rankKey.empty() ? "charisma" : rankKey;

        std::map<QWORD,DWORD> rankHotSet;
        std::map<QWORD,DWORD> rankVerifySet;
        if(!(getHotSet(rankHotSet) && getVerifySet(rankVerifySet)))
        {
            break;
        }

        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(reqRank->id());
        if(!handle)
        {
            break;
        }

        std::set<RankData> rankSet;
        handle->getRevSortSet("charismarank",rankKey.c_str(),rankSet);

        HelloKittyMsgData::AckRank ackRank;
        ackRank.set_id(reqRank->id());
        ackRank.set_area(false);
        ackRank.set_rank(handle->getRevRank("charismarank",rankKey.c_str(),charid));
        ackRank.set_lastweekrank(charbin.charismalastweekrank());
        ackRank.set_lastmonthrank(charbin.charismalastmonthrank());

        for(auto iter = rankSet.begin();iter != rankSet.end();++iter)
        {
            const RankData &rankData = *iter;
            QWORD charID = rankData.charID;
            zMemDB *handleTemp = zMemDBPool::getMe().getMemDBHandle(charID);
            if(!handleTemp)
            {
                continue;
            }
            HelloKittyMsgData::RankInfo *temp = ackRank.add_rankinfo();
            if(!temp)
            {
                continue;
            }
            HelloKittyMsgData::playerShowbase &rbase = *(temp->mutable_head());
            SceneUser::getplayershowbase(charID,rbase);

            temp->set_value(rankData.value);
            temp->set_rank(handle->getRevRank("charismarank",rankKey.c_str(),charID));
            temp->set_total(handleTemp->getInt("rolebaseinfo",charID,"charisma"));
            temp->set_week(handleTemp->getInt("rolebaseinfo",charID,"charismaweek"));
            temp->set_month(handleTemp->getInt("rolebaseinfo",charID,"charismamonth"));
            temp->set_isverify(rankVerifySet.find(charID) != rankVerifySet.end());
            temp->set_ishot(rankHotSet.find(charID) != rankHotSet.end());
            temp->set_city(handleTemp->get("rolebaseinfo",rankData.charID,"cityname"));
            temp->set_born(handleTemp->get("rolebaseinfo",rankData.charID,"birthday"));



        }

        std::string msg;
        encodeMessage(&ackRank,msg);
        sendCmdToMe(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::contributeRank(const HelloKittyMsgData::ReqRank *reqRank)
{
    bool ret = false;
    do
    {
        std::string rankKey = getTimeRankKey(reqRank->time());;
        rankKey = rankKey.empty() ? "contribute" : rankKey;

        std::map<QWORD,DWORD> rankHotSet;
        std::map<QWORD,DWORD> rankVerifySet;
        if(!(getHotSet(rankHotSet) && getVerifySet(rankVerifySet)))
        {
            break;
        }

        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(reqRank->id());
        if(!handle)
        {
            break;
        }
        std::set<RankData> rankSet;
        handle->getRevSortSet("contributerank",rankKey.c_str(),rankSet);

        HelloKittyMsgData::AckRank ackRank;
        ackRank.set_id(reqRank->id());
        ackRank.set_area(false);
        ackRank.set_rank(handle->getRevRank("contributerank",rankKey.c_str(),charid));
        ackRank.set_isverify(rankVerifySet.find(charid) != rankVerifySet.end());
        ackRank.set_ishot(rankHotSet.find(charid) != rankHotSet.end());
        ackRank.set_lastweekrank(charbin.contributelastweekrank());
        ackRank.set_lastmonthrank(charbin.contributelastmonthrank());


        for(auto iter = rankSet.begin();iter != rankSet.end();++iter)
        {
            const RankData &rankData = *iter;
            QWORD charID = rankData.charID;
            zMemDB *handleTemp = zMemDBPool::getMe().getMemDBHandle(charID);
            if(!handleTemp)
            {
                continue;
            }
            HelloKittyMsgData::RankInfo *temp = ackRank.add_rankinfo();
            if(!temp)
            {
                continue;
            }
            HelloKittyMsgData::playerShowbase &rbase = *(temp->mutable_head());
            SceneUser::getplayershowbase(charID,rbase);

            temp->set_value(rankData.value);
            temp->set_rank(handle->getRevRank("contributerank",rankKey.c_str(),charID));
            temp->set_total(handleTemp->getInt("rolebaseinfo",charID,"contribute"));
            temp->set_week(handleTemp->getInt("rolebaseinfo",charID,"contributeweek"));
            temp->set_month(handleTemp->getInt("rolebaseinfo",charID,"contributemonth"));
            temp->set_isverify(rankVerifySet.find(charID) != rankVerifySet.end());
            temp->set_ishot(rankHotSet.find(charID) != rankHotSet.end());
            temp->set_city(handleTemp->get("rolebaseinfo",rankData.charID,"cityname"));
            temp->set_born(handleTemp->get("rolebaseinfo",rankData.charID,"birthday"));



        }

        std::string msg;
        encodeMessage(&ackRank,msg);
        sendCmdToMe(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::popularNowRank(const HelloKittyMsgData::ReqRank *reqRank)
{
    bool ret = false;
    do
    {
        std::string rankKey = getTimeRankKey(reqRank->time());;
        rankKey = rankKey.empty() ? "popularnow" : rankKey;

        std::map<QWORD,DWORD> rankHotSet;
        std::map<QWORD,DWORD> rankVerifySet;
        if(!(getHotSet(rankHotSet) && getVerifySet(rankVerifySet)))
        {
            break;
        }

        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(reqRank->id());
        if(!handle)
        {
            break;
        }
        std::set<RankData> rankSet;
        handle->getRevSortSet("popularnowrank",rankKey.c_str(),rankSet);

        HelloKittyMsgData::AckRank ackRank;
        ackRank.set_id(reqRank->id());
        ackRank.set_area(false);
        ackRank.set_time(reqRank->time());
        ackRank.set_rank(handle->getRevRank("popularnowrank",rankKey.c_str(),charid));
        ackRank.set_isverify(rankVerifySet.find(charid) != rankVerifySet.end());
        ackRank.set_ishot(rankHotSet.find(charid) != rankHotSet.end());
        ackRank.set_lastweekrank(charbin.popularlastweekrank());
        ackRank.set_lastmonthrank(charbin.popularlastmonthrank());


        for(auto iter = rankSet.begin();iter != rankSet.end();++iter)
        {
            const RankData &rankData = *iter;
            QWORD charID = rankData.charID;
            zMemDB *handleTemp = zMemDBPool::getMe().getMemDBHandle(charID);
            if(!handleTemp)
            {
                continue;
            }
            HelloKittyMsgData::RankInfo *temp = ackRank.add_rankinfo();
            if(!temp)
            {
                continue;
            }
            HelloKittyMsgData::playerShowbase &rbase = *(temp->mutable_head());
            SceneUser::getplayershowbase(charID,rbase);

            temp->set_value(rankData.value);
            temp->set_rank(handle->getRevRank("popularnowrank",rankKey.c_str(),charID));
            temp->set_total(handleTemp->getInt("rolebaseinfo",charID,"popularnow"));
            temp->set_week(handleTemp->getInt("rolebaseinfo",charID,"popularnowweek"));
            temp->set_month(handleTemp->getInt("rolebaseinfo",charID,"popularnowmonth"));
            temp->set_isverify(rankVerifySet.find(charID) != rankVerifySet.end());
            temp->set_ishot(rankHotSet.find(charID) != rankHotSet.end());
            temp->set_city(handleTemp->get("rolebaseinfo",rankData.charID,"cityname"));
            temp->set_born(handleTemp->get("rolebaseinfo",rankData.charID,"birthday"));


        }

        std::string msg;
        encodeMessage(&ackRank,msg);
        sendCmdToMe(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}


bool SceneUser::unitybuildRank(const HelloKittyMsgData::ReqRank *reqRank)
{
    do{
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_UnityBuild);
        if(redishandle == NULL)
            break;
        std::set<RankData> rankSet;
        redishandle->getRevSortSet("unitbuildrank","totalscore",rankSet,DWORD(0),DWORD(30));
        HelloKittyMsgData::AckUnitbuildRank ack;
        ack.set_selfrank(redishandle->getRevRank("unitbuildrank","totalscore",charid));
        for(auto iter = rankSet.begin();iter != rankSet.end();iter++)
        {
            const RankData& rRank = *iter; 

            HelloKittyMsgData::MaxUnityBuild rscorebuild;
            if(!RedisMgr::getMe().get_unitybuildrankdata(rRank.charID,rscorebuild))
                continue;

            HelloKittyMsgData::UnitbuildRankInfo *pinfo = ack.add_rankinfo();
            if(pinfo ==NULL)
                continue;
            HelloKittyMsgData::playerShowbase *pleft = pinfo->mutable_headleft();
            if(pleft)
            {
                SceneUser::getplayershowbase(rRank.charID,*pleft);

            }
            zMemDB *handleleft = zMemDBPool::getMe().getMemDBHandle(rRank.charID);
            if(handleleft)
            {
                pinfo->set_leftcity(handleleft->get("rolebaseinfo",rRank.charID,"cityname"));
                pinfo->set_leftborn(handleleft->get("rolebaseinfo",rRank.charID,"birthday"));
            }
            pinfo->set_rank(redishandle->getRevRank("unitbuildrank","totalscore",rRank.charID));
            pinfo->set_lefttotalpopular(rscorebuild.totalpopular());
            pinfo->set_lefttotalmaxpopular(rscorebuild.totalmaxpopular());
            pinfo->set_leftbuildid(rscorebuild.buildid());
            pinfo->set_leftbuildlv(rscorebuild.buildlv());
            HelloKittyMsgData::playerShowbase *pright = pinfo->mutable_headright();
            if(pright)
            {
                SceneUser::getplayershowbase(rscorebuild.otherid(),*pright);
            }
            zMemDB *handleright = zMemDBPool::getMe().getMemDBHandle(rscorebuild.otherid());
            if(handleright)
            {
                pinfo->set_rightcity(handleright->get("rolebaseinfo",rscorebuild.otherid(),"cityname"));
                pinfo->set_rightborn(handleright->get("rolebaseinfo",rscorebuild.otherid(),"birthday"));
            }

        }
        std::string msg;
        encodeMessage(&ack,msg);
        sendCmdToMe(msg.c_str(),msg.size());

    }while(0);
    return true;
}

bool SceneUser::logLevelRank()
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Level);
    if(!handle)
    {
        return false;
    }

    std::set<RankData> rankSet;
    handle->getRevSortSet("rank","level",rankSet);

    Fir::logger->debug("[排行榜] 全球等级排行榜日志begin");

    for(auto iter = rankSet.begin();iter != rankSet.end();++iter)
    {
        const RankData &rankRank = *iter;
        DWORD rank = handle->getRevRank("rank","level",rankRank.charID);
        Fir::logger->debug("[排行榜] 全球等级排行榜日志(%lu,%u,%u)",rankRank.charID,rankRank.value,rank);
    }
    Fir::logger->debug("[排行榜] 全球等级排行榜日志end");
    return true;
}

bool SceneUser::logAllAreaLevelRank()
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Level);
    if(!handle)
    {
        return false;
    }

    std::set<QWORD> earaSet;
    handle->getSet("rank",0,"areaset",earaSet);

    Fir::logger->debug("[排行榜] 区域等级排行榜日志begin");
    for(auto iter = earaSet.begin();iter != earaSet.end();++iter)
    {
        HelloKittyMsgData::AreaType areaType = (HelloKittyMsgData::AreaType)(*iter);
        logAreaLevelRank(areaType);
    }
    Fir::logger->debug("[排行榜] 区域等级排行榜日志end");
    return true;
}

bool SceneUser::logAreaLevelRank(const HelloKittyMsgData::AreaType &areaType)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(areaType);
    if(!handle)
    {
        return false;
    }

    DWORD areaTypeInt = areaType;
    std::set<QWORD> earaSet;
    handle->getSet("rank",0,"areaset",earaSet);
    if(earaSet.find(areaTypeInt) == earaSet.end())
    {
        Fir::logger->debug("[排行榜] 区域类型错误(%u)",areaTypeInt);
        return true;
    }

    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"arealevel_%u",areaTypeInt);


    std::set<RankData> rankSet;
    handle->getRevSortSet("rank",temp,rankSet);

    Fir::logger->debug("[排行榜] 区域(%u)等级排行榜日志begin",areaTypeInt);
    for(auto it = rankSet.begin();it != rankSet.end();++it)
    {
        const RankData &rankData = *it;
        DWORD rank = handle->getRevRank("rank",temp,rankData.charID);
        Fir::logger->debug("[排行榜] 区域(%u)等级排行榜日志(%lu,%u,%u)",areaTypeInt,rankData.charID,rankData.value,rank);
    }
    Fir::logger->debug("[排行榜] 区域(%u)等级排行榜日志end",areaTypeInt);
    return true;
}


bool SceneUser::suShiRankReward()
{
    bool ret = false;
    zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle();
    if(!handleTemp)
    {
        return ret;
    }
    DWORD lock = handleTemp->isLock("sushirankreward",1,"locktime");
    if(lock)
    {
        return ret;
    }
    handleTemp->getLock("sushirankreward",1,"locktime",1);
    do
    {
        zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Sushi);
        if(!handle)
        {
            break;
        }
        const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_SuShi_Rank_Reward);
        if(!emailConf)
        {
            break;
        }
        std::vector<HelloKittyMsgData::ReplaceWord> argVec;
        std::set<RankData> rankSet;
        handle->getRevSortSet("sushirank","sushihistory",rankSet);
        for(auto iter = rankSet.begin();iter != rankSet.end();++iter)
        {
            const RankData &rankData = *iter;
            QWORD charID = rankData.charID;
            DWORD rank = handle->getRevRank("rank","sushihistory",charID);
            QWORD key = pb::Conf_t_SushiRankReward::getKeyByRank(rank);
            const pb::Conf_t_SushiRankReward *rankReward = tbx::SushiRankReward().get_base(key);
            if(!rankReward)
            {
                continue;
            }
            bool ret = EmailManager::sendEmailBySys(charID,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,rankReward->getRewardMap());
            Fir::logger->debug("[寿司排行奖励] %s (%u,%lu)",ret ? "成功" : "失败",rank,charID);
        }
        ret = true;
    }while(false);
    handleTemp->delLock("sushirankreward",1,"rewardtime");
    return ret;
}

bool SceneUser::StarRankReward()
{
    bool ret = false;
    zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle();
    if(!handleTemp)
    {
        return ret;
    }
    DWORD lock = handleTemp->isLock("starrankreward",1,"locktime");
    if(lock)
    {
        return ret;
    }
    handleTemp->getLock("starrankreward",1,"locktime",1);
    do
    {
        zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Star);
        if(!handle)
        {
            break;
        }
        const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Star_Rank_Reward);
        if(!emailConf)
        {
            break;
        }
        std::vector<HelloKittyMsgData::ReplaceWord> argVec;
        std::set<RankData> rankSet;
        handle->getSortSet("starrank","starhistory",rankSet,0,99);
        for(auto iter = rankSet.rbegin();iter != rankSet.rend();++iter)
        {
            const RankData &rankData = *iter;
            QWORD charID = rankData.charID;
            DWORD rank = handle->getRank("starrank","starhistory",charID);
            QWORD key = pb::Conf_t_SushiRankReward::getKeyByRank(rank);
            const pb::Conf_t_SushiRankReward *rankReward = tbx::SushiRankReward().get_base(key);
            if(!rankReward)
            {
                continue;
            }
            bool ret = EmailManager::sendEmailBySys(charID,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,rankReward->getRewardMap());
            Fir::logger->debug("[星座排行奖励] %s (%u,%lu)",ret ? "成功" : "失败",rank,charID);
        }
        ret = true;
    }while(false);
    handleTemp->delLock("starrankreward",1,"rewardtime");
    return ret;
}


bool SceneUser::getHotSet(std::map<QWORD,DWORD> &rankHotSet)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Charisma);
    if(!handle)
    {
        return false;
    }
    handle->getRevSortSet("charismarank","charisma",rankHotSet,0,9);
    return true;
}

bool SceneUser::getVerifySet(std::map<QWORD,DWORD> &rankVerifySet)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_VerifyLevel);
    if(!handle)
    {
        return false;
    }
    handle->getRevSortSet("verifyrank","verify",rankVerifySet,0,9);
    return true;
}

std::string SceneUser::getTimeRankKey(const HelloKittyMsgData::TimeRankType &timeRank)
{
    std::string rankKey;
    switch(timeRank)
    {
        case HelloKittyMsgData::TRT_Week:
            {
                rankKey = "week";
            }
            break;
        case HelloKittyMsgData::TRT_Month:
            {
                rankKey = "month";
            }
            break;
        default:
            {
            }
            break;
    }
    return rankKey;
}

std::string SceneUser::getSexRankKey(const HelloKittyMsgData::SexRankType &sexRank)
{
    std::string rankKey;
    switch(sexRank)
    {
        case HelloKittyMsgData::SRT_Man:
            {
                rankKey = "man";
            }
            break;
        case HelloKittyMsgData::SRT_Female:
            {
                rankKey = "female";
            }
            break;
        default:
            {
            }
            break;
    }
    return rankKey;
}




