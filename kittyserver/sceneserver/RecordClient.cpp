/**
 * \file
 * \version  $Id: RecordClient.cpp 37 2013-04-08 01:52:56Z  $
 * \author  ,
 * \date 2013年04月05日 13时58分55秒 CST
 * \brief 定义档案服务器连接客户端
 *
 * 负责与档案服务器交互，存取档案
 * 
 */

#include <unistd.h>
#include <iostream>

#include "zTCPClient.h"
#include "RecordCommand.h"
#include "RecordClient.h"
#include "SceneServer.h"
#include "Fir.h"
#include "SceneTask.h"
#include "extractProtoMsg.h"
#include "SceneUserManager.h"
#include "trade.pb.h"
#include "FamilyCmdDispatcher.h"
#include "TimeTick.h"
#include "SceneToOtherManager.h"

//RecordClient *recordClient = NULL;
ManagerRecordClient MgrrecordClient;

/**
 * \brief 解压角色需要保存的数据
 *
 * \param pUser 角色指针
 * \param data 输入：压缩的数据 / 输出：解压后的数据 
 * \param dataSize 输入 数据大小
 * \return 解压后的数据大小
 */
//bool uncompressSaveData(SceneUser *pUser , const unsigned char *data ,const DWORD dataSize, unsigned char * petData); 

/**
 * \brief 创建到档案服务器的连接
 *
 * \return 连接是否成功
 */
bool RecordClient::connectToRecordServer()
{
    if (!zTCPClientTask::connect())
    {
        Fir::logger->error("连接档案服务器失败");
        return false;
    }
    using namespace CMD::RECORD;
    t_LoginRecord tCmd;
    tCmd.wdServerID = SceneService::getMe().getServerID();
    tCmd.wdServerType = SceneService::getMe().getServerType();

    std::string ret;
    return encodeMessage(&tCmd,sizeof(tCmd),ret) && sendCmd(ret.c_str(),ret.size());
}
bool RecordClient::connect()
{
    return connectToRecordServer();
}

void RecordClient::addToContainer()
{
    MgrrecordClient.add(this);
}

void RecordClient::removeFromContainer()
{
    MgrrecordClient.remove(this);
}
bool RecordClient::msgParse(const BYTE *message, const DWORD nCmdLen)
{
    return msgPush(message,nCmdLen);
}
bool RecordClient::cmdMsgParse(const BYTE *message, const DWORD cmdLen)
{
    return zProcessor::msgParse(message,cmdLen);
}



bool RecordClient::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
#if 0 
    google::protobuf::Message *message = extraceProtoMsg(data,nCmdLen);
    if(!message)
    {
        return false;
    }
    bool ret = false;
    if(!ret)
    {
        Fir::logger->error("%s(%s, %u)", __PRETTY_FUNCTION__, message->GetTypeName().c_str(),nCmdLen);
    }
    SAFE_DELETE(message);
    return ret;
#endif
    Fir::logger->error("RecordClient::msgParseProto 消息没处理");
    return true;

}


bool RecordClient::msgParseRecordCmd(const CMD::RECORD::RecordNull *ptNullCmd, const DWORD nCmdLen)
{
    using namespace CMD::RECORD;
    switch(ptNullCmd->para)
    {
        case PARA_SCENE_USER_PAPER:
            {
                t_PaperUser_SceneRecord *paper = (t_PaperUser_SceneRecord*)ptNullCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(paper->charid);
                if(!user)
                {
                    return false;
                }
                HelloKittyMsgData::SellPaperBase temp;
                try
                {
                    temp.ParseFromArray(paper->data,paper->datasize);
                }
                catch(...)
                {
                    return false;
                }
                //return user->m_trade.returnSellPaper(&temp);
            }
            break;
        case PARA_FAMILYBASE_CREATE_RETURN:
            {
                t_WriteFamily_RecordScene_Create_Return *cmd = (t_WriteFamily_RecordScene_Create_Return*)ptNullCmd; 
                FamilyCmdHandle::DocreateReturn(cmd);
                return true;
            }
            break;
        case PARA_COMMIT_AUCTION:
            {
                t_Commit_Auction *rev = (t_Commit_Auction*)ptNullCmd;
                commitAuction(rev->auctionID);
                return true;
            }
            break;
        case PARA_UNITY_NOTICE:
            {
                t_UnityInfoNotice_RecordScene *rev = (t_UnityInfoNotice_RecordScene*)ptNullCmd;
                SceneUser* pPlayer = SceneUserManager::getMe().getUserByID(rev->playerId);
                if(pPlayer)
                {
                    pPlayer->m_unitybuild.updateCliColInfoByColId(rev->colId);
                }
                return true;
            }
            break;
        case  PARA_UNITY_MAIL:
            {
                t_UnityInfoMail_RecordScene *rev = (t_UnityInfoMail_RecordScene*)ptNullCmd;
                UnityManager::SendMailToInviteFalse(rev->playerId,rev->FriendplayerId,rev->buildid);
                return true;
            }
            break;
        case PARA_CLEAR_WEEK_RANK:
            {
                SceneUser::clearWeekRank();
                return true;
            }
            break;
        case PARA_CLEAR_MONTH_RANK:
            {
                SceneUser::clearMonthRank();
                return true;
            }
            break;
        case PARA_STAR_GAME_OVER:
            {
                t_StarGameOver *rev = (t_StarGameOver*)ptNullCmd;
                SceneUser* user = SceneUserManager::getMe().getUserByID(rev->charID);
                if(user)
                {
                    return user->cancelStarGame(rev->reason);
                }
            }
            break;
        default:
            {
                break;
            }

    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return false;
}

bool RecordClient::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }

    using namespace CMD::RECORD;
    bool ret = false;
    switch(ptNullCmd->cmd)
    {
        case RECORDCMD:
            {
                ret = msgParseRecordCmd((RecordNull*)ptNullCmd,nCmdLen);
            }
            break;
        default:
            {
                break;
            }
    }
    if(ret== false)
        Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return true;
}

