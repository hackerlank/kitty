#include "Misc.h"
#include "tbx.h"
#include "SceneUser.h"
#include "extractProtoMsg.cpp"
#include "family.pb.h"
#include "SceneMail.h"
#include "SceneTaskManager.h"
DWORD ParamManager::GetSingleParam(eParam eparamType)
{
    const pb::Conf_t_param *pparam = tbx::param().get_base(eparamType);
    if(pparam)
    {
        return pparam->param->intparam();
    }
    return 0;
}

std::vector<DWORD> ParamManager::GetVecParam(eParam eparamType)
{
    const pb::Conf_t_param *pparam = tbx::param().get_base(eparamType);
    if(pparam)
    {
        return pparam->vecParam;
    }
    return std::vector<DWORD>();

}

void MiscManager::getAdditionalRewards(SceneUser *puser,AdditionalType type)
{
    const pb::Conf_t_buildOption *pConf = tbx::buildOption().get_base(type);
    if(pConf == NULL)
        return ;
    const HelloKittyMsgData::vecAward * pvecAward = pConf->getAward();
    if(pvecAward== NULL || pvecAward->award_size() == 0)
        return ;
    if(!puser->checkPush(*pvecAward))
    {
        SceneMailManager::getMe().sendSysMailToPlayerForAdditionalRewards(puser->charid,type,*pvecAward);
    }
    else
    {
        puser->pushItem(*pvecAward,"AdditionalRewards award");
        HelloKittyMsgData::AckGetAdditionalRewards ack;
        ack.set_additionalid(type);
        for(int i = 0; i != pvecAward->award_size();i++)
        {
            HelloKittyMsgData::Award* pAward = ack.add_additionalaward();
            if(pAward)
            {
                *pAward = pvecAward->award(i);
            }

        }
        std::string ret;
        encodeMessage(&ack,ret);
        puser->sendCmdToMe(ret.c_str(),ret.size());
    }

}

DWORD MiscManager::getMoneyForReduceTimer(eTimerReduce eType,DWORD timer)
{
    return pb::Conf_t_Gold::getClearCDCost(eType,timer);
}

DWORD MiscManager::getMoneyForCreateMaterial( DWORD Totalworth)
{
    return ceil((float)Totalworth/5);
}

DWORD MiscManager::getMoneyForExpandEarthOrWareHouse()
{
    return 12;
}

DWORD MiscManager::getMoneyForActiveMaterial()
{
    return 8;
}

void  MiscManager::SendSysNotice(QWORD charid,eSysNoticeId esysId,std::vector<HelloKittyMsgData::ReplaceWord> &vecArgs)
{
    HelloKittyMsgData::AckSysNotice ack;
    ack.set_noticeid(esysId);
    for(auto it = vecArgs.begin();it != vecArgs.end();it++)
    {
        HelloKittyMsgData::ReplaceWord * pParam = ack.add_strreplace();
        if(pParam)
        {
            *pParam = *it;
        }
    }
    std::string ret;
    encodeMessage(&ack,ret);
    SceneTaskManager::getMe().broadcastUserCmdToGateway(charid,ret.c_str(),ret.size());
}
