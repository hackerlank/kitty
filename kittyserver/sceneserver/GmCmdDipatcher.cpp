#include "GmCmdDipatcher.h"
#include "SceneUser.h"
#include "dataManager.h"
#include "SuperCommand.h"
#include "SceneServer.h"
#include "TimeTick.h"
#include "TradeCmdDispatcher.h"
#include "FamilyCmdDispatcher.h"
#include "RecordClient.h"
#include "ChatCmdDispatcher.h"
#include "GuideCmdDispatcher.h"
#include "SceneToOtherManager.h"
#include "SceneUserManager.h"
#include "system.pb.h"
#include "buffer.h"
#include "UnityBuildCmdDispatcher.h"

bool GMCmdHandle::s_initFlg = false;
std::map<std::string,GMCmdHandle::GmFun> GMCmdHandle::s_gmFunMap;
std::map<std::string,std::string> GMCmdHandle::m_helpstr;

bool GMCmdHandle::reqGm(SceneUser* user,const HelloKittyMsgData::ReqGM *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }

    std::vector<std::string> commandVec;
    pb::parseTagString(cmd->command()," ",commandVec);
    if(commandVec.empty())
    {
        return false;
    }

    auto iter = s_gmFunMap.find(commandVec[0]);
    if(iter == s_gmFunMap.end())
    {
        Fir::logger->debug("[GM命令] 命令没有注册(%lu,%s,%s)",user->charid,user->charbase.nickname,commandVec[0].c_str());
        return false;
    }

    bool ret = iter->second(user,commandVec);
    Fir::logger->debug("[GM命令] 执行命令%s(%lu,%s,%s)",ret ? "成功" : "失败",user->charid,user->charbase.nickname,commandVec[0].c_str());
    return ret;
}

void GMCmdHandle::initGmFunMap()
{
    if(s_initFlg)
    {
        return;
    }
    s_initFlg = true;
#define INITGMFUN(fun,word,helpstr) {\
    s_gmFunMap.insert(std::pair<std::string,GmFun>(word,GMCmdHandle::fun));\
    m_helpstr.insert(std::pair<std::string,std::string>(word,helpstr));}
    INITGMFUN(addItem,"additem","id num");
    INITGMFUN(getPid,"pid","");
    INITGMFUN(buildLevel,"buildLevel","type value");
    INITGMFUN(addAtlas,"addAtlas","type id");
    INITGMFUN(deleteAtlas,"delatlas","type id");
    INITGMFUN(clearAtlas,"clearatlas","");
    INITGMFUN(finishTask,"finishtask","taskid");
    INITGMFUN(openTask,"opentask","taskid");
    INITGMFUN(changeHappyDay,"happyday","happyTypeVal happyDay");
    INITGMFUN(finishAchieve,"finishachieve","achieveID");
    INITGMFUN(changeTime,"time","timeStr");
    INITGMFUN(nowTime,"now","");
    INITGMFUN(clearUserData,"clear","");
    INITGMFUN(randPaper,"paper","value");
    INITGMFUN(addVistor,"addvistor","value");
    INITGMFUN(addfriend,"addfriend","roleid");
    INITGMFUN(delfriend,"delfriend","roleid");
    INITGMFUN(getfriendlist,"getfriendlist","pagemax pageno");
    INITGMFUN(getfanslist,"getfanslist","pagemax pageno");
    INITGMFUN(getotherlist,"getotherlist","pagemax");
    INITGMFUN(clearDress,"cleardress","");
    INITGMFUN(rewardBuild,"reward","");
    INITGMFUN(sendEmail,"email","nickname title conten itemid1 itemnum1 itemid2 itemnum2");
    INITGMFUN(clearPaper,"clearpaper","");
    INITGMFUN(god,"god","");
    INITGMFUN(resetDailyData,"clearday","");
    INITGMFUN(reLoadConf,"reloadconf","");
    INITGMFUN(sendSysEmail,"emailsys","title conten itemid1 itemnum1 itemid2 itemnum2");
    INITGMFUN(enterCenter,"auctioncenter","enterFlg");
    INITGMFUN(enterRoom,"auctionroom","roomID enterFlg");
    INITGMFUN(auction,"auction","roomID");
    INITGMFUN(auctionInfo,"auctionlog","roomID");
    INITGMFUN(opBulid,"opbuild","biuldID isicon");
    INITGMFUN(visit,"visit","roleid");
    INITGMFUN(createfamily,"createfamily","");
    INITGMFUN(joinfamily,"joinfamily","familyid");
    INITGMFUN(familyorder,"familyorder","orderid");
    INITGMFUN(familyaward,"familyaward","");
    INITGMFUN(familyinfo,"familyinfo","");
    INITGMFUN(openevent,"openevent","eventid");
    INITGMFUN(logRank,"logrank","rankType areatype");
    INITGMFUN(calfamily,"calfamily","");
    INITGMFUN(help,"help","keyword");
    INITGMFUN(addservernotice,"addsysmsg","lang text");
    INITGMFUN(removenotice,"delsysmsg","noticeid");
    INITGMFUN(getAllnotice,"allsysmsg","");
    INITGMFUN(luatest,"luatest","action");
    INITGMFUN(setName,"setname","name sex");
    INITGMFUN(finishguide,"finishguide","guideid");
    INITGMFUN(recordstaticnpc,"recordstaticnpc","npcid(<= 10),name,level");
    INITGMFUN(recordactivenpc,"recordactivenpc","npcid(> 10 ; <= 100),name");
    INITGMFUN(leavemsg,"leavemsg","charid chattxt public(0,1)");
    INITGMFUN(getleavemsg,"getleavemsg","");
    INITGMFUN(closeguide,"closeguide","");
    INITGMFUN(getCharID,"getcharid","getcharid");
    INITGMFUN(testGmToolTest,"test","test");
    INITGMFUN(ReqOpToy,"reqtoy","id type");
    INITGMFUN(ReqForBid,"forbid","id type time");
    INITGMFUN(RecordPath,"recordpath","filename");
    INITGMFUN(changeHeartTime,"hearttime","hearttime");
    INITGMFUN(setlevel,"level","level");
    INITGMFUN(changeGiftStatus,"setgiftstatus","id status");
    INITGMFUN(addBuffer,"addbuffer","id time");
    INITGMFUN(ReqAllUnitBuildInfo,"reqallunitbuild","");
    INITGMFUN(ReqOpenColId,"reqopencolid","playercolid unitgridid");
    INITGMFUN(ReqResetColId,"reqresetcolid","playercolid unitgridid");
    INITGMFUN(ReqUnitBuild,"requnitbuild","playercolid unitbuildid byinviteplayer paytype");   
    INITGMFUN(ReqAgreeUnitBuild,"reqagreeunitbuild","unitonlyid isagree");
    INITGMFUN(ReqCancelInvite,"reqcancelinvite","unitonlyid");
    INITGMFUN(ReqStopBuild,"reqstopbuild","unitonlyid");
    INITGMFUN(ReqAddSpeedBuild,"reqaddspeedbuild","unitonlyid");
    INITGMFUN(ReqActiveBuild,"reqactivebuild","unitonlyid"); 
    INITGMFUN(addContribute,"addcontribute","charid value");
    INITGMFUN(getunitybuildrank,"buildrank","");
    INITGMFUN(ReqAddPicture,"addpicture","id");
    INITGMFUN(GetName,"name","");
    INITGMFUN(recharge,"recharge","rmb activeID");
    INITGMFUN(upLevel,"uplevel","type");
}

