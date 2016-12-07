/**
 * \file
 * \version  $Id: GateUserManager.cpp 67 2013-04-23 09:44:20Z  $
 * \author  ,@163.com
 * \date 2005年04月01日 11时56分57秒 CST
 * \brief 实现网关用户管理类
 */

#include "GateUserManager.h"
#include "GatewayServer.h"
#include "SceneClient.h"
#include "GatewayTask.h"
#include "login.pb.h"
#include "TimeTick.h"
#include "system.pb.h"
#include "RedisMgr.h"

GateUserManager::GateUserManager()
{
}

GateUserManager::~GateUserManager()
{
}

struct UnloadAllExec
{
    void operator()(GateUser *u)
    {
        if(NULL == u) return;

        //Fir::logger->trace("用户%dds(%ld)因卸载场景注销",pUser->nickname,pUser->id);
        u->TerminateWait();
    }
};

void GateUserManager::removeAllUser()
{
    UnloadAllExec exec;
    execAll(exec);
}

struct UnloadSceneExec
{
    SceneClient *scene;

    UnloadSceneExec(SceneClient *s):scene(s)
    {}

    void operator()(GateUser *u)
    {
        if(NULL == u) return;

        if(u->scene == scene)
        {
            Fir::logger->trace("用户(charid=%lu,nickname=%s)因卸载场景注销",u->charid,u->nickname);
            u->TerminateWait();
        }
    }
};

void GateUserManager::removeUserBySceneClient(SceneClient *scene)
{
    UnloadSceneExec exec(scene);
    execAll(exec);
}

/**
 * \brief 根据accid得到一个用户
 *
 *
 * \param accid: 角色的accid
 * \return 用户
 */
GateUser * GateUserManager::getUserAccid(DWORD accid)
{
    GateUser *ret =NULL;

    arwlock.rdlock();

    auto itrFind = m_mapAccids.find(accid);
    if(itrFind != m_mapAccids.end())
    {
        ret = itrFind->second;
    }

    arwlock.unlock();
    return ret;
}

/**
 * \brief 从网关管理器中删除一个用户
 *
 *
 * \param accid: 好删除用户的accid
 */
void GateUserManager::removeUserAccid(DWORD accid)
{
    arwlock.wrlock();

    m_mapAccids.erase(accid);

    arwlock.unlock();
}

/**
 * \brief 将一个用户添加到到网关用户管理器中
 *
 *
 * \param user: 需要添加到管理器中的用户
 * \return 添加成功返回图若,否则返回false 
 */
bool GateUserManager::addUserAccid(GateUser *user)
{
    if(NULL == user) return false;

    arwlock.wrlock();
    bool ret=m_mapAccids.insert(std::make_pair(user->accid,user)).second;
    arwlock.unlock();
    return ret;
}


GateUser * GateUserManager::getUserCharid( QWORD charid)
{
    GateUser *ret = NULL;

    charid_lock.rdlock();
    auto itrFind = m_mapUsers.find(charid);
    if(itrFind != m_mapUsers.end())
    {
        ret = itrFind->second;
    }
    charid_lock.unlock();

    return ret;
}

DWORD GateUserManager::getRoleCount()
{
    DWORD count = 0;

    charid_lock.rdlock();
    count = m_mapUsers.size();
    charid_lock.unlock();

    return count;
}

bool GateUserManager::addUserCharid(GateUser *user)
{
    bool ret = false;
    GateUser* tmp=NULL;
    charid_lock.wrlock();

    tmp = getUserAccount(user->acctype, user->account);
    if(tmp)
    {
        ret = m_mapUsers.insert(std::make_pair(user->charid,user)).second;
    }

    charid_lock.unlock();
    if(tmp==NULL)
    {
        Fir::logger->error("[异常]:%s,%u",__PRETTY_FUNCTION__,((GateUser*)user)->accid);
    }

    return ret;
}


void GateUserManager::removeUserCharid(QWORD charid)
{
    charid_lock.wrlock();
    m_mapUsers.erase(charid);
    charid_lock.unlock();
}

GateUser* GateUserManager::getUserAccount(DWORD platform, const std::string& account)
{
    GateUser* ret = NULL;
    plat_account_rwlock.rdlock();
    auto it = account_map.find(std::pair<DWORD,std::string>(platform,account));
    if (it != account_map.end())
        ret = (GateUser*)it->second;
    plat_account_rwlock.unlock();
    return ret;
}

