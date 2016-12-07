#include "TradeCmdDispatcher.h"
#include "SceneUser.h"

bool TradeCmdHandle::reqToy(SceneUser* u, const HelloKittyMsgData::ReqOpToy* cmd)
{
	if (!u || !cmd) 
    {
        return false;
    }
    return u->randActiveToy(cmd);
}
