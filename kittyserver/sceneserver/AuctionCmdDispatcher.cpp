#include "TradeCmdDispatcher.h"
#include "SceneUser.h"


bool TradeCmdHandle::reqOpCenter(SceneUser* user,const HelloKittyMsgData::ReqOpCenter* cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    
    return user->opRecreationCenter(cmd->enter());
}

bool TradeCmdHandle::reqOpRoom(SceneUser* user,const HelloKittyMsgData::ReqOpRoom *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->opAuctionRoom(cmd->auctionid(),cmd->enter());
}

bool TradeCmdHandle::reqBid(SceneUser* user,const HelloKittyMsgData::ReqBid *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->auction(cmd->auctionid());
}

bool TradeCmdHandle::reqAutoBid(SceneUser* user,const HelloKittyMsgData::ReqAutoBid *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->setAutoAuction(cmd->auctionid(),cmd->cnt());
}

bool TradeCmdHandle::reqParchaseNormalGift(SceneUser* user,const HelloKittyMsgData::ReqBuyNormalGift *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->parchaseGift(cmd->giftid());
}

bool TradeCmdHandle::reqExchangeGiftNum(SceneUser* user,const HelloKittyMsgData::ReqExchangeGiftNum *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->updateExchangeNum();
}
