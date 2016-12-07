/**
 * \file
 * \version  $Id: SceneClientToOtherManager.cpp 42 2013-04-10 07:33:59Z  $
 * \author  王海军, wanghaijun@ztgame.com 
 * \date 2006年01月04日 16时56分05秒 CST
 * \brief 场景到场景数据缓冲发送
 *
 * 
 */

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "SceneClientToOther.h"
#include "SceneToOtherManager.h"
#include "zXMLParser.h"
#include "SceneServer.h"
#include "SceneTaskManager.h"
#include "SceneUserManager.h"
#include "SceneMail.h"
#include "chat.pb.h" 
#include "TimeTick.h"
/**
 ** \brief 类的唯一实例指针
 **/
SceneClientToOtherManager *SceneClientToOtherManager::instance = NULL;

/**
 ** \brief 构造函数
 **/
SceneClientToOtherManager::SceneClientToOtherManager()
{
    SceneClientToOtherPool = NULL;
}

/**
 ** \brief 析构函数
 **/
SceneClientToOtherManager::~SceneClientToOtherManager()
{
    SAFE_DELETE(SceneClientToOtherPool);
}

void SceneClientToOtherManager::final()
{
    allClients.clear();
    SAFE_DELETE(SceneClientToOtherPool);

}

/**
 ** \brief 初始化管理器
 ** \return 初始化是否成功
 **/
bool SceneClientToOtherManager::init()
{
    const CMD::SUPER::ServerEntry *serverEntry = SceneService::getMe().getServerEntryByType(SCENESSERVER);
    SceneClientToOtherPool = new zTCPClientTaskPool(10,8000);
    if (NULL == SceneClientToOtherPool
            || !SceneClientToOtherPool->init())
        return false;

    while(serverEntry)
    {
        SceneClientToOther *pclient = new SceneClientToOther("Scene服务器", serverEntry);
        if (NULL == pclient)
        {
            Fir::logger->error("没有足够内存，不能建立Scene服务器客户端实例");
            return false;
        }
        if(SceneClientToOtherPool->put(pclient))
        {
            if(!pclient->connect())
            {
                Fir::logger->error("can not connect");
                return false;
            }
            SceneClientToOtherPool->addCheckwait(pclient);

        }
        serverEntry = SceneService::getMe().getNextServerEntryByType(SCENESSERVER, &serverEntry);
    }
    return true;
}

/**
 ** \brief 周期间隔进行连接的断线重连工作
 ** \param ct 当前时间
 **/
void SceneClientToOtherManager::timeAction(const zTime &ct)
{
    if (actionTimer.elapse(ct) > 4)
    {
        if (SceneClientToOtherPool)
            SceneClientToOtherPool->timeAction(ct);
        actionTimer = ct;
    }
}

/**
 ** \brief 向容器中添加已经成功的连接
 ** \param SceneClientToOther 待添加的连接
 **/
void SceneClientToOtherManager::add(SceneClientToOther *SceneClientToOther)
{
    if (SceneClientToOther)
    {
        zRWLock_scope_wrlock scope_wrlock(rwlock);
        allClients.insert(value_type(SceneClientToOther->getServerID(), SceneClientToOther));
    }
}

/**
 ** \brief 从容器中移除断开的连接
 ** \param SceneClientToOther 待移除的连接
 **/
void SceneClientToOtherManager::remove(SceneClientToOther *SceneClientToOther)
{
    if (SceneClientToOther)
    {
        zRWLock_scope_wrlock scope_wrlock(rwlock);
        iter it = allClients.find(SceneClientToOther->getServerID());
        if (it != allClients.end())
        {
            allClients.erase(it);
        }
    }
}

void SceneClientToOtherManager::removebyId(const DWORD tempid)
{
    zRWLock_scope_wrlock scope_wrlock(rwlock);
    iter it = allClients.find(tempid);
    if (it != allClients.end())
    {
        SAFE_DELETE(it->second);
        allClients.erase(it);
    }


}

