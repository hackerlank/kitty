#include "PlayerActiveCmdDispatcher.h"
#include "SceneUser.h"
#include "extractProtoMsg.h"
 //获取所有活动
bool PlayerActiveCmdHandle::ReqgetPlayerActiveList(SceneUser* u, const HelloKittyMsgData::ReqgetPlayerActiveList* cmd)
{
    u->m_active.ReqgetPlayerActiveList(cmd);
    return true;
}

//获取活动奖励
bool PlayerActiveCmdHandle::ReqgetActiveAward(SceneUser* u, const HelloKittyMsgData::ReqgetActiveAward* cmd)
{
    u->m_active.ReqgetActiveAward(cmd);
    return true;

}




