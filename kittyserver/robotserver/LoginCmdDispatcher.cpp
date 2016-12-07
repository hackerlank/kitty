/**
 * \file
 * \version  $Id: LoginCmdDispatcher.cpp 64 2013-04-23 02:05:08Z  $
 * \author   ,
 * \date 2013年03月27日 12时14分48秒 CST
 * \brief 定义用户登录相关命令处理文件，注册给dispatcher
 *
 */

#include "LoginCmdDispatcher.h"
#include "LoadClientManager.h"


bool LoginCmdHandle::AckVersion(LoadClient* task,const HelloKittyMsgData::AckVersion *message)
{
    Fir::logger->info("task:%d,get version :%f,task load state: %u",task->getAccount(),message->version(),task->getLoadState());
    if(Loadfl == task->getLoadState())
    {
        task->setLoadState(flgetversion);
        HelloKittyMsgData::ReqLogin req;
        std::string straccount;
        DWORD type;
        std::string strpwd;
        task->getstrAccountAndPlat(straccount,type,strpwd);
        req.set_account(straccount);
        req.set_tocken(strpwd);
        req.set_platid(HelloKittyMsgData::debug);

        std::string ret; 
        if(encodeMessage(&req,ret)) 
            task->sendCmd(ret.c_str(), ret.size()); 
    }
    return true;
}

bool LoginCmdHandle::AckLoginFailReturn(LoadClient* task,const HelloKittyMsgData::AckLoginFailReturn *message)
{
    if(message->failreason() == HelloKittyMsgData::NoAccount)
    {
        Fir::logger->info("task:%d,register",task->getAccount());
        HelloKittyMsgData::ReqRegister req;
        std::string straccount;
        DWORD type;
        std::string strpwd;
        task->getstrAccountAndPlat(straccount,type,strpwd);
        req.set_account(straccount);
        req.set_pwd(strpwd);
        std::string ret; 
        if(encodeMessage(&req,ret)) 
            task->sendCmd(ret.c_str(), ret.size()); 

    }
    else
    {
        Fir::logger->info("task:%d,login fail :%u",task->getAccount(),message->failreason());

    }
    return true;
}

bool LoginCmdHandle::AckLoginSuccessReturn(LoadClient* task,const HelloKittyMsgData::AckLoginSuccessReturn *message)
{
    Fir::logger->info("task:%d,login suc,gateway Ip is %s, port is %u",task->getAccount(),message->gatewayip().c_str(),message->gatewayport());
    bool bConnectGateWay = false; 
    std::string straccount;
    DWORD type;
    std::string strpwd;
    task->getstrAccountAndPlat(straccount,type,strpwd);

    if(!task->isspecail())
        bConnectGateWay = LoadClientManager::getMe().connectToGateway(task->getAccount(),message->gatewayip(),static_cast<SWORD>(message->gatewayport()),task->getLang());
    else
        bConnectGateWay = LoadClientManager::getMe().specailconnectToGateway(straccount,type,strpwd,message->gatewayip(),static_cast<SWORD>(message->gatewayport()),task->getLang());
    if(!bConnectGateWay)
    {
        Fir::logger->info("task:%d,login suc,gateway Ip is %s, port is %u,connect gateway fail",task->getAccount(),message->gatewayip().c_str(),message->gatewayport());

    }
    return true;

}

bool LoginCmdHandle::AckFlushUserInfo(LoadClient* task,const HelloKittyMsgData::AckFlushUserInfo *message)
{
    Fir::logger->info("task:%d,enter game userid is %lu",task->getAccount(),message->userbase().charid());
    Fir::logger->info("task:%d(%lu) name :%s setName :%u nextguideid %u",task->getAccount(),message->userbase().charid(),message->userbase().name().c_str(),message->userbase().hassetname(),message->userbase().nextguideid());
    task->setLoadState(InGame);
    task->setUserId(message->userbase().charid());
    const HelloKittyMsgData::Evententer &rInit =  message->userbase().eventinit();
    for(int i = 0;i !=  rInit.eventbuild_size();i++)
    {
        const HelloKittyMsgData::EventBuildNotice &rNotice = rInit.eventbuild(i);
        show(&rNotice);
    }
    show(&(rInit.eventinfo()));
    return true;
}

