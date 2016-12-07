/**
 * \file
 * \version  $Id: GateUser.cpp 67 2013-04-23 09:44:20Z  $
 * \author  ,@163.com
 * \date 2005年04月01日 11时56分41秒 CST
 * \brief 实现网关用户类
 */

#include <stdlib.h>
#include "GateUser.h"
#include "GateUserManager.h"
#include "RecordCommand.h"
#include "RecordClient.h"
#include "GatewayTask.h"
#include "SceneClient.h"
#include "GatewayServer.h"
#include "TimeTick.h"
#include "SceneClientManager.h"

DWORD GateUser::HeartTime = 60;
GateUser::GateUser(DWORD accID,GatewayTask *histask)
{
    this->accid=accid;		
    this->charid=0;		
    initState();
    gatewaytask=histask;
    scene=NULL;
    scene_id=0;
    acctype = 0;
    m_byCreate = 0;
    heroid = 0;
    lang = 0;
    needReceiveWorldChat = false;
    if(gatewaytask)
    {
        gatewaytask->m_pUser=this;
    }
    bzero(nickname,sizeof(nickname));
    heartTime = 0;
    reconnect = false;
}

GateUser::GateUser(const std::string& _uuid,GatewayTask *thistask)
{
    this->accid=0;		
    this->charid=0;		
    initState();
    gatewaytask=thistask;
    scene=NULL;
    scene_id=0;
    acctype = 0;
    m_byCreate = 0;
    lang = 0;

    if(gatewaytask)
    {
        gatewaytask->m_pUser=this;
    }
    bzero(nickname,sizeof(nickname));
    heartTime = 0;
    reconnect = false;
}

GateUser::GateUser(WORD wdlogintype, const std::string& strPlatAccount, GatewayTask* thistask)
{
    this->accid=0;		
    this->charid=0;		
    initState();
    gatewaytask=thistask;
    scene=NULL;
    scene_id=0;
    acctype = wdlogintype;
    m_byCreate = 0;
    this->account = strPlatAccount;
    lang = 0;

    if(gatewaytask)
    {
        gatewaytask->m_pUser=this;
    }
    bzero(nickname,sizeof(nickname));
    heartTime = 0;
    reconnect = false;
}

GateUser::~GateUser()
{
    Fir::logger->debug(__PRETTY_FUNCTION__);
}


/**
 * \brief 卸载一个网关用户的信息
 *
 *
 */
void GateUser::final()
{
    Fir::logger->debug("[GS],charid=%lu,%s,结束",this->charid,this->nickname);

    lock();

    GateUserManager::getMe().removeUserCharid(this->charid);

    GateUserManager::getMe().removeUserAccount(this->acctype, this->account);

    unlock();

    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(acctype);
    if (handle) 
    {
        handle->setInt("gatewaylogin", this->acctype, this->account.c_str(), "state", GATEWAY_USER_LOGINSESSION_STATE_NONE);
    }
}


/**
 * \brief 设置游戏状态
 *
 *
 */
void GateUser::playState(SceneClient *s , DWORD scene_tempid)
{
    lock();
    if(scene_tempid)
    {
        this->scene_id=scene_tempid;
    }
    if(s)
    {
        this->scene=s;
    }
    systemstate = SYSTEM_STATE_PLAY;
    unlock();
}

/**
 * \brief 对选择到一个角色进行初始处理
 *
 *
 * \return 处理是否成功
 */
bool GateUser::beginSelect()
{
    zMutex_scope_lock lock(mlock);

    if (!this->syncMemDBBase())
    {
        Fir::logger->debug("[客户端登录_3]:网关选择登录失败(网关向rides获取昵称失败,%lu,%u,%s)",charid,acctype,account.c_str());
        return false;
    }
    //进入等待游戏状态，然后开始向会话注册，会话注册到场景，成功读档后，返回场景注册成功消息，再进入游戏状态
    if(!this->reg())
    {
        Fir::logger->debug("[客户端登录_3]:网关选择登录失败(网关向场景注册失败,%lu,%u,%s)",charid,acctype,account.c_str());
        return false;
    }
    
    Fir::logger->debug("[客户端登录_3]:网关选择登录成功(等待网关向场景注册,%lu,%u,%s)",charid,acctype,account.c_str());
    return true;
}
/**
 * \brief 发送数据给客户端
 *
 *
 * \param pstrCMD: 发送的指令
 * \param nCmdLen: 指令长度
 * \return 发送是否成功
 */
