#include "SceneMail.h"
#include "emailManager.h"
#include "tbx.h"


void SceneMailManager::sendSysMailToPlayerForEvent(QWORD PlayerId,DWORD eventId,QWORD OwerId,const HelloKittyMsgData::vecAward& rvecAward)
{
    Fir::logger->info("send mail to %lu , event %u ,owergarden %lu",PlayerId,eventId,OwerId);
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_FriendEvent_ID);
    if(emailConf == NULL)
        return ;
    const pb::Conf_t_event *eventConf = tbx::event().get_base(eventId);
    if(eventConf == NULL)
        return ;
    std::map<DWORD,DWORD> couponsMap; //item
    for(int i = 0; i != rvecAward.award_size();i++)
    {
        const HelloKittyMsgData::Award& raward = rvecAward.award(i);
        couponsMap[raward.awardtype()] = raward.awardval();

    }
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_Language);
    Arg.set_value(eventConf->event->name());
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(PlayerId,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);

}

void SceneMailManager::sendSysMailToPlayerForAdditionalRewards(QWORD PlayerId,DWORD AdditionalID,const HelloKittyMsgData::vecAward& rvecAward)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_AdditionalReward_ID);
    if(emailConf == NULL)
        return ;
    const pb::Conf_t_buildOption *buildOptionConf = tbx::buildOption().get_base(AdditionalID);
    if(buildOptionConf == NULL)
        return ;
    std::map<DWORD,DWORD> couponsMap; //item
    for(int i = 0; i != rvecAward.award_size();i++)
    {
        const HelloKittyMsgData::Award& raward = rvecAward.award(i);
        couponsMap[raward.awardtype()] = raward.awardval();

    }
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_Language);
    Arg.set_value(buildOptionConf->buildOption->name());
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(PlayerId,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);

}

void SceneMailManager::sendSysMailToPlayerForConfirmLoad(QWORD PlayerId,const std::string& ownername  ,const HelloKittyMsgData::vecAward& rvecAward)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_LoadTrainOther_Confirm);
    if(emailConf == NULL)
        return ;
    std::map<DWORD,DWORD> couponsMap; //item
    for(int i = 0; i != rvecAward.award_size();i++)
    {
        const HelloKittyMsgData::Award& raward = rvecAward.award(i);
        couponsMap[raward.awardtype()] = raward.awardval();
    }
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(ownername);
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(PlayerId,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);


}

void SceneMailManager::sendSysMailToPlayerForReturnUnityBuild(QWORD PlayerId,const std::string& Othername,const std::string&buildname,const std::map<DWORD,DWORD>& couponsMap)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Unity_InvityFal_Return);
    if(emailConf == NULL)
        return ;
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(Othername);
    vecArg.push_back(Arg);
    Arg.set_key(HelloKittyMsgData::ReplaceType_Language);
    Arg.set_value(buildname);
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(PlayerId,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);


}

void SceneMailManager::sendSysMailToPlayerCancelUnityBuild(QWORD charid,QWORD otherPlayer,const std::string& strself,const std::string& strother,const std::string& strBuildName,DWORD unitlevel,const std::string& strcancel)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Unity_Cancel);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strself);
    vecArg.push_back(Arg);

    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strother);
    vecArg.push_back(Arg);

    Arg.set_key(HelloKittyMsgData::ReplaceType_Language);
    Arg.set_value(strBuildName);
    vecArg.push_back(Arg);

    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strcancel);
    vecArg.push_back(Arg);

    char levelbuf[255];
    snprintf(levelbuf,255,"%d",unitlevel);

    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(std::string(levelbuf));
    vecArg.push_back(Arg);

    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);
    EmailManager::sendEmailBySys(otherPlayer,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);


}

void SceneMailManager::sendSysMailToPlayerFinishUnityBuild(QWORD charid,QWORD otherPlayer,const std::string& strself,const std::string& strother,const std::string& strBuildName,DWORD unitlevel)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Unity_Finish);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strself);
    vecArg.push_back(Arg);

    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strother);
    vecArg.push_back(Arg);

    Arg.set_key(HelloKittyMsgData::ReplaceType_Language);
    Arg.set_value(strBuildName);
    vecArg.push_back(Arg);

    char levelbuf[255];
    snprintf(levelbuf,255,"%d",unitlevel);

    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(std::string(levelbuf));
    vecArg.push_back(Arg);

    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);
    EmailManager::sendEmailBySys(otherPlayer,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);

}

void SceneMailManager::sendSysMailToPlayerInviteBuildNeedAgree(QWORD charid,const std::string& strBuildName,const std::string& strother)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_InviteBuild_NeedAgree);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strother);
    vecArg.push_back(Arg);
    Arg.set_key(HelloKittyMsgData::ReplaceType_Language);
    Arg.set_value(strBuildName);
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);

}

void SceneMailManager::sendSysMailToPlayerInviteBuildNotNeedAgree(QWORD charid,const std::string& strBuildName,const std::string& strother)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_InviteBuild_NotNeedAgree);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strother);
    vecArg.push_back(Arg);
    Arg.set_key(HelloKittyMsgData::ReplaceType_Language);
    Arg.set_value(strBuildName);
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);

}
void SceneMailManager::sendSysMailToPlayerApplyFamily(QWORD charid,const std::string& strother)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_ApplyFamily);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strother);
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);

}

void SceneMailManager::sendSysMailToPlayerJoinFamily(QWORD charid,const std::string& strother,const std::string& strfamily)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_JoinFamily);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strother);
    vecArg.push_back(Arg);
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strfamily);
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);

}

void SceneMailManager::sendSysMailToPlayerFriendJoinFamily(QWORD charid,const std::string& strother)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_FriendJionFamily);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strother);
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);


}

void SceneMailManager::sendSysMailToPlayerQuitFamily(QWORD charid,const std::string& strother)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_QuitFamily);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strother);
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);


}

void SceneMailManager::sendSysMailToPlayerLeaveFamily(QWORD charid,const std::string& strother)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_OtherLeaveFamily);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strother);
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);


}

void SceneMailManager::sendSysMailToPlayerRefuseJoin(QWORD charid,const std::string& strother)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_RefuseJoinFamily);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    Arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
    Arg.set_value(strother);
    vecArg.push_back(Arg);
    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);


}

void SceneMailManager::sendSysMailToPlayerKickFamily(QWORD charid)
{
    const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_KickoffFamily);
    if(emailConf == NULL)
        return ; 
    std::map<DWORD,DWORD> couponsMap; 
    std::vector<HelloKittyMsgData::ReplaceWord> vecArg;
    HelloKittyMsgData::ReplaceWord Arg;
    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),vecArg,couponsMap);
}