bool LoginCmdHandle::AckKittyGarden(LoadClient* task,const HelloKittyMsgData::AckKittyGarden *message)
{
    Fir::logger->info("task:%d(charid %lu),get %lu garden  info ",task->getAccount(),task->getcharid(),message->charid());

    return true;
}
bool LoginCmdHandle::EventBuildNotice(LoadClient* task,const HelloKittyMsgData::EventBuildNotice *message)
{
    Fir::logger->info("task:%d(charid %lu),get new  EventBuildNotice ",task->getAccount(),task->getcharid());
    show(message);
    return true;
}

void LoginCmdHandle::show(const HelloKittyMsgData::EventBuildNotice *message)
{
    Fir::logger->info("from charid %lu ,eventid %u,build %lu,data %u,totalreserverTimer %u ,canop %u ,reservetime %u",message->charid(),message->eventid(),message->build().buildid(),message->data(),message->totalreservertimer(),message->canop(),message->reservetime());

}

bool LoginCmdHandle::AckEnterGarden(LoadClient* task,const HelloKittyMsgData::AckEnterGarden *message)
{
    Fir::logger->info("task:%d(charid %lu),visit %lu ",task->getAccount(),task->getcharid(),message->gardeninfo().playershow().playerid());
    const HelloKittyMsgData::Evententer &rInit =  message->gardeninfo().eventinit();
    for(int i = 0;i !=  rInit.eventbuild_size();i++)
    {
        const HelloKittyMsgData::EventBuildNotice &rNotice = rInit.eventbuild(i);
        show(&rNotice);
    }
    show(&(rInit.eventinfo()));

    return true;
}

void LoginCmdHandle::show(const HelloKittyMsgData::EventNotice *message)
{

    Fir::logger->info("EventNotice desc char  %lu ,eventid %u",message->charid(),message->eventid());
}

bool LoginCmdHandle::EventNotice(LoadClient* task,const HelloKittyMsgData::EventNotice *message)
{
    Fir::logger->info("task:%d(charid %lu),get new  EventNotice ",task->getAccount(),task->getcharid());
    show(message);
    return true;

}

bool LoginCmdHandle::returnEventAward(LoadClient* task,const HelloKittyMsgData::returnEventAward *message)
{
    Fir::logger->info("task:%d(charid %lu),get event award ",task->getAccount(),task->getcharid());
    Fir::logger->info("ownercharid %lu,eventid %u,ismail %u",message->charid(),message->eventid(),message->ismail());
    Fir::logger->info("awrd desc: size : %u",message->award_size());
    for(int i= 0; i != message->award_size();i++)
    {
        const  HelloKittyMsgData::Award &raward = message->award(i);
        Fir::logger->info("type is %u,value is %u",raward.awardtype(),raward.awardval());
    }
    return true;
}

bool LoginCmdHandle::AckopBuilding(LoadClient* task,const HelloKittyMsgData::AckopBuilding *message)
{
    Fir::logger->info("task:%d(charid %lu) get AckopBuilding  result %u",task->getAccount(),task->getcharid(),message->result()); 
    if(message->result() == 0)
    {
        Fir::logger->info("owner id :%lu,bulidid %lu,eventid %d,process %d",message->charid(),message->build().buildid(),message->eventid(),message->process());
        if(message->process() == 1)
        {
            Fir::logger->info("awrd desc: size : %u",message->award_size());
            for(int i= 0; i != message->award_size();i++)
            {
                const  HelloKittyMsgData::Award &raward = message->award(i);
                Fir::logger->info("type is %u,value is %u",raward.awardtype(),raward.awardval());
            }

        }


    }
    return true;
}

bool LoginCmdHandle::AckNoticeChat(LoadClient* task,const HelloKittyMsgData::AckNoticeChat *message)
{
    Fir::logger->info("task:%d(charid %lu) rev sendplayer %s(char %lu channel %d) chat: %s",task->getAccount(),task->getcharid(),message->sendplayer().playername().c_str(),message->sendplayer().playerid(),message->channel(),message->chattxt().txt().c_str());
    return true;
}