bool GMCmdHandle::addContribute(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 3)
    {
        return false;
    }
    QWORD charID = 0,value = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        charID = atoll(commandVec[1].c_str());
        command += commandVec[2];
        value = atol(commandVec[2].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"GM获得(%lu,%s)",user->charid,user->charbase.nickname);
    user->opContrubute(charID,value,temp,true);
    return true;
}


bool GMCmdHandle::addItem(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 3)
    {
        return false;
    }
    DWORD itemID = 0,value = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        itemID = atoll(commandVec[1].c_str());
        command += commandVec[2];
        value = atol(commandVec[2].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    if(itemID == HelloKittyMsgData::Attr_Worker)
    {
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"GM获得(%lu,%s)",user->charid,user->charbase.nickname);
    return user->m_store_house.addOrConsumeItem(itemID,value,temp,true);
}

bool GMCmdHandle::getPid(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user)
    {
        return false;
    }
    DWORD pid = getpid();

    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"前服务器进程为:%u",pid);
    HelloKittyMsgData::AckGM ack;
    ack.set_ret(temp);

    std::string ret;
    encodeMessage(&ack,ret);
    user->sendCmdToMe(ret.c_str(),ret.size());
    Fir::logger->debug("[GM命令] 当前服务器进程为 %u(%lu,%s)",pid,user->charid,user->charbase.nickname);
    return true;
}

