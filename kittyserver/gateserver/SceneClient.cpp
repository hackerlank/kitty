/**
 * \file
 * \version  $Id: SceneClient.cpp 65 2013-04-23 09:34:49Z  $
 * \author  ,@163.com
 * \date 2004年11月05日 13时58分55秒 CST
 * \brief 定义场景服务器连接客户端
 *
 */

#include <unistd.h>
#include <iostream>

#include "zTCPClient.h"
#include "SceneCommand.h"
#include "RecordCommand.h"
#include "SceneClient.h"
#include "GatewayServer.h"
#include "GateUserManager.h"
#include "Fir.h"
#include "Command.h"
#include "GatewayTask.h"
#include "RecordClient.h"
#include <time.h>
#include "GatewayTask.h"
#include "RecordClient.h"
#include "SceneClientManager.h"
#include "extractProtoMsg.h"
#include "auction.pb.h"
#include "chat.pb.h"
#include "TimeTick.h"
#include "RedisMgr.h"
#include "active.pb.h"

std::map<DWORD, QWORD> SceneClient::cmdCount;
std::map<DWORD, QWORD> SceneClient::forwardCount;
BYTE SceneClient::cmdCountSwitch = 0;
CmdAnalysis SceneClient::analysisScene(5, "SceneClient", 300);
CmdAnalysis SceneClient::analysisForward(6, "SceneForward", 300);


#if 0
SceneClientManager *SceneClientManager::instance = NULL;
#endif

/**
 * \brief 创建到场景服务器的连接
 *
 * \return 连接是否成功
 */
bool SceneClient::connectToSceneServer()
{
    if (!zTCPClientTask::connect())
    {
        Fir::logger->error("连接场景服务器失败");
        return false;
    }
    /*
       CMD::SUPER::t_restart_ServerEntry_NotifyOther notify; 
       notify.srcID = GatewayService::getMe().getServerID();
       notify.dstID = this->getServerID();

       std::string retNotify;
       encodeMessage(&notify,sizeof(notify),retNotify);
       GatewayService::getMe().sendCmdToSuperServer(retNotify.c_str(), retNotify.size());
       */
    using namespace CMD::SCENE;
    t_LoginScene tCmd;
    tCmd.wdServerID = GatewayService::getMe().getServerID();
    tCmd.wdServerType = GatewayService::getMe().getServerType();

    std::string ret;
    return encodeMessage(&tCmd,sizeof(tCmd),ret) && sendCmd(ret.c_str(),ret.size());
}

#if 0
/**
 * \brief 把一个场景服务器从连接添加到容器中删除
 *
 * \param client 场景服务器连接
 */
void SceneClientManager::remove(SceneClient *client)
{
    //Fir::logger->debug(__PRETTY_FUNCTION__);
    mlock.lock();
    std::vector<SceneClient *>::iterator iter = sceneClients.begin();
    for(;iter != sceneClients.end(); iter ++)
    {
        if(*iter == client)
        {
            sceneClients.erase(iter);
            mlock.unlock();
            return;
        }
    }
    mlock.unlock();
}
/**
 * \brief 把一个场景服务器的连接添加到容器中
 *
 * \param client 场景服务器连接
 */
void SceneClientManager::add(SceneClient *client)
{
    //Fir::logger->debug(__PRETTY_FUNCTION__);
    mlock.lock();
    sceneClients.push_back(client);
    mlock.unlock();
}

/**
 * \brief 通过服务器编号获取场景服务器连接
 *
 * \param serverid 服务器编号
 * \return 场景服务器连接
 */
SceneClient *SceneClientManager::getByServerID(WORD serverid)
{
    mlock.lock();
    for(int i=0,n=sceneClients.size();i<n;i++)
    {
        if(sceneClients[i]->getServerID()==serverid)
        {
            mlock.unlock();
            return sceneClients[i];
        }
    }
    mlock.unlock();
    return NULL;
}

/**
 * \brief 关闭管理器容器，释放资源
 *
 * 将关闭所有的与场景服务器的连接
 *
 */