bool LoginCmdHandle::AckGM(LoadClient* task,const HelloKittyMsgData::AckGM *message)
{
    Fir::logger->info("task:%d(charid %lu) rev Gm notice %s",task->getAccount(),task->getcharid(),message->ret().c_str());
    return true;

}

bool LoginCmdHandle::AckReqCreateFamily(LoadClient* task,const HelloKittyMsgData::AckReqCreateFamily *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckReqCreateFamily result %d",task->getAccount(),task->getcharid(),message->result());
    return true;
}

bool LoginCmdHandle::AckReqAddFamily(LoadClient* task,const HelloKittyMsgData::AckReqAddFamily *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckReqAddFamily result %d,familyid %lu,familiname %s",task->getAccount(),task->getcharid(),message->result(),message->familyid(),message->familyname().c_str());

    return true;
}

bool LoginCmdHandle::AckReqselfFamilyInfo(LoadClient* task,const HelloKittyMsgData::AckReqselfFamilyInfo *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckReqselfFamilyInfo result %d",task->getAccount(),task->getcharid(),message->result());
    if(message->result() == HelloKittyMsgData::FamilyOpResult_Suc)
        show(message->selfinfo());

    return true;
}

bool LoginCmdHandle::AckFinishFamilyOrder(LoadClient* task,const HelloKittyMsgData::AckFinishFamilyOrder *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckFinishFamilyOrder result %d order %d ",task->getAccount(),task->getcharid(),message->result(),message->orderid());
    if(message->result() == HelloKittyMsgData::FamilyOpResult_Suc) 
    {
        Fir::logger->info("awrd desc: size : %u",message->award_size());
        for(int i= 0; i != message->award_size();i++)
        {
            const  HelloKittyMsgData::Award &raward = message->award(i);
            Fir::logger->info("type is %u,value is %u",raward.awardtype(),raward.awardval());
        }

    }

    return true;
}

bool LoginCmdHandle::AckGetlastFamilyAward(LoadClient* task,const HelloKittyMsgData::AckGetlastFamilyAward *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckGetlastFamilyAward result %d  ",task->getAccount(),task->getcharid(),message->result());
    if(message->result() == HelloKittyMsgData::FamilyOpResult_Suc) 
    {
        Fir::logger->info("allaward desc: size : %u",message->allaward_size());
        for(int i= 0; i != message->allaward_size();i++)
        {
            const  HelloKittyMsgData::Award &raward = message->allaward(i);
            Fir::logger->info("type is %u,value is %u",raward.awardtype(),raward.awardval());
        }
        Fir::logger->info("selfaward desc: size : %u",message->selfaward_size());
        for(int i= 0; i != message->selfaward_size();i++)
        {
            const  HelloKittyMsgData::Award &raward = message->selfaward(i);
            Fir::logger->info("type is %u,value is %u",raward.awardtype(),raward.awardval());
        }

    }

    return true;
}

void LoginCmdHandle::show(const HelloKittyMsgData::FamilyInfo &rInfo)
{
    Fir::logger->info("------------begin show FamilyInfo---------------");
    Fir::logger->info("------------base---------------");
    const HelloKittyMsgData::BaseFamilyInfo &rBase = rInfo.baseinfo();
    Fir::logger->info("familyid %lu,familyname %s,familyicon %d,personnum %d,relation %d,totalscore %d,familylevel %d,ranking %d,lastranking %d",rBase.familyid(),rBase.familyname().c_str(),rBase.familyicon(),rBase.personnum(),rBase.relation(),rBase.totalscore(),rBase.familylevel(),rBase.ranking(),rBase.lastranking());
    Fir::logger->info("------------member---------------");  
    for(int i =0 ; i != rInfo.vecmember_size();i++)
    {
        const HelloKittyMsgData::FamilyMember &rMember = rInfo.vecmember(i);
        Fir::logger->info("charid %lu,name %s,level %d,job %d,todaycontribution %d",rMember.playershow().playerid(), rMember.playershow().playername().c_str(),rMember.playershow().playerlevel(),rMember.job(),rMember.todaycontribution());
    }
    Fir::logger->info("------------order---------------");  
    for(int i =0 ; i != rInfo.vecorder_size();i++)
    {
        Fir::logger->info("orderid %d",rInfo.vecorder(i));
    }
    Fir::logger->info("------------contribution---------------");
    Fir::logger->info("totalcontributionlast %d selfcontributionlastrank %d selfcontributionlast %d selfisgetaward %d",rInfo.totalcontributionlast(),rInfo.selfcontributionlastrank(),rInfo.selfcontributionlast(),rInfo.selfisgetaward()); 
    Fir::logger->info("------------end show FamilyInfo---------------");


}