bool GMCmdHandle::upLevel(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD type = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        type = atol(commandVec[1].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    QWORD tempID = user->m_buildManager.getTypeBuild(type);
    return user->m_buildManager.upBuildGrade(tempID);
}


bool GMCmdHandle::buildLevel(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 3)
    {
        return false;
    }
    DWORD type = 0,value = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        type = atol(commandVec[1].c_str());
        command += commandVec[2];
        value = atol(commandVec[2].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return false;
    //return user->m_buildManager.gmUpGrade(type,value);
}

bool GMCmdHandle::addAtlas(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD type = 0,id = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        type = atol(commandVec[1].c_str());
        command += commandVec[2];
        id = atol(commandVec[2].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->m_atlasManager.addAtlas(type,id);
}

bool GMCmdHandle::deleteAtlas(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD type = 0,id = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        type = atol(commandVec[1].c_str());
        command += commandVec[2];
        id = atol(commandVec[2].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->m_atlasManager.delAtlas(type,id);
}

bool GMCmdHandle::clearAtlas(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    return user->m_atlasManager.clearAtlas();
}

bool GMCmdHandle::finishTask(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD taskID = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        taskID = atol(commandVec[1].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->m_taskManager.finishTask(taskID);
}

bool GMCmdHandle::openTask(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD taskID = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        taskID = atol(commandVec[1].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->m_taskManager.openTask(taskID,true);
}

bool GMCmdHandle::changeHappyDay(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 3)
    {
        return false;
    }
    DWORD happyTypeVal = 0,happyDay = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        happyTypeVal = atol(commandVec[1].c_str());
        command += commandVec[2];
        happyDay = atol(commandVec[2].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->changeHappyDataGm(happyTypeVal,happyDay);
}

bool GMCmdHandle::finishAchieve(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD achieveID = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        achieveID = atol(commandVec[1].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->m_achievementManager.finishAchieve(achieveID);
}

bool GMCmdHandle::changeTime(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string timeStr; 
    std::string command;
    try
    {
        command = commandVec[1];
        timeStr = commandVec[1];
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }

    CMD::SUPER::t_ChangeGameTime changTime;
    strncpy(changTime.time,timeStr.c_str(),sizeof(changTime.time));
    std::string ret;
    encodeMessage(&changTime,sizeof(changTime),ret);
    return SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
}

bool GMCmdHandle::nowTime(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user)
    {
        return false;
    }

    std::string time = SceneTimeTick::currentTime.toString(); 
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"前服务器时间为:%s",time.c_str());
    HelloKittyMsgData::AckGM ack;
    ack.set_ret(temp);

    Fir::logger->debug("[GM命令] 当前服务器时间为 %s(%lu,%s)",time.c_str(),user->charid,user->charbase.nickname);
    std::string ret;
    encodeMessage(&ack,ret);
    return user->sendCmdToMe(ret.c_str(),ret.size());
}

bool GMCmdHandle::clearUserData(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user)
    {
        return false;
    }
    user->resetAllData(); 
    return true;
}

bool GMCmdHandle::randPaper(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    std::string command;
    try
    {
        command = commandVec[1];
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::ReqSellPaper cmd;
    cmd.set_randtype(HelloKittyMsgData::Rand_Passer_By);
    user->m_trade.requireCellPaper(&cmd); 
    return true; 
}

bool GMCmdHandle::addVistor(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD value = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        value = atol(commandVec[1].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    user->addVertiseOther(value); 
    return true;
}

bool GMCmdHandle::addfriend(SceneUser* User,const std::vector<std::string> &commandVec)
{
    TradeCmdHandle handle;
    if(commandVec.size() < 2)
        return false;
    HelloKittyMsgData::ReqAddFriend req;
    req.set_playerid(atoll(commandVec[1].c_str()));
    handle.ReqAddFriend(User,&req);
    return false;

}

bool GMCmdHandle::delfriend(SceneUser* User,const std::vector<std::string> &commandVec)
{
    TradeCmdHandle handle;
    if(commandVec.size() < 2)
        return false;
    HelloKittyMsgData::ReqKickFriend req;
    req.set_playerid(atoll(commandVec[1].c_str()));
    handle.ReqKickFriend(User,&req);
    return true;

}

bool GMCmdHandle::getfriendlist(SceneUser* User,const std::vector<std::string> &commandVec)
{
    TradeCmdHandle handle;
    if(commandVec.size() != 3)
        return false;
    HelloKittyMsgData::ReqRelationList req;
    req.set_listtype(HelloKittyMsgData::FriendList);
    req.set_pagemax(atoi(commandVec[2].c_str()));
    req.set_pageno(atoi(commandVec[1].c_str()));
    handle.ReqRelationList(User,&req);
    return true;

}

bool GMCmdHandle::getfanslist(SceneUser* User,const std::vector<std::string> &commandVec)
{
    TradeCmdHandle handle;
    if(commandVec.size() != 3)
        return false;
    HelloKittyMsgData::ReqRelationList req;
    req.set_listtype(HelloKittyMsgData::FansList);
    req.set_pagemax(atoi(commandVec[2].c_str()));
    req.set_pageno(atoi(commandVec[1].c_str()));
    handle.ReqRelationList(User,&req);
    return true;

}

bool GMCmdHandle::getotherlist(SceneUser* User,const std::vector<std::string> &commandVec)
{
    TradeCmdHandle handle;
    if(commandVec.size() != 2)
        return false;
    HelloKittyMsgData::ReqRelationList req;
    req.set_listtype(HelloKittyMsgData::RadomList);
    req.set_pagemax(atoi(commandVec[1].c_str()));
    handle.ReqRelationList(User,&req);
    return true;

}

bool GMCmdHandle::rewardBuild(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    //user->m_buildManager.getReward();
    return true;
}

bool GMCmdHandle::sendEmail(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 4)
    {
        return false;
    }
    const char *title = NULL,*conten = NULL;
    QWORD charID = 0;
    std::map<DWORD,DWORD> itemMap;
    std::string command;
    try
    {
        command = commandVec[1];
        charID = atol(commandVec[1].c_str());

        command += commandVec[2];
        title = commandVec[2].c_str();

        command += commandVec[3];
        conten = commandVec[3].c_str();

        for(size_t index = 4;index + 1 < commandVec.size();)
        {
            command += commandVec[index];
            DWORD key = atol(commandVec[index].c_str());

            command += commandVec[index+1];
            DWORD value = atol(commandVec[index+1].c_str());

            if(itemMap.find(key) == itemMap.end())
            {
                itemMap.insert(std::pair<DWORD,DWORD>(key,value));
            }
            else
            {
                itemMap[key] += value;
            }
            index += 2;
        }

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    std::vector<HelloKittyMsgData::ReplaceWord> argVec;
    return EmailManager::sendEmailBySys(charID,title,conten,argVec,itemMap);
}

bool GMCmdHandle::clearDress(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    return user->m_dressManager.clearDress();
}

bool GMCmdHandle::clearPaper(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    return user->m_paperManager.clearPaper();
}

bool GMCmdHandle::god(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    user->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,1000000,"gm给予",true);
    user->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,1000000,"gm给予",true);
    user->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Exp,100000000,"gm给予",true);
    user->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Coupons,100000,"gm给予",true);
    user->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Token,100000,"gm给予",true);
    user->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Max,1000000,"gm给予",true);
    user->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Now,10000,"gm给予",true);




    //user->m_buildManager.gmUpGrade(10);
    return true;
}
bool GMCmdHandle::setlevel(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD setlevel = atoi(commandVec[1].c_str());
    if(user->checkLevel(setlevel)|| setlevel < 2)
    {
        return false;
    }
    QWORD needexp = 0;
    //当前经验
    QWORD curexp = user->m_store_house.getAttr(HelloKittyMsgData::Attr_Exp);
    //该等级经验
    const pb::Conf_t_upgrade* levelConf = tbx::upgrade().get_base(setlevel-1);//上一级满级经验
    if(levelConf)
    {
        needexp = levelConf->upgrade->exp() - curexp;
    }
    user->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Exp,needexp,"gm给予",true);
    return true;
}
bool GMCmdHandle::resetDailyData(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    user->initDailyData();
    return true;
}