bool GateUser::sendCmd(const void *pstrCmd, const unsigned int nCmdLen,const bool hasPacked)
{
    if (!hasPacked)
    {
        if(gatewaytask)
        {
            return gatewaytask->sendCmd(pstrCmd,nCmdLen);
        }
    }else
    {
        if(gatewaytask)
        {
            return gatewaytask->sendCmdNoPack(pstrCmd,nCmdLen);
        }
    }
    return false;
}

/**
 * \brief 网关注销一个用户
 * \param out: false by default
 *
 */
void GateUser::unreg()
{
    //防止锁加的太大
    bool need=false;

    if (isWaitUnregState())
    {
        return;
    }

    lock();

    if(isWaitPlayState() || isPlayState())
    {
        need=true;

        this->waitUnregState();
    }

    unlock();

    if(need)
    {
        CMD::SCENE::t_unregUser_gatescene send;
        send.charid=charid;
        send.retcode=CMD::SCENE::UNREGUSER_RET_LOGOUT;
        SceneClient * pScene = SceneClientManager::getMe().getSceneByID(scene_id);
        if(pScene)
        {
            std::string ret;
            encodeMessage(&send,sizeof(send),ret);
            pScene->sendCmd(ret.c_str(),ret.size());
            Fir::logger->trace("[登录注销],charid=%lu,nickname=%s, 向会话注销",charid,nickname);
        }
    }
}

/**
 * \brief 网关注册一个用户
 *
 */
bool GateUser::reg()
{
    CMD::SCENE::t_regUser_Gatescene send;
    send.accid=accid;
    send.charid=charid;
    strncpy((char *)send.name, nickname,MAX_NAMESIZE);
    strncpy(send.flat,strFlat.c_str(),MAX_FLAT_LENGTH);
    strncpy(send.phone_uuid,strPhoneUuid.c_str(),100);
    send.byCreate = m_byCreate;
    send.heroid = heroid;
    send.acctype = this->acctype;
    send.lang = lang;
    send.reconnect = reconnect;
    strncpy(send.account, account.c_str(), MAX_ACCNAMESIZE);
    //for change
    //先查看玩家在哪一个scene,如果找不到，那么选个最小的
    SceneClient *pScene =NULL;
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid);
    if(handle)
    {
        DWORD sceneid = handle->getInt("playerscene",charid,"sceneid");
        if(sceneid > 0)
        {
            pScene = SceneClientManager::getMe().getSceneByID(sceneid);      
        }
        else
        {
            pScene = SceneClientManager::getMe().getMinScene();

        }
        Fir::logger->debug("[客户端登录_3]:网关向场景注册成功(选择场景服务器,%lu,%u,%s)",charid,acctype,account.c_str());
    }
    if(!pScene)
    {
        Fir::logger->debug("[客户端登录_3]:网关向场景注册失败(场景服务器为空,%lu,%u,%s)",charid,acctype,account.c_str());
        return false;

    }
    else
    {
        if(!handle->getLock("playerlock",charid,"newplayer",30))
        {
            Fir::logger->debug("[客户端登录_3]:网关向场景注册失败(锁定注册场景角色,%lu,%u,%s)",charid,acctype,account.c_str());
            return false;
        }
    }

    std::string ret;
    encodeMessage(&send,sizeof(send),ret);
    pScene->sendCmd(ret.c_str(),ret.size());
    waitPlayState();
    Fir::logger->debug("[客户端登录_3]:网关向场景注册成功(网关向场景服务器注册,%lu,%u,%s)",charid,acctype,account.c_str());
    reconnect = false;
    return true;
}

/**
 * \brief 中断连接
 *
 */
void GateUser::TerminateWait()
{
    if(gatewaytask)
        gatewaytask->TerminateWait();
}

/**
 * \brief 是否处于等于中断状态
 *
 */
bool GateUser::isTerminateWait()
{
    if(gatewaytask)
        return gatewaytask->isTerminateWait();

    return true;
}

void GateUser::Terminate()
{
    if (gatewaytask)
    {
        return gatewaytask->Terminate();
    }
}

/**
 * \brief 是否已经中断
 *
 */
bool GateUser::isTerminate()
{
    if (gatewaytask)
        return gatewaytask->isTerminate();

    return true;
}

bool GateUser::syncMemDBBase()
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        unlock();
        return false;
    }
    std::string nickName = std::string(handle->get("rolebaseinfo", this->charid, "nickname"));
    firstrncpy(this->nickname, nickName.c_str() , MAX_NAMESIZE);
    return true;
}