bool LoginCmdHandle::AckServerNotice(LoadClient* task,const HelloKittyMsgData::AckServerNotice *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckServerNotice num :%d",task->getAccount(),task->getcharid(),message->sysinfo_size());
    for(int  i = 0; i != message->sysinfo_size();i++)
    {
        Fir::logger->info(" ID %lu  notice %s  time %d",message->sysinfo(i).id(),message->sysinfo(i).chattxt().c_str(),message->sysinfo(i).sendtime());


    }
    return true;

}

bool LoginCmdHandle::AckAddServerNotice(LoadClient* task,const HelloKittyMsgData::AckAddServerNotice *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckAddServerNotice ID %lu  notice %s  time %d",task->getAccount(),task->getcharid(),message->sysinfo().id(),message->sysinfo().chattxt().c_str(),message->sysinfo().sendtime());
    return true;

}

bool LoginCmdHandle::AckDelServerNotice(LoadClient* task,const HelloKittyMsgData::AckDelServerNotice *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckDelServerNotice ID %lu  ",task->getAccount(),task->getcharid(),message->id());
    return true;
}
bool LoginCmdHandle::AckSetRoleName(LoadClient* task,const HelloKittyMsgData::AckSetRoleName *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckSetRoleName result %u  ",task->getAccount(),task->getcharid(),message->result());
    return true;

}

bool LoginCmdHandle::Acksetguidefinish(LoadClient* task,const HelloKittyMsgData::Acksetguidefinish *message)
{
    Fir::logger->info("task:%d(charid %lu) rev Acksetguidefinish nextguideid %u  ",task->getAccount(),task->getcharid(),message->nextguideid());
    return true;

}

bool LoginCmdHandle::AckLeaveMessage(LoadClient* task,const HelloKittyMsgData::AckLeaveMessage *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckLeaveMessage result %u  ",task->getAccount(),task->getcharid(),message->result());
    return true;


}

void LoginCmdHandle::show(const HelloKittyMsgData::ClientChatMessage &rInfo)
{
    Fir::logger->info("sendname %s,messgeid %u,sendid %lu,chattxt: %s,timer %u ",rInfo.sendplayer().playername().c_str(),rInfo.message().messgeid(),rInfo.message().sendid(),rInfo.message().chattxt().c_str(),rInfo.message().timer());
}

bool LoginCmdHandle::AckAddMessage(LoadClient* task,const HelloKittyMsgData::AckAddMessage *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckAddMessage",task->getAccount(),task->getcharid());
    show(message->message());

    return true;
}
bool LoginCmdHandle::AckDelMessage(LoadClient* task,const HelloKittyMsgData::AckDelMessage *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckDelMessage messgeid %u ",task->getAccount(),task->getcharid(),message->messgeid());
    return true;

}