void SceneClientManager::final()
{
    mlock.lock();
    while(!sceneClients.empty())
    {
        Fir::logger->debug(__PRETTY_FUNCTION__);
        std::vector<SceneClient *>::reference ref = sceneClients.back();
        sceneClients.pop_back();
        if (ref)
        {
            (ref)->final();
            (ref)->join();
            SAFE_DELETE(ref);
        }
    }
    mlock.unlock();
}
#endif 
int SceneClient::checkRebound()
{
    return 1;
}
void SceneClient::addToContainer()
{
    SceneClientManager::getMe().add(this);
}

void SceneClient::removeFromContainer()
{
    SceneClientManager::getMe().remove(this);
    GateUserManager::getMe().removeUserBySceneClient(this);
}
bool SceneClient::connect()
{
    return connectToSceneServer();
}

#if 0
/**
 * \brief 线程回调函数
 *
 */
void SceneClient::run()
{
    zTCPBufferClient::run();
    while(!GatewayService::getMe().isTerminate())
    {
        SceneClientManager::getMe().remove(this);
        GateUserManager::getMe().removeUserBySceneClient(this); 
        while(!connect())
        {
            Fir::logger->error("连接场景服务器失败");
            zThread::msleep(1000);
        }
        CMD::SUPER::t_restart_ServerEntry_NotifyOther notify;
        notify.srcID=GatewayService::getMe().getServerID();
        notify.dstID=this->getServerID();
        GatewayService::getMe().sendCmdToSuperServer(&notify, sizeof(notify));
        zThread::msleep(2000);
        connect();
        SceneClientManager::getMe().add(this);
        using namespace ;
        t_LoginScene tCmd;
        tCmd.wdServerID = GatewayService::getMe().getServerID();
        tCmd.wdServerType = GatewayService::getMe().getServerType();

        if(sendCmd(&tCmd, sizeof(tCmd)))
        {
            zTCPBufferClient::run();
        }
        // */
        zThread::msleep(1000);
    }

    //与场景服务器连接断开，关闭服务器
    GatewayService::getMe().Terminate();
}
#endif

bool SceneClient::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    Fir::logger->error("SceneClient::msgParseProto 消息没处理");
    //GateUserManager::getMe().sendCmd(data,nCmdLen);
    return true;
}

bool SceneClient::msgParseSceneFresh(const CMD::SCENE::t_Refresh_LoginScene *cmd,const DWORD nCmdLen)
{
    GateUser *pUser=(GateUser *)GateUserManager::getMe().getUserCharid(cmd->charid);

    if (!pUser) 
    {
        Fir::logger->trace("[登录],charid=%lu,0,0, 会话用户注册成功，网关未找到用户", cmd->charid);
        return true;
    }

    if (!pUser->isWaitPlayState())
    {
        Fir::logger->trace("[登录],charid=%lu,0,0, 会话用户注册成功，网关用户不在等待状态", cmd->charid);
        return true;
    }

    if (!pUser->gatewaytask)
    {
        Fir::logger->trace("[登录],charid=%lu,0,0, 会话用户注册成功，网关用户不在等待状态", cmd->charid);
        return true;
    }

    if (pUser->gatewaytask->isTerminateWait() || pUser->gatewaytask->isTerminate())
    {
        Fir::logger->trace("[登录],charid=%lu,0,0, 会话用户注册成功，网关用户不在等待状态", cmd->charid);
        return true;
    }

    pUser->playState(this, this->getServerID());
    Fir::logger->trace("[登录],charid=%lu,nickname=%s,0, 用户登录成功", pUser->charid, pUser->nickname);
    return true;
}

