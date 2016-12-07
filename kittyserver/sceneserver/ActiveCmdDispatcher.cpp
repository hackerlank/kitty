#include "TradeCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneCommand.h"
#include "SysActiveManager.h"


bool TradeCmdHandle::reqJoinActive(SceneUser* user,const HelloKittyMsgData::ReqJoinActive* cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return false;
#if 0
    return user->joinActive(cmd->activeid());
#endif
}

bool TradeCmdHandle::reqActiveInfo(SceneUser* user,const HelloKittyMsgData::ReqAckActiveInfo* cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    HelloKittyMsgData::AckActiveInfo ack;
    SysActiveManager::getMe().getAllActive(ack);
    std::string ret;
    encodeMessage(&ack,ret);
    user->sendCmdToMe(ret.c_str(),ret.size());
    return true;
    /*
    CMD::SCENE::t_UserActiveInfo activeInfo;
    activeInfo.charID = user->charid;
    std::string ret;
    
    encodeMessage(&activeInfo,sizeof(activeInfo),ret);
    return user->sendCmdToGateway(ret.c_str(),ret.size());
    */
}
