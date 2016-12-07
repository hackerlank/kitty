#include "ActiveManager.h"
#include "TimeTick.h"
#include "RecordTask.h"
#include "zSocket.h"
#include "RecordUser.h"
#include "dataManager.h"
#include "tbx.h"
#include "RecordServer.h"
#include "RecordTaskManager.h"
#include "active.pb.h"
#include <algorithm>

ActiveManager::ActiveManager()
{
}

ActiveManager::~ActiveManager()
{
}
void ActiveManager::checkChange(bool bSendMsg)
{
    if(!RecordService::getMe().hasDBtable("active"))
    {
        return ;
    }
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return ;
    }
    std::set<QWORD> activeIDtep;
    handle->getSet("active",0,"active",activeIDtep);
    std::set<DWORD> activeIDset(activeIDtep.begin(),activeIDtep.end());

    std::set<DWORD> NowOpen;
    getActiveListByCur(NowOpen);
    if(activeIDset == NowOpen)
        return ;
    std::set<DWORD> OpenId;
    std::set_difference(NowOpen.begin(),NowOpen.end(),activeIDset.begin(),activeIDset.end(),std::inserter(OpenId,OpenId.begin()));
    std::set<DWORD> CloseId;
    std::set_difference(activeIDset.begin(),activeIDset.end(),NowOpen.begin(),NowOpen.end(),std::inserter(CloseId,CloseId.begin()));
    for(auto it = OpenId.begin();it != OpenId.end();it++)
    {
        handle->setSet("active",0,"active",*it);
    }
    for(auto it = CloseId.begin();it != CloseId.end();it++)
    {
        handle->delSet("active",0,"active",*it);
    }
    if(bSendMsg == false)
        return;
    //关闭列表
    HelloKittyMsgData::AckUpdateActiveInfo ack;
    for(auto it = OpenId.begin();it != OpenId.end();it++)
    {
        HelloKittyMsgData::UpdateActiveInfo *pInfo = ack.add_updateactiveinfo();
        if(pInfo)
        {
            pInfo->set_activeid(*it);
            pInfo->set_status(HelloKittyMsgData::Active_Close);
        }
    }
    //开启列表
    for(auto it = CloseId.begin();it != CloseId.end();it++)
    {
        HelloKittyMsgData::UpdateActiveInfo *pInfo = ack.add_updateactiveinfo();
        if(pInfo)
        {
            pInfo->set_activeid(*it);
            pInfo->set_status(HelloKittyMsgData::Active_Open);
        }
    }
    using namespace CMD::RECORD;
    BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
    t_BroadCastMsg *ptCmd = (t_BroadCastMsg*)(pBuffer);
    constructInPlace(ptCmd);
    ptCmd->size = 0;
    std::string strret;
    encodeMessage(&ack,strret);
    ptCmd->size = strret.size();
    memcpy(ptCmd->data,strret.c_str(),ptCmd->size);
    //通知网关服务器
    std::string ret;
    encodeMessage(ptCmd,sizeof(t_BroadCastMsg) + ptCmd->size,ret);
    RecordTaskManager::getMe().broadcastByType(GATEWAYSERVER,ret.c_str(),ret.size());

}

bool ActiveManager::init(bool bReLoad)//初始化或则重新加载
{
    if(!RecordService::getMe().hasDBtable("active"))
    {
        return false;
    }
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    checkChange(false);
    if(!bReLoad)
        return  true;
    HelloKittyMsgData::AckActiveInfo rInfo;
    const std::unordered_map<unsigned int, const pb::Conf_t_Active *> &tbxMap = tbx::Active().getTbxMap();
    for(auto it = tbxMap.begin();it != tbxMap.end();it++)
    {
        const pb::Conf_t_Active *pConfig = it->second;
        if(pConfig == NULL)
            continue;
        if(pConfig->active->type() >= HelloKittyMsgData::Active_Type_MAX)
            continue;
        if(pConfig->active->ispublic() >= HelloKittyMsgData::ActivePublic_MAX)
            continue;
        if(pConfig->active->showtype() >= HelloKittyMsgData::ActiveShowType_MAX)
            continue;
        if(pConfig->active->activeopentive() >= HelloKittyMsgData::ActiveOpenType_MAX)
            continue;
        HelloKittyMsgData::ActiveInfo *pInfo = rInfo.add_activeinfo();
        if(pInfo == NULL)
            continue;

        pInfo->set_activeid(pConfig->active->id());
        bool bOpen = handle->checkSet("active",0,"active",pConfig->active->id());
        pInfo->set_status(bOpen ? HelloKittyMsgData::Active_Open : HelloKittyMsgData::Active_Close);
        pInfo->set_type(static_cast<HelloKittyMsgData::ActiveType>(pConfig->active->type()));
        pInfo->set_publictype(static_cast<HelloKittyMsgData::ActivePublic>(pConfig->active->ispublic()));
        pInfo->set_showtype(static_cast<HelloKittyMsgData::ActiveShowType>(pConfig->active->showtype()));
        pInfo->set_order(pConfig->active->order());
        pInfo->set_name(pConfig->active->name());
        pInfo->set_starttimer(pConfig->active->starttime());
        pInfo->set_endtimer(pConfig->active->endtime());
        pInfo->set_opentype(static_cast<HelloKittyMsgData::ActiveOpenType>(pConfig->active->activeopentive()));
        pInfo->set_openparam(pConfig->active->activeopentime());
        pInfo->set_level(pConfig->active->minlevel());
    }
    using namespace CMD::RECORD;
    BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
    t_BroadCastMsg *ptCmd = (t_BroadCastMsg*)(pBuffer);
    constructInPlace(ptCmd);
    ptCmd->size = 0;
    std::string strret;
    encodeMessage(&rInfo,strret);
    ptCmd->size = strret.size();
    memcpy(ptCmd->data,strret.c_str(),ptCmd->size);
    //通知网关服务器
    std::string ret;
    encodeMessage(ptCmd,sizeof(t_BroadCastMsg) + ptCmd->size,ret);
    RecordTaskManager::getMe().broadcastByType(GATEWAYSERVER,ret.c_str(),ret.size());
    return false;
}