bool SceneClient::msgParseSceneCmd(const CMD::SCENE::SceneNull *sceneCmd,const DWORD nCmdLen)
{
    using namespace CMD::SCENE;

    switch(sceneCmd->para)
    {
        case PARA_SCENE_USER: 
            {
                t_User_FromScene* rev = (t_User_FromScene*)sceneCmd;
                if(rev->charid != 0)
                {
                    GateUser* u = GateUserManager::getMe().getUserCharid(rev->charid);
                    if (u && !u->isTerminateWait() && !u->isTerminate())
                    {
                        u->sendCmd(rev->data, rev->size);
                    }
                }
                else
                {
                    GateUserManager::getMe().sendCmd(rev->data,rev->size);

                }


                return true;
            }
            break;
        case PARA_REFRESH_LOGIN_SCENE:
            {
                return msgParseSceneFresh((t_Refresh_LoginScene*)sceneCmd,nCmdLen);
            }
        case PARA_START_OK_SCENE_GATE:
            {
                this->isStartOK = true;
                Fir::logger->debug("场景网关连接启动完成:%u",this->getServerID());
                return true;
            }
            break;
        case PARA_AUCTION_BID:
            {
                t_UserAuctionBid* rev = (t_UserAuctionBid*)sceneCmd;
                brocastAuction(rev->auctionID);
                return true;
            }
            break;
        case PARA_AUCTION_HISTORY:
            {
                t_UserAuctionHistory* rev = (t_UserAuctionHistory*)sceneCmd;
                brocastAuctionCenter(rev->data,rev->size);
                return true;
            }
            break;
        case PARA_SCENE_AUCTION_BRIEF:
            {
                t_UserAuctionBrief *rev = (t_UserAuctionBrief*)sceneCmd;
                //单播
                if(rev->charid)
                {
                    brocastAuctionBriefSingle(rev->charid);
                }
                //广播
                else
                {
                    brocastAuctionBrief();
                }
                return true;
            }
            break;
        case PARA_SCENE_AUCTION:
            {
                t_UserAuction* rev = (t_UserAuction*)sceneCmd;
                brocastAuction(rev->auctionID,rev->charid);
                return true;
            }
            break;
        case PARA_SCENE_AUCTION_END:
            {
                t_UserAuctionEnd* rev = (t_UserAuctionEnd*)sceneCmd;
                brocastAuctionRoomMsg(rev->auctionID,rev->data,rev->size);
                return true;
            }
            break;
        case PARA_SCENE_CHAT_BROAD:
            {
                t_ChatBroad *rev = (t_ChatBroad *)sceneCmd;
                brocastchat(*rev);
            }
            break;
        case PARA_BROADCAST:
            {
                t_UserBroadCast* rev = (t_UserBroadCast*)sceneCmd;
                GateUserManager::getMe().sendCmd(rev->data,rev->size);
                return true;
            }
            break;
            /*
               case PARA_ACTIVE_INFO:
               {
               t_UserActiveInfo* rev = (t_UserActiveInfo*)sceneCmd;
               broadcastActiveSingle(rev->charID);
               return true;
               }
               break;
               */
        case PARA_KICK_OFF:
            {
                t_UserKickOff *rev = (t_UserKickOff*)sceneCmd;
                GateUser* pUser =  GateUserManager::getMe().getUserCharid(rev->charID);
                if(pUser)
                {
                    pUser->Terminate();
                }
                return true;
            }
            break;
        default:
            break;

    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, sceneCmd->cmd, sceneCmd->para, nCmdLen);
    return true;
}


/**
 * \brief 解析来自场景服务器的指令
 *
 * \param ptNullCmd 待处理的指令
 * \param nCmdLen 待处理的指令长度
 * \return 解析是否成功
 */
bool SceneClient::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }

    using namespace CMD::SCENE;
    if (cmdCountSwitch) 
    {
        analysisScene.add(ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    }
    bool ret = false;

    switch(ptNullCmd->cmd)
    {
        case SCENECMD:
            {
                ret = msgParseSceneCmd((SceneNull*)ptNullCmd,nCmdLen);
            }
            break;
    }
    if(ret== false)
        Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return true;
}