bool LoginCmdHandle::AckRandToy(LoadClient* task,const HelloKittyMsgData::AckRandToy *message)
{
    Fir::logger->info("task:%d(charid %lu) rev AckRandToy  ",task->getAccount(),task->getcharid());
    return true;

}
bool LoginCmdHandle::AckHeartBeat(LoadClient* task,const HelloKittyMsgData::AckHeartBeat *message)
{
    Fir::logger->debug("task:%d(charid %lu) rev AckHeartBeat  ",task->getAccount(),task->getcharid());
    HelloKittyMsgData::ReqHeartBeat req;
    std::string ret; 
    if(encodeMessage(&req,ret)) 
        task->sendCmd(ret.c_str(), ret.size()); 

    return true;

}
#define PrintMsg(PROTO)  Fir::logger->info("task:%d(charid %lu) rev  %s ",task->getAccount(),task->getcharid(),#PROTO);
bool LoginCmdHandle::ACKAllUnitBuildInfo(LoadClient* task,const HelloKittyMsgData::ACKAllUnitBuildInfo *message)
{
    PrintMsg(ACKAllUnitBuildInfo);
    Fir::logger->info("message->allcolinfo_size() is %d",message->allcolinfo_size());
    for(int i= 0; i < message->allcolinfo_size();i++)
    {
        Fir::logger->info("message->allunitbuild %d :",i);
        const HelloKittyMsgData::UnitColInfoForCli &rAllInfo = message->allcolinfo(i);

        Fir::logger->info("col id %u",rAllInfo.playercolid());
        Fir::logger->info("col state %u",rAllInfo.state());
        switch(rAllInfo.state())
        {
            case HelloKittyMsgData::UnitColInfoForCliState_None:
                {
                }
                break;
            case HelloKittyMsgData::UnitColInfoForCliState_Open:
                {
                    const HelloKittyMsgData::UnitPlayerColId& rinfo = rAllInfo.selfinfo();
                    Fir::logger->info("PlayerColId %u,SelfUnitGridId %u, Usetimes %u,TimerOut %u",rinfo.playercolid(),rinfo.selfunitgridid(),rinfo.usetimes(),rinfo.timerout());

                }
                break;
            default:
                const HelloKittyMsgData::UnitRunInfoForCli& rinfo = rAllInfo.unitinfo();
                Fir::logger->info("serverinfo UnitOnlyId %lu,paytype %s,InvitePlayer %lu,InvitePlayerColId %u,InviteSpeedTimes %u,ByInvitePlayer %lu,ByInvitePlayerColId %u,ByInviteSpeedTimes %u,LastSpeedTimer %u,InviteUnitGridId %u,TimerOut %u,UnitBuildId %u ,UnitScore %u,UnitLevel %u, UnitLastCheckTimer %u ,state %u, othername %s",rinfo.serverinfo().unitonlyid(),rinfo.serverinfo().paytype() == HelloKittyMsgData::PayType_Self ? "self" : "other",rinfo.serverinfo().inviteplayer(),rinfo.serverinfo().inviteplayercolid(),rinfo.serverinfo().invitespeedtimes(),rinfo.serverinfo().byinviteplayer(),rinfo.serverinfo().byinviteplayercolid(),rinfo.serverinfo().byinvitespeedtimes(),rinfo.serverinfo().lastspeedtimer(),rinfo.serverinfo().inviteunitgridid(),rinfo.serverinfo().timerout(),rinfo.serverinfo().unitbuildid(),rinfo.serverinfo().unitscore(),rinfo.serverinfo().unitlevel(),rinfo.serverinfo().unitlastchecktimer(),rinfo.serverinfo().state(),rinfo.othershow().playername().c_str());
                Fir::logger->info("gift size is %u",rinfo.serverinfo().vecaward().award_size());
                for(int j= 0;j < rinfo.serverinfo().vecaward().award_size();j++)
                {
                    const HelloKittyMsgData::Award& rInfo = rinfo.serverinfo().vecaward().award(j);
                    Fir::logger->info("gift %d , id is %u ,num id %u",j,rInfo.awardtype(),rInfo.awardval()); 
                }
                break;

        }


    }
    return true;
}

bool LoginCmdHandle::ACKOpUnitBuild(LoadClient* task,const HelloKittyMsgData::ACKOpUnitBuild *message)
{
    PrintMsg(ACKOpUnitBuild);
    Fir::logger->info("Result %u",message->result());
    return true; 
}