bool RecordClient::commitAuction(const DWORD auctionID)
{
    bool ret = false;
    do
    {
        const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Auction_ID);
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(GT_Real);
        if(!handle || !emailConf || !handle->checkSet("config",GT_Real,"gift",auctionID))
        {
            break;
        }
        handle = zMemDBPool::getMe().getMemDBHandle(auctionID);
        if(!handle)
        {
            break;
        }
        DWORD lock = handle->isLock("auction",auctionID,"lockbid");
        DWORD state = handle->getInt("auction",auctionID,"state");
        if(lock || state != BS_End)
        {
            break;
        }
        QWORD ownerID = handle->getInt("auction",auctionID,"owner");
        DWORD reward = handle->getInt("auction",auctionID,"reward");
        handle->getLock("auction",auctionID,"lockbid",1);
        std::set<QWORD> memberSet;
        handle->getSet("auction",auctionID,"autobidset",memberSet);
        std::vector<HelloKittyMsgData::ReplaceWord> argVec;
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(reward);
        if(!confBase)
        {
            break;
        }
        //给竞拍失败者返回代币
        for(auto iter = memberSet.begin();iter != memberSet.end();++iter)
        {
            std::map<DWORD,DWORD> couponsMap;
            QWORD charID = *iter;
            DWORD autoCnt = handle->getInt("autobid",auctionID,charID);
            if(!autoCnt)
            {
                continue;
            }
            couponsMap.insert(std::pair<DWORD,DWORD>(HelloKittyMsgData::Attr_Coupons,autoCnt*400));
            EmailManager::sendEmailBySys(charID,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,couponsMap);
        }
        //给竞拍成功者奖励
        emailConf = tbx::SystemEmail().get_base(Email_Auction_Success_ID);
        if(emailConf && ownerID && reward)
        {
            HelloKittyMsgData::GiftCashInfo giftInfo;
            HelloKittyMsgData::GiftInfo *giftTemp = giftInfo.mutable_gift();
            if(giftTemp)
            {
                giftTemp->set_type(reward);
                giftTemp->set_num(1);
                giftTemp->set_status(HelloKittyMsgData::GS_InWare);
                giftTemp->set_endtime(confBase->itemInfo->time() * 24 * 3600 + SceneTimeTick::currentTime.sec());
            }
            HelloKittyMsgData::GiftOrder *order = giftInfo.mutable_order();
            if(order)
            {
                order->set_id(0);
                order->set_deliverycompany("");
                order->set_deliverynum("");
            }
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(ownerID);
            if(!handleTemp)
            {
                break;
            }
            DWORD SenceId = handleTemp->getInt("playerscene",ownerID,"sceneid");
            if(SenceId)
            {
                SceneUser* user = SceneUserManager::getMe().getUserByID(ownerID);
                if(user)
                {
                    user->m_giftPackage.addGiftCash(giftInfo);
                    user->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Auction_Success_Number,1);
                }
                else
                {
                    CMD::SCENE::t_UserBidReward bidReward;
                    bidReward.charID = ownerID;
                    giftInfo.SerializeToArray(bidReward.data,giftInfo.ByteSize());
                    bidReward.size = giftInfo.ByteSize();

                    std::string msg;
                    encodeMessage(&bidReward,sizeof(bidReward) + bidReward.size,msg);
                    SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,msg.c_str(),msg.size());
                }
            }
            else
            {
                SceneUser* user =  SceneUserManager::getMe().CreateTempUser(ownerID);
                if(user)
                {
                    user->m_giftPackage.addGiftCash(giftInfo);
                    user->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Auction_Success_Number,1);
                }
            }
#if 0
            std::map<DWORD,DWORD> rewardMap;
            rewardMap.insert(std::pair<DWORD,DWORD>(reward,1));
            std::vector<HelloKittyMsgData::ReplaceWord> argVec;
            EmailManager::sendEmailBySys(ownerID,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,rewardMap);
