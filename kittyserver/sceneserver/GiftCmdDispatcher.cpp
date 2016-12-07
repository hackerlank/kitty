#include "TradeCmdDispatcher.h"
#include "SceneUser.h"

bool TradeCmdHandle::reqGiftOp(SceneUser* user,const HelloKittyMsgData::ReqOpGift* cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    
    return user->m_giftPackage.reqGiftOp(cmd);
}

bool TradeCmdHandle::reqUpdate(SceneUser* user,const HelloKittyMsgData::ReqUpdate *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_giftPackage.reqUpdate(cmd);
}

bool TradeCmdHandle::reqAddress(SceneUser* user,const HelloKittyMsgData::ReqAddress* cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    
    return user->m_addressManager.update();
}

bool TradeCmdHandle::reqChangeAddress(SceneUser* user,const HelloKittyMsgData::ReqChangeAddress *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_addressManager.changeAddress(cmd->address());
}

bool TradeCmdHandle::reqCashGift(SceneUser* user,const HelloKittyMsgData::ReqCashGift* cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    
    return user->m_giftPackage.reqCashGift(cmd);
}

bool TradeCmdHandle::reqCommitGift(SceneUser* user,const HelloKittyMsgData::ReqCommitGift *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_giftPackage.reqCashGift(cmd);
}

bool TradeCmdHandle::reqPhyCondInfo(SceneUser* user,const HelloKittyMsgData::ReqPhyCondInfo *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_giftPackage.updatePhyCondInfo(cmd->id());
}

bool TradeCmdHandle::reqSendFlower(SceneUser* user,const HelloKittyMsgData::ReqSendFlower *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_giftPackage.sendFlower(cmd);
}

bool TradeCmdHandle::reqSendVirtualGift(SceneUser* user,const HelloKittyMsgData::ReqSendVirtualGift *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->sendVirtualGift(cmd);
}