bool LoginCmdHandle::ACKUpdateUnitBuild(LoadClient* task,const HelloKittyMsgData::ACKUpdateUnitBuild *message)
{
    PrintMsg(ACKUpdateUnitBuild);
    const HelloKittyMsgData::UnitColInfoForCli &rAllInfo = message->colinfo();

    Fir::logger->info("col id %u",rAllInfo.playercolid());
    Fir::logger->info("col state %u",rAllInfo.state());
    switch(rAllInfo.state())
    {
        case HelloKittyMsgData::UnitColInfoForCliState_None:
            {
            }
            break;
        case HelloKittyMsgData::UnitColInfoForCliState_Open:
            {
                const HelloKittyMsgData::UnitPlayerColId& rinfo = rAllInfo.selfinfo();
                Fir::logger->info("PlayerColId %u,SelfUnitGridId %u, Usetimes %u,TimerOut %u",rinfo.playercolid(),rinfo.selfunitgridid(),rinfo.usetimes(),rinfo.timerout());

            }
            break;
        default:
            const HelloKittyMsgData::UnitRunInfoForCli& rinfo = rAllInfo.unitinfo();
            Fir::logger->info("serverinfo UnitOnlyId %lu,paytype %s,InvitePlayer %lu,InvitePlayerColId %u,InviteSpeedTimes %u,ByInvitePlayer %lu,ByInvitePlayerColId %u,ByInviteSpeedTimes %u,LastSpeedTimer %u,InviteUnitGridId %u,TimerOut %u,UnitBuildId %u ,UnitScore %u,UnitLevel %u, UnitLastCheckTimer %u ,state %u, othername %s",rinfo.serverinfo().unitonlyid(),rinfo.serverinfo().paytype() == HelloKittyMsgData::PayType_Self ? "self" : "other",rinfo.serverinfo().inviteplayer(),rinfo.serverinfo().inviteplayercolid(),rinfo.serverinfo().invitespeedtimes(),rinfo.serverinfo().byinviteplayer(),rinfo.serverinfo().byinviteplayercolid(),rinfo.serverinfo().byinvitespeedtimes(),rinfo.serverinfo().lastspeedtimer(),rinfo.serverinfo().inviteunitgridid(),rinfo.serverinfo().timerout(),rinfo.serverinfo().unitbuildid(),rinfo.serverinfo().unitscore(),rinfo.serverinfo().unitlevel(),rinfo.serverinfo().unitlastchecktimer(),rinfo.serverinfo().state(),rinfo.othershow().playername().c_str());
            Fir::logger->info("gift size is %u",rinfo.serverinfo().vecaward().award_size());
            for(int j= 0;j < rinfo.serverinfo().vecaward().award_size();j++)
            {
                const HelloKittyMsgData::Award& rInfo = rinfo.serverinfo().vecaward().award(j);
                Fir::logger->info("gift %d , id is %u ,num id %u",j,rInfo.awardtype(),rInfo.awardval()); 
            }
            break;

    }
    return true;

}

bool LoginCmdHandle::AckUnitbuildRank(LoadClient* task,const HelloKittyMsgData::AckUnitbuildRank *message)
{
    Fir::logger->info("rank size :%d, self rank :%d",message->rankinfo_size(),message->selfrank());
    for(int i = 0; i != message->rankinfo_size();i++)
    {
        const HelloKittyMsgData::UnitbuildRankInfo &rtep = message->rankinfo(i);
       Fir::logger->info("rank:%d , headleft :%lu ,leftcity :%s,leftborn :%s,lefttotalpopular :%d,lefttotalmaxpopular :%d,leftbuildid :%d,leftbuildlv:%d,headright:%lu,rightcity %s,rightborn %s",rtep.rank(),rtep.headleft().playerid(),rtep.leftcity().c_str(),rtep.leftborn().c_str(),rtep.lefttotalpopular(),rtep.lefttotalmaxpopular(),rtep.leftbuildid(), rtep.leftbuildlv(),rtep.headright().playerid(),rtep.rightcity().c_str(),rtep.rightborn().c_str());
    }
    return true;
}

bool LoginCmdHandle::AckSysNotice(LoadClient* task,const HelloKittyMsgData::AckSysNotice *message)
{
    Fir::logger->info("LoginCmdHandle::AckSysNotice");
    return true;
}
