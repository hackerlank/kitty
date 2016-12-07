#include "TradeCmdDispatcher.h"
#include "SceneUser.h"

bool TradeCmdHandle::reqRank(SceneUser* user,const HelloKittyMsgData::ReqRank *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->reqRank(cmd);
}

