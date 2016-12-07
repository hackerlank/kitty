#include "GmToolDispatcher.h"

bool GmToolCmdHandle::reqTest(GmToolTask* task,const HelloKittyMsgData::ReqTest *message)
{
    if(!task || !message)
    {
        return false;
    }
    Fir::logger->debug("[GM工具] 收到测试(%u)",message->num());
    HelloKittyMsgData::AckTest ack;
    ack.set_num(message->num());
    std::string ret;
    encodeMessage(&ack,ret);
    return task->sendCmd(ret.c_str(),ret.size());
}

bool GmToolCmdHandle::reqGmLogin(GmToolTask* task,const HelloKittyMsgData::ReqGmLogin *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->login(message->account().c_str(),message->passwd().c_str());
}

bool GmToolCmdHandle::reqGmModifypasswd(GmToolTask* task,const HelloKittyMsgData::ReqGmModifypasswd *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->motifyPasswd(message->account().c_str(),message->passwd().c_str(),message->newpasswd().c_str());
}

bool GmToolCmdHandle::reqGmShowGmAccount(GmToolTask* task,const HelloKittyMsgData::ReqGmShowGmAccount *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->showAllGm();
}

bool GmToolCmdHandle::reqModifyGmData(GmToolTask* task,const HelloKittyMsgData::ReqModityGmData *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->modifyGm(message);
}

bool GmToolCmdHandle::reqModifyUserAttr(GmToolTask* task,const HelloKittyMsgData::ReqModifyUserAttr *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->modifyUserAttr(message);
}

bool GmToolCmdHandle::reqModifyUserBuild(GmToolTask* task,const HelloKittyMsgData::ReqModifyUserBuild *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->modifyUserBuild(message);
}

bool GmToolCmdHandle::reqForbid(GmToolTask* task,const HelloKittyMsgData::ReqForbid *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->reqForbid(message);
}

bool GmToolCmdHandle::reqSendEmail(GmToolTask* task,const HelloKittyMsgData::ReqGmToolSendEmail *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->reqSendEmail(message);
}

bool GmToolCmdHandle::reqOpNotice(GmToolTask* task,const HelloKittyMsgData::ReqOpNotice *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->reqOpNotice(message);
}

bool GmToolCmdHandle::reqModifyGiftStore(GmToolTask* task,const HelloKittyMsgData::ReqModifyGiftStore *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->modifyGiftStore(message);
}

bool GmToolCmdHandle::ReqAddPlayerActive(GmToolTask* task,const HelloKittyMsgData::ReqAddPlayerActive *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->ReqAddPlayerActive(message);

}
bool GmToolCmdHandle::ReqModifyPlayerActive(GmToolTask* task,const HelloKittyMsgData::ReqModifyPlayerActive *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->ReqModifyPlayerActive(message);

}

bool GmToolCmdHandle::ReqOpenActive(GmToolTask* task,const HelloKittyMsgData::ReqOpenActive *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->ReqOpenActive(message);

}

bool GmToolCmdHandle::ReqDelUserPicture(GmToolTask* task,const HelloKittyMsgData::ReqDelUserPicture *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->delUserPicture(message);
}

bool GmToolCmdHandle::ReqSendGlobalEmail(GmToolTask* task,const HelloKittyMsgData::ReqSendGlobalEmail *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->sendGlobalEmail(message);
}

bool GmToolCmdHandle::ReqModifyGiftInfo(GmToolTask* task,const HelloKittyMsgData::ReqModifyGiftInfo *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->opGiftInfo(message);
}

bool GmToolCmdHandle::ReqModifyUserVerify(GmToolTask* task,const HelloKittyMsgData::ReqModifyUserVerify *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->modifyUserVerify(message);
}

bool GmToolCmdHandle::ReqAddActiveCode(GmToolTask* task,const HelloKittyMsgData::ReqAddActiveCode *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->reqAddActiveCode(message);
}