#endif
        }
        //拍卖结束
        SceneUser::sendEndBroadcast(handle,auctionID);
        //广播大厅数据
        SceneUser::sendHistoryBroadcast();
        handle->setInt("auction",auctionID,"state",BS_Reward);
        handle->delLock("auction",auctionID,"lockbid");
        ret = true;
    }while(false);
    return true;
} 

bool ManagerRecordClient::init()
{
    const CMD::SUPER::ServerEntry *serverEntry = SceneService::getMe().getServerEntryByType(RECORDSERVER);
    recordclientPool = new zTCPClientTaskPool(100,8000);
    if (NULL == recordclientPool
            || !recordclientPool->init())
        return false;

    while(serverEntry)
    {
        RecordClient *recordclient = new RecordClient(serverEntry->pstrIP, serverEntry->wdPort,serverEntry->wdServerID);
        if (NULL == recordclient)
        {
            Fir::logger->error("没有足够内存，不能建立record服务器客户端实例");
            return false;
        }
        if(recordclientPool->put(recordclient))
        {
            recordclientPool->put(recordclient);
            if(!recordclient->connect())
            {
                Fir::logger->error("can not connect");
                return false;
            }
            recordclientPool->addCheckwait(recordclient);
        }
        serverEntry = SceneService::getMe().getNextServerEntryByType(RECORDSERVER, &serverEntry);
    }
    return true;
}
/**
 ** \brief 周期间隔进行连接的断线重连工作
 ** \param ct 当前时间
 **/
void ManagerRecordClient::timeAction(const zTime &ct)
{
    if (actionTimer.elapse(ct) > 4)
    {
        if (recordclientPool)
            recordclientPool->timeAction(ct);
        actionTimer = ct;
    }
}
/**
 ** \brief 向容器中添加已经成功的连接
 ** \param recordclient 待添加的连接
 **/
void ManagerRecordClient::add(RecordClient *recordclient)
{
    if (recordclient)
    {
        zRWLock_scope_wrlock scope_wrlock(rwlock);
        m_mapRecord.insert(std::make_pair(recordclient->getServerID(), recordclient));
    }
}

/**
 ** \brief 从容器中移除断开的连接
 ** \param recordclient 待移除的连接
 **/
void ManagerRecordClient::remove(RecordClient *recordclient)
{
    if (recordclient)
    {
        zRWLock_scope_wrlock scope_wrlock(rwlock);
        auto it = m_mapRecord.find(recordclient->getServerID());
        if (it != m_mapRecord.end())
        {
            m_mapRecord.erase(it);
        }
    }
}

bool ManagerRecordClient::reConnectrecord(const CMD::SUPER::ServerEntry *serverEntry)
{
    if (NULL == recordclientPool)
    {
        return false;
    }

    if (serverEntry)
    {
        RecordClient *recordclient = FIR_NEW RecordClient(serverEntry->pstrIP, serverEntry->wdPort,serverEntry->wdServerID);
        if (NULL == recordclient)
        {
            Fir::logger->error("没有足够内存，不能建立record服务器客户端实例");
            return false;
        }
        if(recordclientPool->put(recordclient))
        {
            if(!recordclient->connect())
            {
                Fir::logger->error("can not connect");
                return false;
            }
            recordclientPool->addCheckwait(recordclient);
        }
#ifdef _PQQ_DEBUG
        Fir::logger->debug("[重连档案] 网关重新连接档案 ip=%s,name=%s,port=%d"
                ,serverEntry->pstrIP,serverEntry->pstrName,serverEntry->wdPort);
#endif
    }
    return true;
}

/**
 * \brief 设置到档案的某一连接是否需要重连
 * \param ip 要连接的ip
 * \param port 端口
 * \param reconn 是否重连 true重连，false 不再重连，将会删掉
 */
void ManagerRecordClient::setTaskReconnect(const std::string& ip, unsigned short port, bool reconn)
{
    if (recordclientPool)
    {
        recordclientPool->setTaskReconnect(ip, port, reconn);
    }
}


RecordClient* ManagerRecordClient::GetRecordByServerId(DWORD wServerId)
{
    auto it = m_mapRecord.find(wServerId);
    if(it == m_mapRecord.end())
    {
        return NULL;
    }
    return it->second;

}

RecordClient* ManagerRecordClient::GetRecordByTableName(const std::string &strTableName)
{
    for(auto it = m_mapRecord.begin(); it != m_mapRecord.end(); it++)
    {
        if(!SceneService::getMe().OtherServerhasDBtable(it->first,strTableName))
        {
            continue;
        }
        return it->second;
    }
    return NULL;
}


void  ManagerRecordClient::final()
{
    SAFE_DELETE(recordclientPool);
    zRWLock_scope_wrlock scope_wrlock(rwlock);
    m_mapRecord.clear();

}

void ManagerRecordClient::execEvery()
{
    zRWLock_scope_wrlock scope_wrlock(rwlock);
    for (auto it = m_mapRecord.begin(); it != m_mapRecord.end(); ++it)
    {
        if(it->second)
            it->second->doCmd();
    }


}