bool SceneClient::brocastAuction(const DWORD auctionID,const QWORD charid)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(auctionID);
    if(!handle)
    {
        return false;
    }
    std::set<QWORD> memberSet;
    handle->getSet("auction",auctionID,"room",memberSet);
    if(memberSet.empty())
    {
        return false;
    }
    DWORD bidCnt = handle->getInt("auction",auctionID,"bidcnt");
    QWORD lastBider = handle->getInt("auction",auctionID,"lastbider");
    QWORD newBider = handle->getInt("auction",auctionID,"newbider");
    DWORD reward = handle->getInt("auction",auctionID,"reward");
    DWORD lastSec = handle->getInt("auction",auctionID,"lastsec");
    DWORD autoFlg = handle->getInt("auction",auctionID,"autoflg");
    DWORD bidsec = handle->getInt("auction",auctionID,"bidtime");
    HelloKittyMsgData::AckAuctionInfo ackAuctionInfo;
    ackAuctionInfo.set_auctionid(auctionID);
    ackAuctionInfo.set_bidcnt(bidCnt);
    ackAuctionInfo.set_lastbid("");
    ackAuctionInfo.set_newbid("");
    ackAuctionInfo.set_bidsec(bidsec);
    if(lastBider)
    {
        zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(lastBider);
        if(handleTemp)
        {
            ackAuctionInfo.set_lastbid(handleTemp->get("rolebaseinfo",lastBider,"nickname"));
        }
    }
    if(newBider)
    {
        zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(newBider);
        if(handleTemp)
        {
            ackAuctionInfo.set_newbid(handleTemp->get("rolebaseinfo",newBider,"nickname"));
        }
    }
    ackAuctionInfo.set_reward(reward);
    ackAuctionInfo.set_lastsec(lastSec);
    ackAuctionInfo.set_autobid(autoFlg ? true : false);
    if(charid)
    {   
        DWORD cnt = handle->getInt("auction",auctionID,charid);
        ackAuctionInfo.set_cnt(cnt);
        GateUser* user = GateUserManager::getMe().getUserCharid(charid);
        if(!user)
        {
            return false;
        }
        std::string ret;
        encodeMessage(&ackAuctionInfo,ret);
        return user->sendCmd(ret.c_str(),ret.size());

    }

    for(auto iter = memberSet.begin();iter != memberSet.end();++iter)
    {
        DWORD cnt = handle->getInt("auction",auctionID,*iter);
        ackAuctionInfo.set_cnt(cnt);
        GateUser* user = GateUserManager::getMe().getUserCharid(*iter);
        if(!user)
        {
            continue;
        }
        std::string ret;
        encodeMessage(&ackAuctionInfo,ret);
        user->sendCmd(ret.c_str(),ret.size());
    }
    return true;
}

bool SceneClient::brocastAuctionRoomMsg(const DWORD auctionID,const void *pstrCmd, const DWORD nCmdLen)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    std::set<QWORD> memberSet;
    handle->getSet("auction",auctionID,"room",memberSet);
    if(memberSet.empty())
    {
        return false;
    }
    for(auto iter = memberSet.begin();iter != memberSet.end();++iter)
    {
        GateUser* user = GateUserManager::getMe().getUserCharid(*iter);
        if(!user)
        {
            continue;
        }
        user->sendCmd(pstrCmd,nCmdLen);
    }
    return true;
}


bool SceneClient::brocastAuctionCenter(const void *pstrCmd, const DWORD nCmdLen)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    std::set<QWORD> memberSet;
    handle->getSet("auction",0,"center",memberSet);
    if(memberSet.empty())
    {
        return false;
    }
    for(auto iter = memberSet.begin();iter != memberSet.end();++iter)
    {
        GateUser* user = GateUserManager::getMe().getUserCharid(*iter);
        if(!user)
        {
            continue;
        }
        user->sendCmd(pstrCmd,nCmdLen);
    }
    return true;
}

