#include "TradeCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "TradeCommand.h"
#include "serialize.pb.h"
#include "extractProtoMsg.h"
#include "SceneToOtherManager.h"
#include "SceneTaskManager.h"

bool TradeCmdHandle::requireSallAddItem(SceneUser* u, const HelloKittyMsgData::ReqSallPutItem *cmd)
{
    if (!u || !cmd)
    {
        return false;
    }
    return u->m_trade.addSallItem(cmd);
}

bool TradeCmdHandle::requireSellPaper(SceneUser* u, const HelloKittyMsgData::ReqSellPaper *cmd)
{
    if (!u || !cmd)
    {
        return false;
    }
    return u->m_trade.requireCellPaper(cmd);
}

bool TradeCmdHandle::advertise(SceneUser* u, const HelloKittyMsgData::ReqAdvertise *cmd)
{
    if (!u || !cmd)
    {
        return false;
    }
    return u->m_trade.advertise(cmd->cellid());
}

bool TradeCmdHandle::reqSallSystem(SceneUser* u, const HelloKittyMsgData::ReqSallSystem *cmd)
{
    if (!u || !cmd)
    {
        return false;
    }
    return u->m_store_house.sallSystem(cmd->itemid(),cmd->itemcount());
}

bool TradeCmdHandle::reqOpCell(SceneUser* u, const HelloKittyMsgData::ReqOpCell *cmd)
{
    if(!u || !cmd || cmd->optype() > HelloKittyMsgData::Op_Type_Get_Money)
    {
        return false;
    }
    if(cmd->optype() == HelloKittyMsgData::Op_Type_Open_Cell)
    {
        return u->m_trade.openCeil();
    }
    else if(cmd->optype() == HelloKittyMsgData::Op_Type_Down_Item)
    {
        return u->m_trade.offSallItem(cmd->cellid());
    }
    else
    {
        return u->m_trade.reqGetCellMoney(cmd->cellid());
    }
}
bool TradeCmdHandle::ReqAddFriend(SceneUser* User,const HelloKittyMsgData::ReqAddFriend *message)
{
    HelloKittyMsgData::AckAddFriend ack;
    //不能加自己
    HelloKittyMsgData::ReqAddFriendReSult result =  HelloKittyMsgData::ReqAddFriendReSult_Suc;
    do{
        if(message->playerid() == User->charid)
        {
            result =  HelloKittyMsgData::ReqAddFriendReSult_Other;

            break;
        }
        if(User->getFriendManager().IsFriend(message->playerid()))
        {
            result =  HelloKittyMsgData::ReqAddFriendReSult_IsFriendAlready;
            break;

        }
        zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(message->playerid());
        if(!handle2)
        {
            result =  HelloKittyMsgData::ReqAddFriendReSult_NoPerson;

            break;
        }
        CharBase charbase;
        if (handle2->getBin("charbase",message->playerid(),"charbase",(char*)&charbase) == 0)
        {
            result =  HelloKittyMsgData::ReqAddFriendReSult_NoPerson;
            break;
        }
        if((User->getFriendManager().GetFriendSize() >= 999))
        {
            result =  HelloKittyMsgData::ReqAddFriendReSult_Full;
            break;
        }


    }while(0);
    ack.set_result(result);
    if(result == HelloKittyMsgData::ReqAddFriendReSult_Suc)
    {
        User->getFriendManager().AddFriend(message->playerid());
        User->addFriendNum(1);

    }
    HelloKittyMsgData::MemberRelation* pmember = ack.mutable_ackmember();
    User->getFriendManager().GetShowInfoForMe(message->playerid(),*pmember);
    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());

    return true;
}