void ActiveManager::getActiveListByCur(std::set<DWORD> &rset)
{//读表，当前应该开放的活动列表
    const std::unordered_map<unsigned int, const pb::Conf_t_Active *> &tbxMap = tbx::Active().getTbxMap();
    for(auto it = tbxMap.begin();it != tbxMap.end();it++)
    {
        DWORD nowTimer = RecordTimeTick::currentTime.sec();
        if(it->second->active->starttime() != std::string("0"))
        {
            if(nowTimer < it->second->getBeginTime())
            {
                continue;
            }

        }
        if(it->second->active->endtime() != std::string("0"))
        {
            if(nowTimer > it->second->getEndTime())
            {
                continue;
            }

        }
        if(it->second->active->activeopentive() >= HelloKittyMsgData::ActiveOpenType_MAX)
            continue;
        bool bopen =false;
        switch(static_cast<HelloKittyMsgData::ActiveOpenType>(it->second->active->activeopentive()))
        {
            case HelloKittyMsgData::ActiveOpenType_Close:
                {
                }
                break;
            case HelloKittyMsgData::ActiveOpenType_AlreadyOpen:
                {
                    bopen = true;
                }
                break;
            case HelloKittyMsgData::ActiveOpenType_ByTimer:
                {
                    std::vector<std::string> temp;
                    pb::parseTagString(it->second->active->activeopentime(),",",temp);
                    if(temp.size() != 2)
                    {
                        break;
                    }
                    if(temp[0] != std::string("0"))
                    {
                        zRTime begin = zRTime(temp[0].c_str());
                        if( nowTimer < begin.sec())
                        {
                            break;
                        }

                    }
                    if(temp[1] != std::string("0"))
                    {
                        zRTime end = zRTime(temp[1].c_str());
                        if( nowTimer > end.sec())
                        {
                            break;
                        }

                    }
                    bopen = true; 

                }
                break;
            case HelloKittyMsgData::ActiveOpenType_ByWeek:
                {
                    std::vector<std::string> temp;
                    pb::parseTagString(it->second->active->activeopentime(),",",temp);
                    if(temp.size() != 2)
                        break;
                    struct tm tv;
                    zRTime::getLocalTime(tv,nowTimer);
                    std::vector<std::string> begintemp;
                    pb::parseTagString(temp[0],"_",begintemp);
                    if(begintemp.size() != 3)
                        break;
                    //星期天为0,0~6

                    int week,hour,min;
                    week = atoi(begintemp[0].c_str());
                    hour =  atoi(begintemp[1].c_str());
                    min =  atoi(begintemp[2].c_str()); 
                    std::vector<std::string> endtemp;
                    pb::parseTagString(temp[1],"_",endtemp);
                    if(endtemp.size() != 3)
                        break;
                    int week1,hour1,min1;
                    week1 = atoi(endtemp[0].c_str());
                    hour1 =  atoi(endtemp[1].c_str());
                    min1 =  atoi(endtemp[2].c_str()); 
                    //第一种情况 星期相同
                    if(week == week1)//暂不支持同一天开始时间大于结束时间的情况
                    {
                        if(week != tv.tm_wday)
                        {
                            break;
                        }
                        //全天 
                        if(hour1 == hour)
                        {
                            if(min >= min1)//开始时间大于等于结束时间，非法
                            {
                                break;
                            }
                            if(tv.tm_min < min || tv.tm_min >= min1) //不在时间内
                            {
                                break;
                            }
                            bopen = true;
                            break;

                        }
                        else if(hour > hour1) //开始小时大于结束小时,非法，24表示全天
                        {
                            break;
                        }
                        else
                        {
                            if(tv.tm_hour < hour || tv.tm_hour > hour1)//不在小时判定内
                            {
                                break;
                            }
                            if(tv.tm_hour == hour)//开始的那个小时
                            {
                                if(tv.tm_min < min)//未开始
                                {
                                    break;
                                }
                            }
                            if(tv.tm_hour == hour1)//结束的那个小时
                            {
                                if(tv.tm_min >= min1)//已结束
                                {
                                    break;
                                }
                            }
                            bopen = true;
                            break; 
                        }

                    }
                    //第二种情况，开始星期少于结束星期
                    else if(week < week1)
                    {
                        //判断星期
                        if(tv.tm_wday < week || tv.tm_wday > week1)
                        {
                            break;
                        }
                        //如果是开始的那个星期
                        if(tv.tm_wday == week)
                        {
                            //判断小时
                            if(tv.tm_hour < hour)//未开始
                            {
                                break;
                            }
                            else if(tv.tm_hour == hour)//开始的那个小时
                            {
                                if(tv.tm_min < min)//未开始
                                {
                                    break;
                                }
                            }

                        }
                        if(tv.tm_wday == week1)
                        {
                            //判断小时
                            if(tv.tm_hour > hour1)//已结束
                            {
                                break;
                            }
                            else if(tv.tm_hour == hour1)//结束的那个小时
                            {
                                if(tv.tm_min >= min1)//已结束
                                {
                                    break;
                                }
                            }

                        }
                        bopen = true;
                        break; 

                    }
                    //第三种情况，开始星期大于结束星期,反选
                    else
                    {
                        //判断星期
                        if(tv.tm_wday > week1 && tv.tm_wday < week)
                        {
                            break;
                        }
                        //如果是开始那个星期
                        if(tv.tm_wday == week)
                        {
                            //判断小时
                            if(tv.tm_hour < hour)//未开始
                            {
                                break;
                            }
                            else if(tv.tm_hour == hour)//开始的那个小时
                            {
                                if(tv.tm_min < min)//未开始
                                {
                                    break;
                                }
                            }
                        }
                        if(tv.tm_wday == week1)
                        {
                            //判断小时
                            if(tv.tm_hour > hour1)//已结束
                            {
                                break;
                            }
                            else if(tv.tm_hour == hour1)//开始的那个小时
                            {
                                if(tv.tm_min >= min1)//已结束
                                {
                                    break;
                                }
                            }

                        }
                        bopen = true;
                        break; 
                    }

                }
                break;
            case HelloKittyMsgData::ActiveOpenType_ByDay:
                {
                    std::vector<std::string> temp;
                    pb::parseTagString(it->second->active->activeopentime(),",",temp);
                    if(temp.size() != 2)
                        break;
                    struct tm tv;
                    zRTime::getLocalTime(tv,nowTimer);
                    std::vector<std::string> begintemp;
                    pb::parseTagString(temp[0],"_",begintemp);
                    if(begintemp.size() != 2)
                        break;
                    //星期天为0,0~6

                    int hour,min;
                    hour =  atoi(begintemp[0].c_str());
                    min =  atoi(begintemp[1].c_str()); 
                    std::vector<std::string> endtemp;
                    pb::parseTagString(temp[1],"_",endtemp);
                    if(endtemp.size()!=2)
                        break;
                    int hour1,min1;
                    hour1 =  atoi(endtemp[0].c_str());
                    min1 =  atoi(endtemp[1].c_str()); 
                    if(hour == hour1)//一小时结束，暂不支持一小时内，开始时间大于结束时间的情况
                    {
                        if(tv.tm_hour != hour)
                        {
                            break;
                        }
                        if(tv.tm_min < min)//未开始
                        {
                            break;
                        }
                        if(tv.tm_min >= min1)//已结束
                        {
                            break;
                        }
                        bopen = true;
                        break;

                    }
                    else if(hour < hour1)
                    {
                        if(tv.tm_hour < hour || tv.tm_hour > hour1)
                        {
                            break;
                        }
                        if(tv.tm_hour == hour)
                        {
                            if(tv.tm_min < min)//未开始
                            {
                                break;
                            }

                        }
                        if(tv.tm_hour == hour1)
                        {
                            if(tv.tm_min >= min1)//已结束
                            {
                                break;
                            }

                        }
                        bopen = true;
                        break;

                    }
                    else
                    {
                        //判断小时
                        if(tv.tm_hour >  hour1 && tv.tm_hour < hour)
                        {
                            break;
                        }
                        if(tv.tm_hour == hour)//开始的那个小时
                        {
                            if(tv.tm_min < min)//未开始
                            {
                                break;
                            }
                        }
                        else if(tv.tm_hour == hour1)//开始的那个小时
                        {
                            if(tv.tm_min >= min1)//已结束
                            {
                                break;
                            }
                        }

                    }
                    bopen = true;
                    break; 
                }
            default:
                break;



        }
        if(bopen)
            rset.insert(it->first);
    }


}



bool ActiveManager::loop()
{
    checkChange(true); 
    return true;
}