bool GMCmdHandle::reLoadConf(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    CMD::SUPER::t_ReloadConfig reloadConf;
    std::string ret;
    encodeMessage(&reloadConf,sizeof(reloadConf),ret);
    return SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
}

bool GMCmdHandle::sendSysEmail(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 3)
    {
        return false;
    }
    const char *title = NULL,*conten = NULL;
    std::map<DWORD,DWORD> itemMap;
    std::string command;
    try
    {
        command = commandVec[1];
        title = commandVec[1].c_str();

        command += commandVec[2];
        conten = commandVec[2].c_str();

        for(size_t index = 3;index + 1 < commandVec.size();)
        {
            command += commandVec[index];
            DWORD key = atol(commandVec[index].c_str());

            command += commandVec[index+1];
            DWORD value = atol(commandVec[index+1].c_str());

            if(itemMap.find(key) == itemMap.end())
            {
                itemMap.insert(std::pair<DWORD,DWORD>(key,value));
            }
            else
            {
                itemMap[key] += value;
            }
            index += 2;
        }

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    std::vector<HelloKittyMsgData::ReplaceWord> argVec;
    return EmailManager::sendEmailBySys(title,conten,argVec,itemMap);
}

bool GMCmdHandle::enterCenter(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    bool enterFlg = true;
    std::string command;
    try
    {
        if(commandVec.size() >=2 )
        {
            command = commandVec[1];
            DWORD val =  atol(commandVec[1].c_str());
            enterFlg = val ? true : false;
        } 
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->opRecreationCenter(enterFlg);
}

bool GMCmdHandle::enterRoom(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD roomID = 0;
    bool enterFlg = true;
    std::string command;
    try
    {
        command = commandVec[1];
        roomID = atol(commandVec[1].c_str());
        if(commandVec.size() >=3)
        {
            command += commandVec[2];
            DWORD val =  atol(commandVec[2].c_str());
            enterFlg = val ? true : false;
        } 
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->opAuctionRoom(roomID,enterFlg);
}

bool GMCmdHandle::auctionInfo(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    DWORD roomID = 0;
    std::string command;
    try
    {
        if(commandVec.size() >=2)
        {
            command = commandVec[1];
            roomID = atol(commandVec[1].c_str());
        } 
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->logAuctionInfo(roomID);
}

bool GMCmdHandle::auction(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD roomID = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        roomID = atol(commandVec[1].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->auction(roomID);
}

bool GMCmdHandle::opBulid(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    QWORD biuldID = 0;
    DWORD isicon = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        biuldID = atol(commandVec[1].c_str());
        if(commandVec.size() >= 2)
        {
            isicon = atoll(commandVec[2].c_str());
        }
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::Builditype rBuild;
    rBuild.set_buildid(biuldID);
    rBuild.set_isicon(isicon > 0 ? 1 : 0);
    user->opBuild(user->charid,rBuild);
    return true;

}

bool GMCmdHandle::visit(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    QWORD visitID = 0;
    std::string command;
    try
    {
        command = commandVec[1];
        visitID = atoll(commandVec[1].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::ReqEnterGarden req;
    req.set_charid(visitID);
    TradeCmdHandle handle;
    return handle.ReqEnterGarden(user,&req);

}

bool GMCmdHandle::createfamily(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string familyname;
    std::string command;
    try
    {
        command = commandVec[0];
        familyname = commandVec[1];
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::ReqCreateFamily req;
    req.set_familyname(familyname);
    req.set_familyicon(1000);   
    req.set_pulictype(HelloKittyMsgData::PublicForAll);
    req.set_lowlevel(0);
    req.set_highlevel(0);
    FamilyCmdHandle handle;
    handle.ReqCreateFamilyByGm(user,&req);
    return true;

}

bool GMCmdHandle::joinfamily(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    QWORD familyid = 0;
    std::string command;
    try
    {
        command = commandVec[0];
        familyid = atoll(commandVec[1].c_str());;
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::ReqAddFamily req;
    req.set_familyid(familyid);
    FamilyCmdHandle handle;
    return handle.ReqAddFamilyByGm(user,&req);


}

bool GMCmdHandle::familyinfo(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    std::string command;
    try
    {
        command = commandVec[0];
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::ReqselfFamilyInfo req;
    FamilyCmdHandle handle;
    return handle.ReqselfFamilyInfo(user,&req);


}

bool GMCmdHandle::familyorder(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    QWORD familyorderid = 0;
    std::string command;
    try
    {
        command = commandVec[0];
        familyorderid = atol(commandVec[1].c_str());;

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::ReqFinishFamilyOrder req;
    req.set_orderid(familyorderid);
    FamilyCmdHandle handle;
    return handle.ReqFinishFamilyOrder(user,&req);


}

bool GMCmdHandle::familyaward(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    std::string command;
    try
    {
        command = commandVec[0];
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::ReqGetlastFamilyAward req;
    FamilyCmdHandle handle;
    return handle.ReqGetlastFamilyAward(user,&req);


}

bool GMCmdHandle::openevent(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    DWORD eventid = 0;
    std::string command;
    try
    {
        command = commandVec[0];
        eventid = atol(commandVec[1].c_str());;

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    user->m_eventmanager.Gmopenevent(eventid);
    return true;


}
bool GMCmdHandle::logRank(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }

    DWORD rankType = 0,areaType = (DWORD)(-1);
    std::string command;
    try
    {
        command = commandVec[1];
        rankType = atoi(commandVec[1].c_str());
        if(commandVec.size() > 2)
        {
            command += commandVec[2];
            areaType = atoi(commandVec[2].c_str());
        }


    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }

    HelloKittyMsgData::RankType rank = (HelloKittyMsgData::RankType)(rankType);
    switch(rank)
    {
        case HelloKittyMsgData::RT_Level:
            {
                if(areaType == ((DWORD)(-1)))
                {
                    return user->logLevelRank();
                }
                return areaType ? user->logAreaLevelRank((HelloKittyMsgData::AreaType)(areaType)) : user->logAllAreaLevelRank();
            }
            break;
        default:
            {
                break;
            }
    }
    return false;
}

bool GMCmdHandle::calfamily(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    std::string command;
    try
    {
        command = commandVec[0];

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    if(commandVec.size() == 1)
    {
    }
    RecordClient *recordClient = MgrrecordClient.GetRecordByTableName("t_family");
    if(!recordClient)
        return false;
    CMD::RECORD::t_CalFamilyByGM cmd;
    std::string ret;
    encodeMessage(&cmd,sizeof(cmd),ret);
    recordClient->sendCmd(ret.c_str(),ret.size());
    return true;


}

bool GMCmdHandle::help(SceneUser* user,const std::vector<std::string> &commandVec) 
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    std::string command;
    try
    {
        command = commandVec[0];

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    std::ostringstream oss;
    if(commandVec.size() == 1)
    {
        for(auto it = m_helpstr.begin();it != m_helpstr.end();it++)
        {

            oss <<"\n" << it->first<<":" << it->second <<"\n";

        }
    }
    else
    {
        const std::string &helpstr = commandVec[1];
        for(auto it = m_helpstr.begin();it != m_helpstr.end();it++)
        {
            if(int(it->first.find(helpstr)) != -1)
                oss <<"\n" << it->first<<":" << it->second <<"\n";

        }


    }
    HelloKittyMsgData::AckGM ack;
    ack.set_ret(oss.str());
    std::string ret;
    encodeMessage(&ack,ret);
    user->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool GMCmdHandle::addservernotice(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 3)
    {
        return false;
    }
    std::string command;
    DWORD lang = 0;
    std::string notice;
    try
    {
        command = commandVec[0];
        lang = atoi(commandVec[1].c_str());
        notice = commandVec[2];

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    RecordClient *recordClient = MgrrecordClient.GetRecordByTableName("servernotice");
    if(!recordClient)
        return false;
    CMD::RECORD::t_SeverNoticeScenAdd addinfo;
    addinfo.lang =lang;
    sprintf(addinfo.notice,notice.c_str(),sizeof(addinfo.notice));
    std::string ret;
    encodeMessage(&addinfo,sizeof(addinfo),ret);
    recordClient->sendCmd(ret.c_str(),ret.size());
    return true;
}

bool GMCmdHandle::removenotice(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    QWORD ID = 0;
    try
    {
        command = commandVec[0];
        ID = atoll(commandVec[1].c_str());

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    RecordClient *recordClient = MgrrecordClient.GetRecordByTableName("servernotice");
    if(!recordClient)
        return false;
    CMD::RECORD::t_SeverNoticeScenDel delinfo;
    delinfo.ID = ID;
    std::string ret;
    encodeMessage(&delinfo,sizeof(delinfo),ret);
    recordClient->sendCmd(ret.c_str(),ret.size());
    return true;


}
bool GMCmdHandle::getAllnotice(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    std::string command;
    try
    {
        command = commandVec[0];

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::ReqServerNotice cmd;
    ChatCmdHandle handle;
    handle.ReqServerNotice(user,&cmd);
    return true;

}

bool GMCmdHandle::luatest(SceneUser* user,const std::vector<std::string> &commandVec) 
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    std::string action;
    try
    {
        command = commandVec[0];
        action = commandVec[1];

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    std::ostringstream oss;
    if(std::string("lock") == action)
    {
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
        if(handle)
        {
            if(handle->getLockLua("testlock",0,NULL,30))
            {
                oss << "add lock ok";
            }
            else
            {
                oss << "add lock err";
            }
        }

    }
    else if(std::string("unlock") == action)
    {
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
        if(handle)
        {
            if(handle->delLockLua("testlock",0,NULL))
            {
                oss << "del lock ok";
            }
            else
            {
                oss << "del lock err";
            }
        }
    }
    else if(std::string("islock") == action)
    {
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
        if(handle)
        {
            if(handle->isLockLua("testlock",0,NULL))
            {
                oss << "is lock ok";
            }
            else
            {
                oss << "is lock err";
            }
        }
    }
    HelloKittyMsgData::AckGM ack;
    ack.set_ret(oss.str());
    std::string ret;
    encodeMessage(&ack,ret);
    user->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool GMCmdHandle::setName(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 3)
    {
        return false;
    }
    std::string command;
    std::string newName;
    BYTE sex = 0;

    try
    {
        command = commandVec[0];
        newName = commandVec[1];
        sex  = atoi(commandVec[2].c_str());
        if(sex  != HelloKittyMsgData::Female && sex != HelloKittyMsgData::Male)
        {
            return false;
        }

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::SexType esex = static_cast<HelloKittyMsgData::SexType>(sex);
    HelloKittyMsgData::ReqSetRoleName Req;
    Req.set_name(newName);
    Req.set_sex(esex);
    guideCmdHandle handle;
    handle.ReqSetRoleName(user,&Req);
    return true;

}
bool GMCmdHandle::finishguide(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    DWORD curGuideId;

    try
    {
        command = commandVec[0];
        curGuideId  = atoi(commandVec[1].c_str());

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::Reqsetguidefinish Req;
    Req.set_guideid(curGuideId);
    guideCmdHandle handle;
    handle.Reqsetguidefinish(user,&Req);
    return true;

}

bool GMCmdHandle::recordstaticnpc(SceneUser* User,const std::vector<std::string> &commandVec)
{
    if(!User || commandVec.size() < 4)
    {
        return false;
    }
    std::string command;
    DWORD NpcId;
    string Npcname; 
    DWORD level;

    try
    {
        command = commandVec[0];
        NpcId  = atoi(commandVec[1].c_str());
        if(NpcId == 0 || NpcId > 10)
        {
            return false;
        }
        Npcname = commandVec[2];
        level = atoi(commandVec[3].c_str());
        if(level == 0)
            return false;



    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    std::ostringstream oss;
    oss << "创建静态npc :";
    oss << NpcId;
    oss << "名字 ";
    oss << Npcname.c_str();
    HelloKittyMsgData::AckGM ack;
    ack.set_ret(oss.str());
    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    User->recordstaticnpc(NpcId,Npcname,level);
    return true;

}

bool GMCmdHandle::RecordPath(SceneUser* User,const std::vector<std::string> &commandVec)
{
    if(!User || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    string filename; 

    try
    {
        command = commandVec[0];
        filename = commandVec[1];
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    std::ostringstream oss;
    oss << "录制道路 :";
    oss << filename;
    HelloKittyMsgData::AckGM ack;
    ack.set_ret(oss.str());
    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    User->recordPath(filename);
    return true;

}

bool GMCmdHandle::recordactivenpc(SceneUser* User,const std::vector<std::string> &commandVec)
{
    if(!User || commandVec.size() < 3)
    {
        return false;
    }
    std::string command;
    DWORD NpcId;
    string Npcname;

    try
    {
        command = commandVec[0];
        NpcId  = atoi(commandVec[1].c_str());
        if(NpcId < 11 || NpcId > 100)
        {
            return false;
        }
        Npcname = commandVec[2]; 


    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    std::ostringstream oss;
    oss << "创建动态npc: ";
    oss << NpcId;
    oss << "名字 ";
    oss << Npcname.c_str();
    if(!User->recordactivenpc(NpcId,Npcname))
    {
        oss << "失败，因为服务器忙，请再试";
    }
    else
    {
        oss << "成功";
    }
    HelloKittyMsgData::AckGM ack;
    ack.set_ret(oss.str());
    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool GMCmdHandle::leavemsg(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 4)
    {
        return false;
    }
    std::string command;
    QWORD charid;
    std::string chattxt;
    try
    {
        command = commandVec[0];
        charid = atoll(commandVec[1].c_str());
        chattxt = commandVec[2];
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::ReqLeaveMessage cmd;
    cmd.set_recvid(charid);
    cmd.set_chattxt(chattxt);
    ChatCmdHandle handle;
    handle.ReqLeaveMessage(user,&cmd);
    return true;

}
bool GMCmdHandle::getleavemsg(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    std::string command;
    try
    {
        command = commandVec[0];
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return true;

}

bool GMCmdHandle::closeguide(SceneUser* user,const std::vector<std::string> &commandVec)
{

    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    std::string command;
    try
    {
        command = commandVec[0];
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::Reqsetguidefinish cmd;
    cmd.set_guideid(pb::Conf_t_NewGuide::getlastGuide());
    guideCmdHandle handle;
    handle.Reqsetguidefinish(user,&cmd);
    return true;

}

bool GMCmdHandle::getCharID(SceneUser* user,const std::vector<std::string> &commandVec)
{

    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    char *account = NULL;
    DWORD acctype = 1;
    try
    {
        command = commandVec[1];
        account = const_cast<char*>(commandVec[1].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(acctype);
    if(!handle)
    {
        return false;
    }
    QWORD charID = handle->getInt("rolebaseinfo",acctype,account);

    HelloKittyMsgData::AckGM ack;
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"账号(%s)的角色id(%lu)",account,charID);
    ack.set_ret(temp);

    std::string ret;
    encodeMessage(&ack,ret);
    bool flg = user->sendCmdToMe(ret.c_str(),ret.size());
    Fir::logger->debug("[GM命令] 执行命令%s(%lu,%s,%s,%s,%lu)",flg ? "成功" : "失败",user->charid,user->charbase.nickname,commandVec[0].c_str(),account,charID);
    return flg;
}

bool GMCmdHandle::testGmToolTest(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }

    HelloKittyMsgData::ReqTest test;
    test.set_num(10);
    std::string ret;
    encodeMessage(&test,ret);
    Fir::logger->debug("test proto size %lu:",ret.size());
    return true;
}

bool GMCmdHandle::ReqOpToy(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 3)
    {
        return false;
    }
    std::string command;
    DWORD activeid;
    DWORD type;
    try
    {
        command = commandVec[0];
        activeid = atoi(commandVec[1].c_str());
        type = atoi(commandVec[2].c_str());

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }

    HelloKittyMsgData::ReqOpToy cmd;
    cmd.set_activeid(activeid);
    cmd.set_optype(static_cast<HelloKittyMsgData::OpToyType>(type));
    TradeCmdHandle handle;
    handle.reqToy(user,&cmd);
    return true;

}

bool GMCmdHandle::ReqForBid(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    DWORD type = 0,time = 24;
    QWORD charID = 0;
    try
    {
        command = commandVec[1];
        charID = atol(commandVec[1].c_str());
        if(commandVec.size() >= 3)
        {
            command + commandVec[2];
            type = atoi(commandVec[2].c_str());
        }
        if(commandVec.size() >= 4)
        {
            command + commandVec[3];
            time = atoi(commandVec[3].c_str());
        }
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charID);
    if(!handle)
    {
        return false;
    }
    DWORD endTime = 0; 
    if(type == 0 || type ==2)
    {
        endTime = SceneTimeTick::currentTime.sec() + time * 3600;
    }
    else if(type == 1 || type ==3)
    {
        endTime = SceneTimeTick::currentTime.sec() - 10;
    }
    else
    {
        return false;
    }

    DWORD SenceId = handle->getInt("playerscene",charID,"sceneid");
    if(SenceId > 0)
    {
        SceneUser* user = SceneUserManager::getMe().getUserByID(charID);
        if(user)
        {
            return type < 2 ? user->forBid(endTime,"封号") : user->forBidSys(endTime,"禁言");
        }
        CMD::SCENE::t_UserForBid sendCmd;
        sendCmd.charID = charID; 
        sendCmd.endTime = endTime;
        sendCmd.opForBid = type < 2 ? true : false;
        strncpy(sendCmd.reason,type < 2 ? "封号" : "禁言",sizeof(sendCmd.reason));
        std::string ret;
        encodeMessage(&sendCmd,sizeof(sendCmd),ret);
        return SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size());
    }
    SceneUser* u  =  SceneUserManager::getMe().CreateTempUser(charID);
    return u->forBid(endTime,"封号");
}

bool GMCmdHandle::changeHeartTime(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    DWORD heartTime = 0;
    try
    {
        command = commandVec[1];
        heartTime = atol(commandVec[1].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }

    CMD::SUPER::t_ChangeHeartTime changeHeartTime;
    changeHeartTime.heartTime = heartTime;
    std::string ret;
    encodeMessage(&changeHeartTime,sizeof(changeHeartTime),ret);
    return SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
}

bool GMCmdHandle::changeGiftStatus(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    QWORD giftID = 0,status = 0;
    try
    {
        command = commandVec[1];
        giftID = atoll(commandVec[1].c_str());
        command += commandVec[2];
        status = atoll(commandVec[2].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }

    return user->m_giftPackage.changeGiftStautus(giftID,status);
}

bool GMCmdHandle::addBuffer(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 3)
    {
        return false;
    }
    std::string command;
    QWORD bufferID = 0,time = 0;
    try
    {
        command = commandVec[0];
        bufferID = atoll(commandVec[1].c_str());
        command += commandVec[2];
        time = atoll(commandVec[2].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    std::map<DWORD,pb::BufferMsg> bufferMap;
    pb::BufferMsg bufferMsg;
    bufferMsg.id = bufferID;
    bufferMsg.time = time ? time : 360;
    bufferMap.insert(std::pair<DWORD,pb::BufferMsg>(bufferID,bufferMsg));
    return opBuffer(user,HelloKittyMsgData::BST_Default,bufferMap);
}

bool GMCmdHandle::ReqAllUnitBuildInfo(SceneUser* User,const std::vector<std::string> &commandVec)
{
    unitbuildCmdHandle  cmd;
    HelloKittyMsgData::ReqAllUnitBuildInfo req;
    cmd.ReqAllUnitBuildInfo(User,&req);

    return true;
}

bool GMCmdHandle::ReqOpenColId(SceneUser* User,const std::vector<std::string> &commandVec)
{
    unitbuildCmdHandle cmd;
    HelloKittyMsgData::ReqOpenColId req;
    if(!User || commandVec.size() < 3)
    {
        return false;
    }
    std::string command;
    DWORD colID = 0,grid = 0;
    try
    {
        command = commandVec[0]; 
        colID = atoll(commandVec[1].c_str());
        grid = atoll(commandVec[2].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    req.set_playercolid(colID);
    req.set_unitgridid(grid);
    cmd.ReqOpenColId(User,&req); 

    return true;
}

bool GMCmdHandle::ReqResetColId(SceneUser* User,const std::vector<std::string> &commandVec)
{
    unitbuildCmdHandle cmd;
    HelloKittyMsgData::ReqResetColId req;
    if(!User || commandVec.size() < 3)
    {
        return false;
    }
    std::string command;
    DWORD colID = 0,grid = 0;
    try
    {
        command = commandVec[0]; 
        colID = atoll(commandVec[1].c_str());
        grid = atoll(commandVec[2].c_str());
    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    req.set_playercolid(colID);
    req.set_unitgridid(grid);
    cmd.ReqResetColId(User,&req); 

    return true;
}

bool GMCmdHandle::ReqUnitBuild(SceneUser* User,const std::vector<std::string> &commandVec)
{
    unitbuildCmdHandle cmd;
    HelloKittyMsgData::ReqUnitBuild req;
    if(!User || commandVec.size() < 5)
    {
        return false;
    }
    std::string command;
    DWORD colID = 0,Bulidid = 0;
    QWORD ByInvitePlayer = 0;
    HelloKittyMsgData::PayType paytype = HelloKittyMsgData::PayType_Self;

    try
    {
        command = commandVec[0]; 
        colID = atoi(commandVec[1].c_str());
        Bulidid = atoi(commandVec[2].c_str());
        ByInvitePlayer = atoll(commandVec[3].c_str()); 
        if(atoi(commandVec[4].c_str()) > 0)
        {
            paytype = HelloKittyMsgData::PayType_Other;
        }

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    req.set_playercolid(colID);
    req.set_unitbuildid(Bulidid);
    req.set_byinviteplayer(ByInvitePlayer);
    req.set_paytype(paytype);
    cmd.ReqUnitBuild(User,&req); 
    return true;
}

bool GMCmdHandle::ReqAgreeUnitBuild(SceneUser* User,const std::vector<std::string> &commandVec)
{
    unitbuildCmdHandle cmd;
    HelloKittyMsgData::ReqAgreeUnitBuild req;
    if(!User || commandVec.size() < 3)
    {
        return false;
    }
    std::string command;
    DWORD PlayerColID = 0;
    bool bAgree = false;

    try
    {
        command = commandVec[0]; 
        PlayerColID = atoi(commandVec[1].c_str());
        bAgree = atoi(commandVec[2].c_str()) > 0;

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    req.set_colid(PlayerColID);
    req.set_isagree(bAgree);
    cmd.ReqAgreeUnitBuild(User,&req); 
    return true;
}

bool GMCmdHandle::ReqCancelInvite(SceneUser* User,const std::vector<std::string> &commandVec)
{
    unitbuildCmdHandle cmd;
    HelloKittyMsgData::ReqCancelInvite req;
    if(!User || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    DWORD PlayerColID = 0;
    try
    {
        command = commandVec[0]; 
        PlayerColID = atoi(commandVec[1].c_str());

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    req.set_colid(PlayerColID);
    cmd.ReqCancelInvite(User,&req); 
    return true;
}

bool GMCmdHandle::ReqStopBuild(SceneUser* User,const std::vector<std::string> &commandVec)
{
    unitbuildCmdHandle cmd;
    HelloKittyMsgData::ReqStopBuild req;
    if(!User || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    DWORD PlayerColID = 0;
    try
    {
        command = commandVec[0]; 
        PlayerColID = atoi(commandVec[1].c_str());

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    req.set_colid(PlayerColID);
    cmd.ReqStopBuild(User,&req); 
    return true;

}

bool GMCmdHandle::ReqAddSpeedBuild(SceneUser* User,const std::vector<std::string> &commandVec)
{
    unitbuildCmdHandle cmd;
    HelloKittyMsgData::ReqAddSpeedBuild req;
    if(!User || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    DWORD PlayerColID = 0;
    try
    {
        command = commandVec[0]; 
        PlayerColID = atoi(commandVec[1].c_str());

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    req.set_colid(PlayerColID);
    cmd.ReqAddSpeedBuild(User,&req); 
    return true;

}

bool GMCmdHandle::ReqActiveBuild(SceneUser* User,const std::vector<std::string> &commandVec)
{
    unitbuildCmdHandle cmd;
    HelloKittyMsgData::ReqActiveBuild req;
    if(!User || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    DWORD PlayerColID = 0;
    try
    {
        command = commandVec[0]; 
        PlayerColID = atoi(commandVec[1].c_str());

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),User->charid,User->charbase.nickname);
        return false;
    }
    req.set_colid(PlayerColID);
    cmd.ReqActiveBuild(User,&req); 
    return true;
}

bool GMCmdHandle::getunitybuildrank(SceneUser* User,const std::vector<std::string> &commandVec) 
{
    User->unitybuildRank(NULL);
    return true;
}

bool GMCmdHandle::ReqAddPicture(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    DWORD pictureID = 0;
    try
    {
        command = commandVec[0]; 
        pictureID = atoi(commandVec[1].c_str());

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    HelloKittyMsgData::ReqAddPicture reqMsg;
    reqMsg.set_id(pictureID);
    user->addPicture(&reqMsg);
    return true;
}

bool GMCmdHandle::GetName(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 1)
    {
        return false;
    }
    std::string command;
    try
    {
        command = commandVec[0]; 

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    Fir::logger->debug("name is %s",user->charbase.nickname);
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(user->charid);
    if(handle)
    {
          const char *nickName = (const char*)handle->get("rolebaseinfo",user->charid, "nickname");
          if(nickName)
          {
               Fir::logger->debug("name is %s",std::string(nickName).c_str());
          }
          Fir::logger->debug("name is %s",std::string(handle->get("rolebaseinfo",user->charid, "nickname")).c_str());

    }

    return true;

}

bool GMCmdHandle::recharge(SceneUser* user,const std::vector<std::string> &commandVec)
{
    if(!user || commandVec.size() < 2)
    {
        return false;
    }
    std::string command;
    DWORD rmb = 0;
    DWORD activeID = 0;
    try
    {
        command = commandVec[0]; 
        command += commandVec[1];
        rmb = atol(commandVec[1].c_str());
        if(commandVec.size() > 2)
        {
            command += commandVec[2];
            activeID = atol(commandVec[2].c_str());
        }

    }
    catch(...)
    {
        Fir::logger->debug("[GM命令] 命令格式错误%s(%lu,%s)",command.c_str(),user->charid,user->charbase.nickname);
        return false;
    }
    return user->rechargeRMB(rmb,activeID);
}
