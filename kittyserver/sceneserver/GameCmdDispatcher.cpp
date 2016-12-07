#include "TradeCmdDispatcher.h"
#include "SceneUser.h"

bool TradeCmdHandle::reqBeginStar(SceneUser* user,const HelloKittyMsgData::ReqBeginStar *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->beginStar(cmd->step());
}

bool TradeCmdHandle::reqCommitStar(SceneUser* user,const HelloKittyMsgData::ReqStarCommitStep *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->reqCommitStar(cmd);
}

bool TradeCmdHandle::reqBeginSlot(SceneUser* user,const HelloKittyMsgData::ReqBeginSlot *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->beginSlotMachine(cmd);
}

bool TradeCmdHandle::reqBeginMacro(SceneUser* user,const HelloKittyMsgData::ReqBeginMacro *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->beginMacro(cmd);
}

bool TradeCmdHandle::reqBeginSuShi(SceneUser* user,const HelloKittyMsgData::ReqBeginSuShi *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->beginSuShi();
}

bool TradeCmdHandle::reqCommitStepData(SceneUser* user,const HelloKittyMsgData::ReqCommitStep *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->reqCommitSuShi(cmd);
}

bool TradeCmdHandle::reqStartGame(SceneUser* user,const HelloKittyMsgData::ReqStartGame *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->startGame(cmd);
}

bool TradeCmdHandle::reqJoinStartGame(SceneUser* user,const HelloKittyMsgData::ReqJoinStartGame *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->ackJoinStartGame(cmd);
}

bool TradeCmdHandle::reqCancelStartGame(SceneUser* user,const HelloKittyMsgData::ReqCancelStar *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->cancelStarGame(cmd->reason());
}

bool TradeCmdHandle::reqSyncStar(SceneUser* user,const HelloKittyMsgData::ReqSyncStar *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->syncGame(cmd);
}

bool TradeCmdHandle::reqReadyGame(SceneUser* user,const HelloKittyMsgData::ReqReadyGame *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->ackReadyGame();
}

