#include "SignInCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "extractProtoMsg.h"
#include "SceneTaskManager.h"
#include "SceneCommand.h"
#include "TimeTick.h"
#include "Misc.h"
#include "tbxbase.h"
 bool SignInCmdHandle::ReqGetSignInData(SceneUser* u, const HelloKittyMsgData::ReqGetSignInData* cmd)
{
    u->m_managersignin.checkinfo(false);
    u->m_managersignin.updatetocli();
    return true;

}

bool SignInCmdHandle::ReqSignIn(SceneUser* u, const HelloKittyMsgData::ReqSignIn* cmd)
{
    u->m_managersignin.checkinfo(true);
    HelloKittyMsgData::AckSignIn ack;
    HelloKittyMsgData::SignInResult result = HelloKittyMsgData::SignInResult_Suc;
    do{
        if(u->m_managersignin.hasSignIn())
        {
            result =  HelloKittyMsgData::SignInResult_HasSign;
            break;
        }
        DWORD curDay = u->m_managersignin.getSignDay();
        
       const pb::Conf_t_SignInEveryDay* base = tbx::SignInEveryDay().get_base(curDay);
       if(base == NULL)
       {
           result =  HelloKittyMsgData::SignInResult_OtherERR;
           break;
       }
       if(!u->checkPush(base->awarditem))
       {
           result =  HelloKittyMsgData::SignInResult_FullPacket;
           break;
       }
       u->pushItem(base->awarditem,"Signeveryday award");
       u->m_managersignin.signIn();
    }while(0);
    ack.set_result(result);
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());

    return true;
}

bool SignInCmdHandle::ReqGetTotalAward(SceneUser* u, const HelloKittyMsgData::ReqGetTotalAward* cmd)
{
    u->m_managersignin.checkinfo(true);
    HelloKittyMsgData::AckGetTotalAward ack;
    HelloKittyMsgData::SignInResult result = HelloKittyMsgData::SignInResult_Suc;
    do{
        if(u->m_managersignin.hasGetAward(cmd->id()))
        {
            result =  HelloKittyMsgData::SignInResult_GetAward;
            break;
        }
       const pb::Conf_t_SignInTotalDay* base = tbx::SignInTotalDay().get_base(cmd->id());
       if(base == NULL)
       {
           result =  HelloKittyMsgData::SignInResult_AwardIDErr;
           break;
       }
       if(u->m_managersignin.getMonthDays() < base->SignInTotalDay->days())
       {
           result =  HelloKittyMsgData::SignInResult_SignDaysLimit;
           break;
       }
       if(!u->checkPush(base->awarditem))
       {
           result =  HelloKittyMsgData::SignInResult_FullPacket;
           break;
       }
       
       u->pushItem(base->awarditem,"SignInTotal award");
       u->m_managersignin.setAward(cmd->id());
    }while(0);
    ack.set_result(result);
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}