bool TradeCmdHandle::ReqKickFriend(SceneUser* User,const HelloKittyMsgData::ReqKickFriend *message)
{
    HelloKittyMsgData::AckKickFriend ack;
    HelloKittyMsgData::ReqKickFriendReSult result = HelloKittyMsgData::ReqKickFriendReSult_Suc;
    do{
        if(!User->getFriendManager().IsFriend(message->playerid()))
        {
            result =  HelloKittyMsgData::ReqKickFriendReSult_NoFriend;
            break;
        }


    }while(0);
    ack.set_result(result);
    if(result == HelloKittyMsgData::ReqKickFriendReSult_Suc)
    {
        User->getFriendManager().KickFriend(message->playerid());

    }
    HelloKittyMsgData::MemberRelation* pmember = ack.mutable_ackmember();
    User->getFriendManager().GetShowInfoForMe(message->playerid(),*pmember);

    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool TradeCmdHandle::ReqRelationList(SceneUser* User,const HelloKittyMsgData::ReqRelationList  *message)
{
    //引导推荐
    //
    HelloKittyMsgData::AckRelationList ack;
    ack.set_listtype(message->listtype());
    ack.set_pageno(message->pageno());
    ack.set_pagemax(message->pagemax());

    vector<QWORD> vecPlayer;
    switch (message->listtype())
    {

        case HelloKittyMsgData::FriendList:
            {
                ack.set_allpageno(User->getFriendManager().GetFriendList(message->pageno(),message->pagemax(),vecPlayer,false,true));
            }
            break;
        case HelloKittyMsgData::FansList:
            {
                ack.set_allpageno(User->getFriendManager().GetFansList(message->pageno(),message->pagemax(),vecPlayer,false,true));

            }
            break;
        case HelloKittyMsgData::RadomList:
            {
                ack.set_allpageno(1);
                User->getFriendManager().GetOtherListOnline(message->pagemax(),vecPlayer,true,false,true);
            }
            break;
    }
    //加入系统推荐
    if(message->listtype() == HelloKittyMsgData::FriendList && message->pageno() == 0)
    {
        zMemDB* redishandleAll = zMemDBPool::getMe().getMemDBHandle();
        QWORD NPCID = 0;
        if(redishandleAll)
        {
            std::set<QWORD> setnpc;
            redishandleAll->getSet("npc", 0, "staticlevel", setnpc);
            bool bfind = false;
            for(auto it = setnpc.begin(); it != setnpc.end();it++)
            {
                if(User->charbase.level <= *it)
                {
                    bfind = true;
                    NPCID = redishandleAll->getInt("npc", *it, "staticid");
                    break;
                }

            }
            if(!bfind && !setnpc.empty())
            {
                 NPCID = redishandleAll->getInt("npc", *(setnpc.rbegin()), "staticid");

            }
            if(NPCID  > 0)
            {
                HelloKittyMsgData::MemberRelation *relation = ack.add_vecmember();
                User->getFriendManager().GetShowInfoForMe(NPCID,*relation);
            }

        }
    }

    for(auto iter = vecPlayer.begin(); iter != vecPlayer.end();iter++)
    {
        HelloKittyMsgData::MemberRelation *relation = ack.add_vecmember();
        User->getFriendManager().GetShowInfoForMe(*iter,*relation);
    }
    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool TradeCmdHandle::ReqGetOnePerson(SceneUser* User,const HelloKittyMsgData::ReqGetOnePerson *message)
{
    HelloKittyMsgData::AckGetOnePerson ack;
    HelloKittyMsgData::MemberRelation *relation = ack.mutable_person();
    if(relation == NULL)
        return false;
    User->getFriendManager().GetShowInfoForMe(message->playerid(),*relation);
    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool TradeCmdHandle::ReqEnterGarden(SceneUser* User,const HelloKittyMsgData::ReqEnterGarden *message)
{
    if(ISSTATICNPC(message->charid()))
    {
        User->sendStaticNpc(message->charid());
        return true;
    }
    if(message->charid() == 0 || message->charid() == User->getvisit() || (User->charid == message->charid() && User->getvisit() == 0))
        return true;
    if(User->charid != message->charid())
    {
        do{
            zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(message->charid());
            if(!handle)
                break;
            CharBase    charbase;
            if (handle->getBin("charbase",message->charid(),"charbase",(char*)&(charbase)) == 0)
            {
                Fir::logger->error("账号不存在 %lu", message->charid());  
                break ;
            }

            if(!handle->getLock("playerlock",message->charid(),"newplayer",30))
            {
                break;
            }
            DWORD SenceId = handle->getInt("playerscene",message->charid(),"sceneid");
            if(SenceId > 0)
            {
                User->setVisit(message->charid());
                break;
            }
            //玩家不存在，创建角色
            SceneUser* u  =  SceneUserManager::getMe().CreateTempUser(message->charid());
            if (u)
            {
                User->setVisit(message->charid());
                break;

            }
            else
            {
                handle->delLock("playerlock",message->charid(),"newplayer");
            }

        }while(0);
    }
    else//访问自己家园
    {
        //退出别人家园
        User->setVisit(0);
    }

    return true;

}

bool TradeCmdHandle::opBuilding(SceneUser* User,const HelloKittyMsgData::opBuilding *message)
{
    HelloKittyMsgData::PlayerOpEventResult result = HelloKittyMsgData::PlayerOpEventResult_Suc;
    DWORD SenceId = 0;
    QWORD VisitID = User->getvisit();
    if(VisitID == 0)
    {
        VisitID = User->charid;
    }
    do{
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(VisitID);
        if(!handle)
        {
            result = HelloKittyMsgData::PlayerOpEventResult_OtherErr;
            break;
        }

        SenceId = handle->getInt("playerscene",VisitID,"sceneid");
        if(SenceId == 0)
        {
            result = HelloKittyMsgData::PlayerOpEventResult_OtherErr;
            break;

        }
    }while(0);
    if(result != HelloKittyMsgData::PlayerOpEventResult_Suc)
    {
        HelloKittyMsgData::AckopBuilding ack;
        ack.set_result(result);
        ack.set_charid(VisitID);
        HelloKittyMsgData::Builditype* pbulid =  ack.mutable_build();
        if(pbulid)
        {
            *pbulid = message->build();
        }
        std::string ret;
        encodeMessage(&ack,ret);
        User->sendCmdToMe(ret.c_str(),ret.size());
    }
    else
    {
        SceneUser* otherUser = SceneUserManager::getMe().getUserByID(VisitID); 
        if(otherUser)
        {
            otherUser->opBuild(User->charid,message->build());  
        }
        else
        {
            zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(VisitID);
            if(!handle)
            {
                return false;
            }
            CMD::SCENE::t_DoBulid_scenescene cmd;
            cmd.charid = User->charid;
            cmd.ownerid = VisitID;
            cmd.buildid = message->build().buildid();
            cmd.isIcon = message->build().isicon();
            std::string ret;
            encodeMessage(&cmd,sizeof(cmd),ret);
            if(!SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
            {
                Fir::logger->error(" 找不到对应server，id %d",SenceId);
            }
        }
    }

    return true;
}


bool TradeCmdHandle::ReqPurchase(SceneUser* User,const HelloKittyMsgData::ReqPurchase *message)
{
    if(ISSTATICNPC(message->charid()))
    {
        return User->m_trade.buyNpcStall(message->cellid());

    }
    if(User->charid == message->charid())
    {
        return false;
    }

    CMD::SCENE::t_UserPurchaseLockItem lockItem;
    lockItem.reqcharid = User->charid;
    lockItem.ackcharid = message->charid();
    lockItem.cellID = message->cellid();
    SceneUser* sceneUser = SceneUserManager::getMe().getUserByID(message->charid());
    if(sceneUser)
    {
        sceneUser->m_trade.purchaseItemLock(&lockItem);
    }
    else
    {
        SceneClientToOtherManager::getMe().SendMsgToOtherSceneCharID(message->charid(),&lockItem,sizeof(lockItem));
    }
    return true;
}

bool TradeCmdHandle::reqPurchaseItem(SceneUser* user,const HelloKittyMsgData::ReqPurchaseItem *message)
{
    if(!user || !message)
    {
        return false;
    }
    return user->purchaseItem(message);
}

bool TradeCmdHandle::reqAdvertiseTime(SceneUser* user,const HelloKittyMsgData::ReqAdvertiseTime *message)
{
    if(!user || !message)
    {
        return false;
    }
    return user->m_trade.ackAdvertiseTime();
}

bool TradeCmdHandle::reqPurchaseAdvertiseCD(SceneUser* user,const HelloKittyMsgData::ReqPurchaseAdvertiseCD *message)
{
    if(!user || !message)
    {
        return false;
    }
    return user->m_trade.parchaseCD();
}

bool TradeCmdHandle::reqContribute(SceneUser* user,const HelloKittyMsgData::ReqContribute *message)
{
    if(!user || !message)
    {
        return false;
    }
    user->updateContibute(message->charid());
    return true;
}

bool TradeCmdHandle::reqRewardActiveCode(SceneUser* user,const HelloKittyMsgData::ReqRewardActiveCode *message)
{
    if(!user || !message)
    {
        return false;
    }
    return user->rewardActiveCode(message->activecode());
}