bool GateUserManager::addUserAccount(GateUser* user)
{
    if (!user)
    {
        return false;
    }

    bool ret = false;
    plat_account_rwlock.wrlock();
    if (account_map.find(std::pair<DWORD,std::string>(user->acctype, user->account)) == account_map.end())
    {
        ret = account_map.insert(std::make_pair(std::pair<DWORD,std::string>(user->acctype, user->account),user)).second;
    }
    plat_account_rwlock.unlock();
    return ret;
}

void GateUserManager::removeUserAccount(DWORD platform, const std::string& account)
{
    plat_account_rwlock.wrlock();
    account_map.erase(std::pair<DWORD,std::string>(platform,account));
    plat_account_rwlock.unlock();
}

void GateUserManager::sendChatToCity(DWORD cityid,const void *pstrCmd,const unsigned int nCmdLen)
{
    int _sendLen;                                   
    t_StackCmdQueue cmd_queue;              
    DWORD cmdLen = nCmdLen;                   
    _sendLen = zSocket::packetPackZip(pstrCmd,cmdLen,cmd_queue); 
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cityid);
    if(handle ==NULL)
        return ;
    charid_lock.rdlock();                       
    auto iter = m_mapUsers.begin();
    for(;iter != m_mapUsers.end() ; ++iter)
    { 
        CharBase charbase; 
        if(!RedisMgr::getMe().get_charbase(iter->first,charbase)) 
        {
            continue;
        }
        if(charbase.areaType != cityid)
        {
            continue;
        }
        (iter->second)->sendCmd(cmd_queue.rd_buf(),_sendLen,true);
    } 
    charid_lock.unlock();     


}
void GateUserManager::sendChatToworld(const void *pstrCmd, const unsigned int nCmdLen, BYTE lang)
{
    int _sendLen;                                   
    t_StackCmdQueue cmd_queue;              
    DWORD cmdLen = nCmdLen;                   
    _sendLen = zSocket::packetPackZip(pstrCmd,cmdLen,cmd_queue); 

    charid_lock.rdlock();                       
    auto iter = m_mapUsers.begin();
    for(;iter != m_mapUsers.end() ; ++iter)
    { 
        if (iter->second->lang == lang)
        {                      
            (iter->second)->sendCmd(cmd_queue.rd_buf(),_sendLen,true);
        }
    } 
    charid_lock.unlock();     

}

void GateUserManager::sendCmdToWorld(const void *pstrCmd, const unsigned int nCmdLen,bool worldChat)
{                                               
    int _sendLen;                                   
    t_StackCmdQueue cmd_queue;              
    DWORD cmdLen = nCmdLen;                   
    _sendLen = zSocket::packetPackZip(pstrCmd,cmdLen,cmd_queue); 

    charid_lock.rdlock();                       
    auto iter = m_mapUsers.begin();
    if (worldChat)
    {
        for(;iter != m_mapUsers.end() ; ++iter)
        { 
            if (iter->second->needReceiveWorldChat)
            {                      
                (iter->second)->sendCmd(cmd_queue.rd_buf(),_sendLen,true);
            }
        } 
    }else
    {
        for(;iter != m_mapUsers.end() ; ++iter)
        {                
            (iter->second)->sendCmd(cmd_queue.rd_buf(),_sendLen,true);
        } 
    }                                      
    charid_lock.unlock();                       
}

bool GateUserManager::syncGameTime()
{
    charid_lock.rdlock();
    HelloKittyMsgData::AckHeartBeat message;
    message.set_gametime(GatewayTimeTick::currentTime.sec());
    std::string ret;
    encodeMessage(&message,ret);
    std::set<QWORD> terminateSet;
    for(auto iter = m_mapUsers.begin();iter != m_mapUsers.end();++iter)
    {
        GateUser *user = const_cast<GateUser*>(iter->second);
        if(user)
        {
            if(user->clentIsTerminate())
            {
                terminateSet.insert(iter->first);
            }
            else
            {
                user->sendCmd(ret.c_str(),ret.size());
                user->addHeartTime();
            }
        }
    }
    charid_lock.unlock();
    //超时就断开吧
    for(auto iter = terminateSet.begin();iter != terminateSet.end();++iter)
    {
        GateUser *user = getUserCharid(*iter);
        if(user)
        {
            user->Terminate();
            Fir::logger->debug("[心跳包] 超时没有收到心跳包断开连接(%lu,%s)",user->charid,user->account.c_str());
            user->resetHearTime();
        }
    }
    return true;
}                                       

void GateUserManager::sendCmd(const void *pstrCmd, const DWORD nCmdLen)
{
    charid_lock.rdlock();
    for(auto itr = m_mapUsers.begin();itr != m_mapUsers.end();++itr)
    {
        itr->second->sendCmd(pstrCmd,nCmdLen);
    }
    charid_lock.unlock();
}
