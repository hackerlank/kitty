#include "TradeCmdDispatcher.h"
#include "atlasManager.h"
#include "SceneUser.h"

bool TradeCmdHandle::reqAtlas(SceneUser* user,const HelloKittyMsgData::ReqAtlas *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    
    return user->m_atlasManager.flushAtlas();
}

bool TradeCmdHandle::reqDress(SceneUser* user,const HelloKittyMsgData::ReqDress *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_dressManager.flushDress();
}

bool TradeCmdHandle::reqOpDress(SceneUser* user,const HelloKittyMsgData::ReqOpDress *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    if(cmd->optype() == HelloKittyMsgData::DOT_UpLevel)
    {
        return user->m_dressManager.upLevel(cmd->id());
    }
    else if(cmd->optype() == HelloKittyMsgData::DOT_Change)
    {
        return user->m_dressManager.changeDress(cmd->id());
    }
    return false;
}

bool TradeCmdHandle::reqPaper(SceneUser* user,const HelloKittyMsgData::ReqPaper *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_paperManager.flushPaper();
}

bool TradeCmdHandle::reqOpPaper(SceneUser* user,const HelloKittyMsgData::ReqOpPaper *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_paperManager.produce(cmd->paper());
}
