#include "LeaveMessageManager.h"
#include "tbx.h"
#include "zMisc.h"
#include "SceneUser.h"
#include "Misc.h"
#include "zMemDBPool.h"
#include "RedisMgr.h"
#include "SceneUserManager.h"
#include "SceneToOtherManager.h"
#include "TimeTick.h"
#include "wordFilter.h"



LeaveMessageManager::LeaveMessageManager(SceneUser& rUser):m_rUser(rUser)
{

}

LeaveMessageManager::~LeaveMessageManager()
{

}


void LeaveMessageManager::load(const HelloKittyMsgData::Serialize& binary)
{
    for (int i = 0; i < binary.leavemessage_size(); i++) {
        m_mapMessage[binary.leavemessage(i).messgeid()] = binary.leavemessage(i);
    }

}

void LeaveMessageManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto it = m_mapMessage.begin(); it != m_mapMessage.end() ;it++)
    {
        HelloKittyMsgData::ChatMessage *pLeaveMessage = binary.add_leavemessage();
        if(pLeaveMessage)
        {
            *pLeaveMessage = it->second;
        }
    }
}

void LeaveMessageManager::ReqLeaveMessage(const std::string& strMessage,QWORD recvID)
{
    HelloKittyMsgData::AckLeaveMessage ack;
    ack.set_result(HelloKittyMsgData::ChatResult_Suc);
    do
    {
        if(strMessage.size() > MAX_LEAVEMESSAGE)
        {
            ack.set_result(HelloKittyMsgData::ChatResult_MessageTooLong);
            break;
        }
        SceneUser* user = SceneUserManager::getMe().getUserByID(recvID);
        if(user)
        {
            user->m_leavemessage.leaveMessage(m_rUser.charid,strMessage);
            break;
        }
        else
        {
            zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(recvID);
            if(handle)
            {
                DWORD SenceId = handle->getInt("playerscene",recvID,"sceneid");
                if(SenceId != 0)
                {
                    CMD::SCENE::t_leaveMessage sendCmd;
                    sendCmd.ownerid = recvID;
                    sendCmd.sendid = m_rUser.charid;
                    strncpy(sendCmd.chattxt,strMessage.c_str(),MAX_LEAVEMESSAGE);
                    std::string ret;
                    encodeMessage(&sendCmd,sizeof(sendCmd),ret);
                    SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size());
                    break;
                }
                else//创建临时用户
                {
                    if(!handle->getLock("playerlock",recvID,"newplayer",30))
                    {
                        ack.set_result(HelloKittyMsgData::ChatResult_sysbusy); 
                        break;
                    }
                    //玩家不存在，创建角色
                    SceneUser* u  =  SceneUserManager::getMe().CreateTempUser(recvID);
                    if (u)
                    {
                        u->m_leavemessage.leaveMessage(m_rUser.charid,strMessage);

                    }
                    else
                    {
                        ack.set_result(HelloKittyMsgData::ChatResult_Noperson);
                    }
                    handle->delLock("playerlock",recvID,"newplayer");

                }

            }
        }

    }while(0);
    std::string ret;
    encodeMessage(&ack,ret);
    //m_rUser.sendCmdToMe(ret.c_str(),ret.size());
}

DWORD LeaveMessageManager::getNewID()
{
    if(m_mapMessage.empty())
    {
        return 1;
    }
    return (m_mapMessage.rbegin()->first) + 1;
}
void  LeaveMessageManager::getClientMsg(const HelloKittyMsgData::ChatMessage &rData,HelloKittyMsgData::ClientChatMessage* pmessage,QWORD forcharid)
{
    HelloKittyMsgData::ChatMessage* pclint = pmessage->mutable_message();
    if(pclint)
    {
        *pclint = rData;
    }
    HelloKittyMsgData::playerShowbase* pbase =  pmessage->mutable_sendplayer();
    if(!pbase)
        return ;
    SceneUser::getplayershowbase(rData.sendid(),*pbase);

}



void LeaveMessageManager::leaveMessage(QWORD charid,const std::string& strMessage)
{
    HelloKittyMsgData::ChatMessage newMsg;
    newMsg.set_messgeid(getNewID());
    newMsg.set_sendid(charid);
    wordFilter::getMe().doFilter(const_cast<string &>(strMessage));
    newMsg.set_chattxt(strMessage);
    DWORD NowTimer = SceneTimeTick::currentTime.sec();
    newMsg.set_timer(NowTimer);
    m_mapMessage[newMsg.messgeid()] = newMsg;
    //检查留言数，删除多余留言
    SendAddMsg(newMsg.messgeid());
    CheckMsgNum();
    m_rUser.ackOtherInfo();
}

void  LeaveMessageManager::SendAddMsg(const DWORD msgID)
{
    auto iter = m_mapMessage.find(msgID);
    if(iter == m_mapMessage.end())
    {
        return;
    }
    HelloKittyMsgData::AckAddMessage ackMsg;
    HelloKittyMsgData::ClientChatMessage* message = ackMsg.mutable_message();
    if(message)
    {
        getClientMsg(iter->second,message,iter->first);
    }
    
    std::string ret;
    encodeMessage(&ackMsg,ret);
//    m_rUser.sendCmdToMe(ret.c_str(),ret.size());
}

void LeaveMessageManager::SendDelMsg(DWORD dwMsgId)
{
    std::string ret;
    HelloKittyMsgData::AckDelMessage ack;
    ack.set_messgeid(dwMsgId);
    encodeMessage(&ack,ret);
    //m_rUser.sendCmdToMe(ret.c_str(),ret.size());
    m_rUser.ackOtherInfo();

}

void LeaveMessageManager::CheckMsgNum()
{
    DWORD curcount = m_mapMessage.size();
    if(curcount <= MAX_LEAVEMESSAGENUM)
    {
        return ;
    }
    std::vector<DWORD> vecDel;
    for(auto it =m_mapMessage.begin();it != m_mapMessage.end();it++)
    {
        vecDel.push_back(it->first);
        curcount--;
        if(curcount <= MAX_LEAVEMESSAGENUM)
        {
            break;
        }
    }
    for(auto it = vecDel.begin();it != vecDel.end();it++)
    {
        m_mapMessage.erase(*it);
        SendDelMsg(*it);
    }

}

void LeaveMessageManager::ReqDelMessage(DWORD messageid)
{
    do{
        auto it = m_mapMessage.find(messageid);
        if(it == m_mapMessage.end())
        {
            break;
        }
        m_mapMessage.erase(it);
        SendDelMsg(it->first);
    }while(0);
}

void LeaveMessageManager::fillMessage(HelloKittyMsgData::OtherInfo *other)
{
    for(auto iter = m_mapMessage.begin();iter != m_mapMessage.end();++iter)
    {
        HelloKittyMsgData::ClientChatMessage* message = other->add_leavemessage();
        if(message)
        {
            getClientMsg(iter->second,message,iter->first);
        }
    }
}


