#include "TradeCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "TradeCommand.h"
#include "serialize.pb.h"
#include "extractProtoMsg.h"

bool TradeCmdHandle::useItem(SceneUser* u, const HelloKittyMsgData::ReqUseItem* cmd)
{
	if (!u || !cmd) 
    {
        return false;
    }
    //return u->m_store_house.useItem(cmd->itemid(),cmd->num());
    return false;
}