/**
 ** \brief 向成功的所有连接广播指令
 ** \param pstrCmd 待广播的指令
 ** \param nCmdLen 待广播指令的长度
 **/
bool SceneClientToOtherManager::broadcastOne(const void *pstrCmd, int nCmdLen)
{
    return false;
}

bool SceneClientToOtherManager::sendTo(const DWORD tempid, const void *pstrCmd, int nCmdLen)
{
    zRWLock_scope_rdlock scope_rdlock(rwlock);
    iter it = allClients.find(tempid);
    if (it == allClients.end())
        return false;
    else
        return it->second->sendCmd(pstrCmd, nCmdLen);
}

/**
 ** \brief  重新连接场景服务器
 ** \return 是否初始化成功
 **/
bool SceneClientToOtherManager::reConnectScene(const CMD::SUPER::ServerEntry *serverEntry)
{
    if (NULL == SceneClientToOtherPool)
    {
        return false;
    }

    if (serverEntry)
    {
        SceneClientToOther *pclient = FIR_NEW SceneClientToOther("Scene服务器", serverEntry);
        if (NULL == pclient)
        {
            Fir::logger->error("没有足够内存，不能建立Scene服务器客户端实例");
            return false;
        }
        if(SceneClientToOtherPool->put(pclient))
        {
            if(!pclient->connect())
            {
                Fir::logger->error("can not connect");
                return false;
            }
            SceneClientToOtherPool->addCheckwait(pclient);

        }
#ifdef _PQQ_DEBUG
        Fir::logger->debug("[重连场景] 场景重新连接场景 ip=%s,name=%s,port=%d"
                ,serverEntry->pstrIP,serverEntry->pstrName,serverEntry->wdPort);
#endif
    }
    return true;
}

/**
 * \brief 设置到场景的某一连接是否需要重连
 * \param ip 要连接的ip
 * \param port 端口
 * \param reconn 是否重连 true重连，false 不再重连，将会删掉
 */
void SceneClientToOtherManager::setTaskReconnect(const std::string& ip, unsigned short port, bool reconn)
{
    if (SceneClientToOtherPool)
    {
        SceneClientToOtherPool->setTaskReconnect(ip, port, reconn);
    }
}

SceneClientToOther* SceneClientToOtherManager::getSceneByID(DWORD id)
{
    SceneClientToOther_map::iterator iter = allClients.find(id);
    if (iter!=allClients.end())
        return (SceneClientToOther*)iter->second;

    return NULL;

}

bool SceneClientToOtherManager::isAllStartOK()
{
    zRWLock_scope_rdlock scope_rdlock(rwlock);
    if (allClients.empty())
        return false;
    for (auto it = allClients.begin(); it != allClients.end(); ++it)
    {
        if (it->second && !it->second->getStartOK())
            return false;
    }
    return true;
}


bool SceneClientToOtherManager::SendMsgToOtherScene(const DWORD ServerId,const void *pstrCmd, int nCmdLen)
{
    SceneClientToOther* pOther = getSceneByID(ServerId);
    if(pOther)
    {
        return pOther->sendCmd(pstrCmd,nCmdLen);
    }
    SceneTask * pTask = SceneTaskManager::getMe().getTaskByID(ServerId);
    if(pTask)
    {
        return pTask->sendCmd(pstrCmd,nCmdLen);
    }
    return false;

}

bool SceneClientToOtherManager::SendMsgToOtherSceneCharID(const QWORD charid,const _null_cmd_ *message,const DWORD len)
{
    std::string ret;
    encodeMessage(message,len,ret);
    return SceneClientToOtherManager::getMe().SendMsgToOtherSceneCharID(charid,ret.c_str(),ret.size());
}

bool SceneClientToOtherManager::SendMsgToOtherSceneCharID(const QWORD charid,const google::protobuf::Message *message)
{
    std::string ret;
    encodeMessage(message,ret);
    return SceneClientToOtherManager::getMe().SendMsgToOtherSceneCharID(charid,ret.c_str(),ret.size());
}

