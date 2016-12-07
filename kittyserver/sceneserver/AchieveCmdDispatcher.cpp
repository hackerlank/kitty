#include "TradeCmdDispatcher.h"
#include "SceneUser.h"
#include "achievementManager.h"

bool TradeCmdHandle::reqAchieve(SceneUser* u, const HelloKittyMsgData::ReqAllAchieve* cmd)
{
    if(!u || !cmd)
    {
        return false;
    }
    return u->m_achievementManager.flushAllAchieve();
}

bool TradeCmdHandle::commitAchieve(SceneUser* u, const HelloKittyMsgData::ReqSubmitAchieve* cmd)
{
    if(!u || !cmd)
    {
        return false;
    }
    return u->m_achievementManager.rewardAchieve(cmd->id());
}
