#include "TradeCmdDispatcher.h"
#include "SceneUser.h"

bool TradeCmdHandle::reqEmail(SceneUser* u, const HelloKittyMsgData::ReqEmail *cmd)
{
    if (!u || !cmd)
    {
        return false;
    }
    return u->m_emailManager.reqEmail(cmd);
}

bool TradeCmdHandle::reqSendEmail(SceneUser* u, const HelloKittyMsgData::ReqSendEmail *cmd)
{
    if (!u || !cmd)
    {
        return false;
    }
    return u->m_emailManager.sendEmail(&(cmd->emailbase()));
}

bool TradeCmdHandle::reqOpEmail(SceneUser* u, const HelloKittyMsgData::ReqOpEmail *cmd)
{
    if (!u || !cmd)
    {
        return false;
    }
    return u->m_emailManager.opEmail(cmd);
}



