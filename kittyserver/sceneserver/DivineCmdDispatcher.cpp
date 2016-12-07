#include "TradeCmdDispatcher.h"
#include "SceneUser.h"

bool TradeCmdHandle::reqDivineInfo(SceneUser* user,const HelloKittyMsgData::ReqDivineInfo *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    
    return user->flushDivine();
}

bool TradeCmdHandle::reqDivineFristNotice(SceneUser* user,const HelloKittyMsgData::ReqDivineNotice *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->divineNotice();
}

bool TradeCmdHandle::reqDivine(SceneUser* user,const HelloKittyMsgData::ReqDivine *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->divine(cmd->notify());
}

bool TradeCmdHandle::reqDivineVerify(SceneUser* user,const HelloKittyMsgData::ReqDivineVerify *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->divineVerify(cmd->older());
}

bool TradeCmdHandle::reqBurstEvent(SceneUser* user,const HelloKittyMsgData::ReqBurstEvent *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return cmd->tempid() ? user->m_burstEventManager.updateBurstEvent(cmd->tempid()) : user->m_burstEventManager.flushEvent();
}

bool TradeCmdHandle::reqOpBurstEvent(SceneUser* user,const HelloKittyMsgData::ReqOpBurstEvent *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_burstEventManager.opEvent(cmd);
}