bool SceneClient::brocastAuctionBriefSingle(const QWORD charID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    GateUser* user = GateUserManager::getMe().getUserCharid(charID);
    if(!user)
    {
        return false;
    }

    std::set<QWORD> centerMember;
    handle->getSet("auction",0,"center",centerMember);
    if(centerMember.find(charID) == centerMember.end())
    {
        return false;
    }
    std::set<QWORD> memberSet;
    handle->getSet("auction",0,"auction",memberSet);

    std::map<QWORD,std::set<QWORD>> auctionJoinMap;
    std::map<QWORD,std::set<QWORD>> auctionAutoBidMap;
    HelloKittyMsgData::AckAuctionBrief ackBrief;
    for(auto iter = memberSet.begin();iter != memberSet.end();++iter)
    {
        QWORD auctionID = *iter;
        zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(auctionID);
        if(!handleTemp)
        {
            continue;
        }
        DWORD state = handleTemp->getInt("auction",auctionID,"state"); 
        HelloKittyMsgData::AuctionBrief *temp = ackBrief.add_auctionbrief();
        if(!temp)
        {
            continue;
        }

        auto itJoin = auctionJoinMap.find(auctionID);
        if(itJoin == auctionJoinMap.end())
        {
            std::set<QWORD> joinSet;
            handleTemp->getSet("auction",auctionID,"room",joinSet);
            auctionJoinMap.insert(std::pair<QWORD,std::set<QWORD>>(auctionID,joinSet));
        }
        itJoin = auctionJoinMap.find(auctionID);
        const std::set<QWORD> &joinSet = itJoin->second;

        auto itAuto = auctionAutoBidMap.find(auctionID);
        if(itAuto == auctionAutoBidMap.end())
        {
            std::set<QWORD> autoBidSet;
            handleTemp->getSet("auction",auctionID,"autobidset",autoBidSet);
            auctionAutoBidMap.insert(std::pair<QWORD,std::set<QWORD>>(auctionID,autoBidSet));
        }
        itAuto = auctionAutoBidMap.find(auctionID);
        const std::set<QWORD> &autoBidSet = itAuto->second;

        DWORD lastSec = handleTemp->getInt("auction",auctionID,"lastsec"); 
        DWORD reward = handleTemp->getInt("auction",auctionID,"reward");
        QWORD lastBider = handleTemp->getInt("auction",auctionID,"lastbider");
        DWORD beginTime = handleTemp->getInt("auction",auctionID,"begintime");
        DWORD bidSec = handleTemp->getInt("auction",auctionID,"bidtime");
        temp->set_lastsec(lastSec);
        temp->set_auctionid(auctionID);
        temp->set_reward(reward);
        temp->set_bidsec(bidSec);
        temp->set_lastbid("");
        if(lastBider)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(lastBider);
            if(handleTemp)
            {
                temp->set_lastbid(handleTemp->get("rolebaseinfo",lastBider,"nickname"));
            }
        }
        bool joinFlg = joinSet.find(charID) != joinSet.end() ? true : false;
        bool autoFlg = autoBidSet.find(charID) != autoBidSet.end() ? true : false;
        bool ownerFlg = lastBider == charID ? true : false;
        temp->set_autobid(autoFlg);
        temp->set_owner(ownerFlg);
        temp->set_join(joinFlg);
        temp->set_state(state);
        temp->set_begintime(beginTime);
    }
    std::string ret;
    encodeMessage(&ackBrief,ret);
    user->sendCmd(ret.c_str(),ret.size());
    return true;
}