bool SceneClientToOtherManager::SendMsgToOtherSceneCharID(const QWORD charid,const void *data,const DWORD len)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid);
    if(!handle)
    {
        Fir::logger->error("%s(%lu)", __PRETTY_FUNCTION__,charid);
        return false;
    }
    SceneUser* sceneUser = SceneUserManager::getMe().getUserByID(charid); 
    if(sceneUser)
    {
        return false;
    }
    DWORD senceId = handle->getInt("playerscene",charid,"sceneid");
    if(!senceId)
    {
        Fir::logger->error("%s(%lu,%u)", __PRETTY_FUNCTION__,charid,senceId);
        return false;
    }
    return SceneClientToOtherManager::getMe().SendMsgToOtherScene(senceId,data,len);
}

bool SceneClientToOtherManager::msgParseOtherSceneCmd(const DWORD ServerId,const CMD::SCENE::SceneNull *sceneCmd,const DWORD nCmdLen)
{
    using namespace CMD::SCENE;

    switch(sceneCmd->para)
    {

        case PARA_PLAYERAWARD:
            {
                const CMD::SCENE::t_awardtoPlayer_scenescene *Cmd = (CMD::SCENE::t_awardtoPlayer_scenescene *)sceneCmd;
                HelloKittyMsgData::vecAward rvecAward;
                rvecAward.ParseFromArray(Cmd->data,Cmd->size);
                SceneUser* user = SceneUserManager::getMe().getUserByID(Cmd->charid);
                if(user)
                {
                    user->DoBuildAward(rvecAward, Cmd->eventid,Cmd->charowner,Cmd->bnotice);
                }
                else
                {
                    //SendMail
                    SceneMailManager::getMe().sendSysMailToPlayerForEvent(Cmd->charid,Cmd->eventid,Cmd->charowner,rvecAward);
                }

                return true;
            }
            break;
        case PARA_SETFANS:
            {
                const CMD::SCENE::t_setFans_scenescene *Cmd = (CMD::SCENE::t_setFans_scenescene *)sceneCmd;

                SceneUser* user = SceneUserManager::getMe().getUserByID(Cmd->charid);
                if(user)
                {
                    if(Cmd->type == 0)
                    {
                        user->getFriendManager().AddFans(Cmd->fansid);
                    }
                    else
                    {
                        user->getFriendManager().DelFans(Cmd->fansid);

                    }
                }
                return true;
            }
        case PARA_EMAIL_SEND:
            {
                const CMD::SCENE::t_EmailSendPlayer_scenescene *cmd = (CMD::SCENE::t_EmailSendPlayer_scenescene*)sceneCmd;
                HelloKittyMsgData::EmailInfo emailInfo;
                emailInfo.ParseFromArray(cmd->data,cmd->size);
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charid);
                if(user)
                {
                    user->m_emailManager.acceptEmail(emailInfo);
                    return true;
                }
                Fir::logger->debug("[发送邮件],找不到角色(%lu)",cmd->charid);
                return false;
            }
            break;
        case PARA_SetVisit:
            {
                const CMD::SCENE::t_SetVisit_scenescene *cmd = (CMD::SCENE::t_SetVisit_scenescene *)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->ownerid);

                if(cmd->chargateid == 0)//退出
                {
                    if(user)
                    {
                        user->DelVisit(cmd->charid);
                    }

                }
                else //访问
                {
                    if(!user)
                        user = SceneUserManager::getMe().CreateTempUser(cmd->ownerid);
                    if(user)
                    {
                        user->AddVisit(cmd->charid,cmd->chargateid);
                    }
                    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->ownerid);
                    if(handle)
                        handle->delLock("playerlock",cmd->ownerid,"newplayer");   
                }

                return true;

            }
            break;
        case PARA_DoBuliding:
            {
                const CMD::SCENE::t_DoBulid_scenescene *cmd = (CMD::SCENE::t_DoBulid_scenescene *)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->ownerid);
                if(!user)
                {
                    Fir::logger->debug("[设置访问者],找不到角色(%lu)",cmd->ownerid); 
                    return false;
                }
                HelloKittyMsgData::Builditype rBuild;
                rBuild.set_isicon(cmd->isIcon);
                rBuild.set_buildid(cmd->buildid);
                user->opBuild(cmd->charid,rBuild);
                return true;
            }
            break;
        case PARA_REQSALL:
            {
                const CMD::SCENE::t_UserReqSall *cmd = (CMD::SCENE::t_UserReqSall*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->ackcharid);
                if(!user)
                {
                    return false;
                }
                HelloKittyMsgData::AckPbSaleCeilFlush ackFlush;
                ackFlush.set_charid(cmd->ackcharid);
                user->m_trade.flushSaleBooth(ackFlush);
                std::string ret;
                encodeMessage(&ackFlush,ret);
                SceneTaskManager::getMe().broadcastUserCmdToGateway(cmd->reqcharid,ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_PURCHASE_LOCK_ITEM:
            {
                const CMD::SCENE::t_UserPurchaseLockItem *cmd = (CMD::SCENE::t_UserPurchaseLockItem*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->ackcharid);
                if(!user)
                {
                    return false;
                }
                user->m_trade.purchaseItemLock(cmd);
                return true;
            }
            break;
        case PARA_PURCHASE_PRICE:
            {
                const CMD::SCENE::t_UserPurchasePrice *cmd = (CMD::SCENE::t_UserPurchasePrice*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->reqcharid);
                if(!user)
                {
                    return false;
                }
                return user->deductPrice(cmd);
            }
            break;
        case PARA_PURCHASE_UNLOCK_ITEM:
            {
                const CMD::SCENE::t_UserPurchaseUnlockeItem *cmd = (CMD::SCENE::t_UserPurchaseUnlockeItem*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->ackcharid);
                if(!user)
                {
                    return false;
                }
                user->m_trade.purchaseUnLockItem(cmd);
                return true;
            }
            break;
        case PARA_PURCHASE_SHIFT_ITEM:
            {
                const CMD::SCENE::t_UserPurchaseShiftItem *cmd = (CMD::SCENE::t_UserPurchaseShiftItem*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->reqcharid);
                if(!user)
                {
                    return false;
                }
                return user->purchaseAddItem(cmd);
            }
            break;
        case PARA_SCENE_CHAT_MAP:
            {
                CMD::SCENE::t_ChatMap *cmd  = (CMD::SCENE::t_ChatMap*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->ownerid);
                if(!user)
                {
                    return false;
                }

                HelloKittyMsgData::AckNoticeChat OtherAck;
                HelloKittyMsgData::chatinfo* pbuf = OtherAck.mutable_chattxt();
                if(pbuf)
                {
                    pbuf->ParseFromArray(cmd->data,cmd->size);
                }
                OtherAck.set_channel(HelloKittyMsgData::ChatChannel_Map);
                OtherAck.set_senddate(SceneTimeTick::currentTime.sec());
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->sendid);
                if(!handle)
                    break;
                CharBase charbase;
                if (handle->getBin("charbase",cmd->sendid,"charbase",(char*)&charbase) == 0)
                {
                    break;
                }

                HelloKittyMsgData::playerShowbase* player = OtherAck.mutable_sendplayer();
                if(!player)
                    return false;
                SceneUser::getplayershowbase(cmd->sendid,*player);
                std::string ret;
                encodeMessage(&OtherAck,ret);
                user->broadcastMsgInMap(ret.c_str(),ret.size(),true);
                return true;

            }
            break;
        case PARA_GIFT_SEND:
            {
                const CMD::SCENE::t_GiftSendPlayer_scenescene *cmd = (CMD::SCENE::t_GiftSendPlayer_scenescene*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->accepter);
                if(!user)
                {
                    Fir::logger->debug("[赠送礼品],找不到角色(%lu)",cmd->sender);
                    return false;
                }
                if(!cmd->type)
                {
                    HelloKittyMsgData::GiftContain giftInfo;
                    giftInfo.ParseFromArray(cmd->data,cmd->size);
                    user->m_giftPackage.acceptGift(cmd->sender,cmd->time,giftInfo);
                    return true;
                }
                else
                {
                    HelloKittyMsgData::ReqSendVirtualGift sendVirugalGift;
                    sendVirugalGift.ParseFromArray(cmd->data,cmd->size);
                    return user->acceptVirtualGift(cmd->sender,&sendVirugalGift);
                }
                return false;
            }
            break;
        case PARA_SCENE_LEAVE_MESSAGE:
            {
                CMD::SCENE::t_leaveMessage *cmd  = (CMD::SCENE::t_leaveMessage*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->ownerid);
                if(!user)
                {
                    return false;
                }
                user->m_leavemessage.leaveMessage(cmd->sendid,cmd->chattxt);
                return true;

            }
            break;
        case PARA_SCENE_GETLIST_MESSAGE:
            {
                CMD::SCENE::t_GetMessageList *cmd  = (CMD::SCENE::t_GetMessageList*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->ownerid);
                if(!user)
                {
                    return false;
                }
                return true;
            }
            break;
        case PARA_VIST_ROOM:
            {
                CMD::SCENE::t_UserVistRoom *cmd = (CMD::SCENE::t_UserVistRoom*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                if(user)
                {
                    return cmd->enter ? user->ackRoom(cmd->vistor) : user->outRoom(cmd->vistor);
                }
                return false;
            }
            break;
        case PARA_FORBID:
            {
                CMD::SCENE::t_UserForBid *cmd = (CMD::SCENE::t_UserForBid*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                if(user)
                {
                    return cmd->opForBid ? user->forBid(cmd->endTime,cmd->reason) : user->forBidSys(cmd->endTime,cmd->reason);
                }
                return false;
            }
            break;
        case PARA_PLAYER_INFO:
            {
                return false;
            }
            break;
        case PARA_BID_RERAED:
            {
                CMD::SCENE::t_UserBidReward *cmd = (CMD::SCENE::t_UserBidReward*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                if(user)
                {
                    HelloKittyMsgData::GiftCashInfo giftInfo;
                    giftInfo.ParseFromArray(cmd->data,cmd->size);
                    user->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Auction_Success_Number,1);
                    return user->m_giftPackage.addGiftCash(giftInfo);
                }
                return false;
            }
            break;
        case PARA_TRAIN_LOAD:
            {
                CMD::SCENE::t_TrainLoad *cmd =(CMD::SCENE::t_TrainLoad*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                if(user)
                {
                    user->m_managertrain.loadothertrain(cmd->friendID,cmd->trainID);
                }
                return true;

            }
            break;
        case PARA_addOrConsumeItem_Scence:
            {

                CMD::SCENE::t_addOrConsumeItem *cmd =(CMD::SCENE::t_addOrConsumeItem*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                if(user)
                {
                    user->m_store_house.addOrConsumeItem(cmd->ItemID,cmd->ItemNum,cmd->remark,cmd->bIsAdd,cmd->bjudgeFull);

                }
                return true;

            }
            break;
        case PARA_UnityBuild_AddTimes:
            {
                const CMD::SCENE::t_UnityBuild_AddTimes *cmd = (CMD::SCENE::t_UnityBuild_AddTimes *)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);

                if(!user)
                    user = SceneUserManager::getMe().CreateTempUser(cmd->charID);
                if(user)
                {
                    user->m_unitybuild.addUnitBuildTimes(cmd->colID);
                }
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->charID);
                if(handle)
                    handle->delLock("playerlock",cmd->charID,"newplayer");   
                return true;
            }
            break;
        case PARA_UnityBuild_Push:
            {
                const CMD::SCENE::t_UnityBuild_Push *cmd = (CMD::SCENE::t_UnityBuild_Push *)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);

                if(!user)
                    user = SceneUserManager::getMe().CreateTempUser(cmd->charID);
                if(user)
                {
                    user->m_buildManager.pushUinityBuild(cmd->onlyID,cmd->buildlevel,cmd->friendID,cmd->buildID);
                }
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->charID);
                if(handle)
                    handle->delLock("playerlock",cmd->charID,"newplayer");   
                return true;
            }
            break;
        case PARA_UnityBuild_Syn:
            {
                const CMD::SCENE::t_UnityBuild_Syn *cmd = (CMD::SCENE::t_UnityBuild_Syn *)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);

                if(!user)
                    user = SceneUserManager::getMe().CreateTempUser(cmd->charID);
                if(user)
                {
                    user->m_buildManager.setUinityBuildLevel(cmd->onlyID,cmd->buildlevel);
                }
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->charID);
                if(handle)
                    handle->delLock("playerlock",cmd->charID,"newplayer");   
                return true;
            }
            break;
        case PARA_UnityBuild_NoticeUpdate:
            {
                const CMD::SCENE::t_UnityBuild_NoticeUpdate *cmd = (const CMD::SCENE::t_UnityBuild_NoticeUpdate *)sceneCmd;
                 SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                 if(user)
                     user->m_unitybuild.updateCliColInfoByColId(cmd->colID);
            }
            break;
        case PARA_STAR_GAME:
            {
                const CMD::SCENE::t_Star_Game *cmd = (const CMD::SCENE::t_Star_Game*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                if(user)
                {
                    HelloKittyMsgData::AckStartGame ackMsg;
                    ackMsg.ParseFromArray(cmd->data,cmd->size);
                    return user->ackStartGame(&ackMsg);
                }
            }
            break;
        case PARA_CLEAR_RANK_DATA:
            {
                const CMD::SCENE::t_ClearRankData *cmd = (const CMD::SCENE::t_ClearRankData*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                if(user)
                {
                    return cmd->type ? user->clearMonthData() : user->clearWeekData();
                }
            }
            break;
        case PARA_LIKE_OP:
            {
                const CMD::SCENE::t_LikeOp *cmd = (const CMD::SCENE::t_LikeOp*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                if(user)
                {
                    user->opLike(cmd->oper,cmd->addOp);
                }
                return true;
            }
            break;
        case PARA_SEE_OTHER_PERSON:
            {
                const CMD::SCENE::t_SeeOtherPerson *cmd = (const CMD::SCENE::t_SeeOtherPerson*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                if(user)
                {
                    return cmd->onlyPerson ? user->ackPersonalInfo(cmd->oper) : user->ackRoomAndPerson(cmd->oper);
                }
            }
            break;
        case PARA_VIEW_WECHAT:
            {
                const CMD::SCENE::t_ViewWechat *cmd = (const CMD::SCENE::t_ViewWechat*)sceneCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->charID);
                if(user)
                {
                    user->addBuyWechat(cmd->viewer);
                    user->ackRoomAndPerson(cmd->viewer);
                    HelloKittyMsgData::AckViewWechat ackMsg;
                    std::string msg;
                    encodeMessage(&ackMsg,msg);
                    SceneTaskManager::getMe().broadcastUserCmdToGateway(cmd->viewer,msg.c_str(),msg.size());
                    return true;
                }
            }
            break;
        default:
            {
                break;
            }
    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, sceneCmd->cmd, sceneCmd->para, nCmdLen);
    return false;


}

void SceneClientToOtherManager::execEvery()
{
    zRWLock_scope_wrlock scope_wrlock(rwlock);
    for (auto it = allClients.begin(); it != allClients.end(); ++it)
    {
        if(it->second)
            it->second->doCmd();
    }


}
