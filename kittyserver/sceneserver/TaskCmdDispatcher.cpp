#include "TradeCmdDispatcher.h"
#include "taskManager.h"
#include "SceneUser.h"

bool TradeCmdHandle::reqTask(SceneUser* u, const HelloKittyMsgData::ReqAllTask* cmd)
{
    if(!u || !cmd)
    {
        return false;
    }
    return u->m_taskManager.flushAllTask(cmd->tasktype());
}

bool TradeCmdHandle::commitTask(SceneUser* u, const HelloKittyMsgData::ReqSubmitTask* cmd)
{
    if(!u || !cmd)
    {
        return false;
    }
    return u->m_taskManager.rewardTask(cmd->id());
}