bool SceneClient::brocastAuctionBrief()
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    std::set<QWORD> centerMember;
    handle->getSet("auction",0,"center",centerMember);
    if(centerMember.empty())
    {
        return false;
    }
    std::map<QWORD,std::set<QWORD>> auctionJoinMap;
    std::map<QWORD,std::set<QWORD>> auctionAutoBidMap;
    //这期的所有拍卖
    std::set<QWORD> memberSet;
    handle->getSet("auction",0,"auction",memberSet);
    for(auto it = centerMember.begin();it != centerMember.end();++it)
    {
        QWORD charID = *it;
        GateUser* user = GateUserManager::getMe().getUserCharid(charID);
        if(!user)
        {
            continue;
        }
        HelloKittyMsgData::AckAuctionBrief ackBrief;
        for(auto iter = memberSet.begin();iter != memberSet.end();++iter)
        {
            QWORD auctionID = *iter;
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(auctionID);
            if(!handleTemp)
            {
                continue;
            }
            DWORD state = handleTemp->getInt("auction",auctionID,"state"); 
            HelloKittyMsgData::AuctionBrief *temp = ackBrief.add_auctionbrief();
            if(!temp)
            {
                continue;
            }

            auto itJoin = auctionJoinMap.find(auctionID);
            if(itJoin == auctionJoinMap.end())
            {
                std::set<QWORD> joinSet;
                handleTemp->getSet("auction",auctionID,"room",joinSet);
                auctionJoinMap.insert(std::pair<QWORD,std::set<QWORD>>(auctionID,joinSet));
            }
            itJoin = auctionJoinMap.find(auctionID);
            const std::set<QWORD> &joinSet = itJoin->second;

            auto itAuto = auctionAutoBidMap.find(auctionID);
            if(itAuto == auctionAutoBidMap.end())
            {
                std::set<QWORD> autoBidSet;
                handleTemp->getSet("auction",auctionID,"autobidset",autoBidSet);
                auctionAutoBidMap.insert(std::pair<QWORD,std::set<QWORD>>(auctionID,autoBidSet));
            }
            itAuto = auctionAutoBidMap.find(auctionID);
            const std::set<QWORD> &autoBidSet = itAuto->second;

            DWORD lastSec = handleTemp->getInt("auction",auctionID,"lastsec"); 
            DWORD reward = handleTemp->getInt("auction",auctionID,"reward");
            QWORD lastBider = handleTemp->getInt("auction",auctionID,"lastbider");
            DWORD beginTime = handleTemp->getInt("auction",auctionID,"begintime");
            DWORD bidSec = handleTemp->getInt("auction",auctionID,"bidtime");
            temp->set_auctionid(auctionID);
            temp->set_reward(reward);
            temp->set_lastsec(lastSec);
            temp->set_state(state);
            temp->set_begintime(beginTime);
            temp->set_bidsec(bidSec);
            temp->set_lastbid("");
            if(lastBider)
            {
                zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(lastBider);
                if(handleTemp)
                {
                    temp->set_lastbid(handleTemp->get("rolebaseinfo",lastBider,"nickname"));
                }
            }
            bool joinFlg = joinSet.find(charID) != joinSet.end() ? true : false;
            bool autoFlg = autoBidSet.find(charID) != autoBidSet.end() ? true : false;
            bool ownerFlg = lastBider == charID ? true : false;
            temp->set_autobid(autoFlg);
            temp->set_owner(ownerFlg);
            temp->set_join(joinFlg);
        }
        std::string ret;
        encodeMessage(&ackBrief,ret);
        user->sendCmd(ret.c_str(),ret.size());
    }
    return true;
}
BYTE fillchat(HelloKittyMsgData::playerShowbase& base ,QWORD playerId)
{
    CharBase charbase;
    if(!RedisMgr::getMe().get_charbase(playerId,charbase))
    {
        return 0;

    }
    HelloKittyMsgData::Serialize binary;
    if(!RedisMgr::getMe().get_binary(playerId, binary))
    {
        return 0;
    }
    base.set_playerid(playerId);
    base.set_playername(charbase.nickname);
    base.set_playerlevel(charbase.level);
    HelloKittyMsgData::SexType sextype = charbase.sex > 0 ? HelloKittyMsgData::Female : HelloKittyMsgData::Male;
    base.set_playersex(sextype);
    base.set_area(charbase.areaType);
    *(base.mutable_head()) = binary.charbin().head();
    return charbase.lang;


}

void SceneClient::brocastchat(const CMD::SCENE::t_ChatBroad &cmd)
{
    HelloKittyMsgData::ChatChannel channel = static_cast<HelloKittyMsgData::ChatChannel>(cmd.channel);
    switch(channel)
    {
        case  HelloKittyMsgData::ChatChannel_Family:
            {
                HelloKittyMsgData::AckNoticeChat OtherAck;
                HelloKittyMsgData::chatinfo* pbuf = OtherAck.mutable_chattxt();
                if(pbuf)
                {
                    pbuf->ParseFromArray(cmd.data,cmd.size);
                }
                OtherAck.set_channel(channel);
                OtherAck.set_senddate(GatewayTimeTick::currentTime.sec());
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd.sendid);
                if(!handle)
                    break;
                HelloKittyMsgData::playerShowbase* player = OtherAck.mutable_sendplayer();
                if(!player)
                {
                    break;
                }
                fillchat(*player,cmd.sendid);
                QWORD familyID = (QWORD)handle->getInt("family",cmd.sendid,"belong");
                if(familyID == 0)
                    break;
                zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(familyID);
                if(!redishandle)
                    break;
                std::set<QWORD> setMember;
                if(!redishandle->getSet("family", familyID, "include", setMember))
                    break;
                for(auto iter = setMember.begin();iter != setMember.end();iter++)
                {
                    GateUser* user = GateUserManager::getMe().getUserCharid(*iter);
                    if(!user)
                        continue;
                    std::string ret; 
                    encodeMessage(&OtherAck,ret); 
                    user->sendCmd(ret.c_str(),ret.size()); 

                }

            }
            break;
        case HelloKittyMsgData::ChatChannel_World:
            {
                HelloKittyMsgData::AckNoticeChat OtherAck;
                HelloKittyMsgData::chatinfo* pbuf = OtherAck.mutable_chattxt();
                if(pbuf)
                {
                    pbuf->ParseFromArray(cmd.data,cmd.size);
                }
                OtherAck.set_channel(channel);
                OtherAck.set_senddate(GatewayTimeTick::currentTime.sec());
                HelloKittyMsgData::playerShowbase* player = OtherAck.mutable_sendplayer(); 
                if(!player)
                    break;
                BYTE lang = fillchat(*player,cmd.sendid);
                std::string ret; 
                encodeMessage(&OtherAck,ret); 
                GateUserManager::getMe().sendChatToworld(ret.c_str(),ret.size(),lang); 
            }
            break;
        case HelloKittyMsgData::ChatChannel_City:
            {
                HelloKittyMsgData::AckNoticeChat OtherAck;
                HelloKittyMsgData::chatinfo* pbuf = OtherAck.mutable_chattxt();
                if(pbuf)
                {
                    pbuf->ParseFromArray(cmd.data,cmd.size);
                }
                OtherAck.set_channel(channel);
                OtherAck.set_senddate(GatewayTimeTick::currentTime.sec());
                HelloKittyMsgData::playerShowbase* player = OtherAck.mutable_sendplayer(); 
                if(!player)
                    break;
                fillchat(*player,cmd.sendid);
                std::string ret; 
                encodeMessage(&OtherAck,ret); 
                GateUserManager::getMe().sendChatToCity(player->area(),ret.c_str(),ret.size()); 
            }
            break;

        default:
            break;
    }
}
/*
   bool SceneClient::broadcastActiveSingle(const QWORD charID)
   {
   GateUser *user = GateUserManager::getMe().getUserCharid(charID);
   if(!user)
   {
   return false;
   }
   zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle();
   if(!handleTemp)
   {
   return false;
   }
   HelloKittyMsgData::AckActiveInfo ack;
   std::set<QWORD> activeIDset;
   handleTemp->getSet("active",0,"active",activeIDset);
   for(auto iter = activeIDset.begin();iter != activeIDset.end();++iter)
   {
   QWORD activeID = *iter;
   zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(activeID % MAX_MEM_DB+1);
   if(!handleTemp)
   {
   continue;
   }
   HelloKittyMsgData::ActiveInfo *temp = ack.add_activeinfo();
   if(!temp)
   {
   continue;
   }
   DWORD state = handleTemp->getInt("active",activeID,"state");
#if 0
DWORD beginTime = handleTemp->getInt("active",activeID,"begintime");
DWORD endTime = handleTemp->getInt("active",activeID,"endtime");
#endif
temp->set_activeid(activeID);
temp->set_status(HelloKittyMsgData::ActiveStatus(state));
#if 0
temp->set_begintime(beginTime);
temp->set_endtime(endTime);
#endif
}

std::string ret;
encodeMessage(&ack,ret);
user->sendCmd(ret.c_str(),ret.size());
return true;
}
*/
