/**
 * \file
 * \version  $Id: SceneClientManager.cpp 42 2013-04-10 07:33:59Z  $
 * \author  王海军, wanghaijun@ztgame.com 
 * \date 2006年01月04日 16时56分05秒 CST
 * \brief 网关到场景数据缓冲发送
 *
 * 
 */

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "SceneClient.h"
#include "SceneClientManager.h"
#include "zXMLParser.h"
#include "GatewayServer.h"

/**
 ** \brief 类的唯一实例指针
 **/
SceneClientManager *SceneClientManager::instance = NULL;

/**
 ** \brief 构造函数
 **/
SceneClientManager::SceneClientManager()
{
    sceneClientPool = NULL;
}

/**
 ** \brief 析构函数
 **/
SceneClientManager::~SceneClientManager()
{
    SAFE_DELETE(sceneClientPool);
}

void  SceneClientManager::final()
{
    SAFE_DELETE(sceneClientPool);
}

/**
 ** \brief 初始化管理器
 ** \return 初始化是否成功
 **/
bool SceneClientManager::init()
{
    const CMD::SUPER::ServerEntry *serverEntry = GatewayService::getMe().getServerEntryByType(SCENESSERVER);
    sceneClientPool = new zTCPClientTaskPool(100,8000);
    if (NULL == sceneClientPool
            || !sceneClientPool->init())
        return false;

    while(serverEntry)
    {
        SceneClient *sceneClient = new SceneClient("Scene服务器", serverEntry);
        if (NULL == sceneClient)
        {
            Fir::logger->error("没有足够内存，不能建立Scene服务器客户端实例");
            return false;
        }
        if(sceneClientPool->put(sceneClient))
        {
            if(!sceneClient->connect())
            {
                Fir::logger->error("connect scen err");
                return false;
            }
            sceneClientPool->addCheckwait(sceneClient);
        }
        serverEntry = GatewayService::getMe().getNextServerEntryByType(SCENESSERVER, &serverEntry);
    }
    return true;
}

/**
 ** \brief 周期间隔进行连接的断线重连工作
 ** \param ct 当前时间
 **/
void SceneClientManager::timeAction(const zTime &ct)
{
    if (actionTimer.elapse(ct) > 4)
    {
        if (sceneClientPool)
            sceneClientPool->timeAction(ct);
        actionTimer = ct;
    }
}

/**
 ** \brief 向容器中添加已经成功的连接
 ** \param sceneClient 待添加的连接
 **/
void SceneClientManager::add(SceneClient *sceneClient)
{
    if (sceneClient)
    {
        zRWLock_scope_wrlock scope_wrlock(rwlock);
        allClients.insert(value_type(sceneClient->getServerID(), sceneClient));
    }
}

/**
 ** \brief 从容器中移除断开的连接
 ** \param sceneClient 待移除的连接
 **/
void SceneClientManager::remove(SceneClient *sceneClient)
{
    if (sceneClient)
    {
        zRWLock_scope_wrlock scope_wrlock(rwlock);
        iter it = allClients.find(sceneClient->getServerID());
        if (it != allClients.end())
        {
            allClients.erase(it);
        }
    }
}

/**
 ** \brief 向成功的所有连接广播指令
 ** \param pstrCmd 待广播的指令
 ** \param nCmdLen 待广播指令的长度
 **/
bool SceneClientManager::broadcastOne(const void *pstrCmd, int nCmdLen)
{
    return false;
}

bool SceneClientManager::sendTo(const DWORD tempid, const void *pstrCmd, int nCmdLen)
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
bool SceneClientManager::reConnectScene(const CMD::SUPER::ServerEntry *serverEntry)
{
    if (NULL == sceneClientPool)
    {
        return false;
    }

    if (serverEntry)
    {
        SceneClient *sceneClient = FIR_NEW SceneClient("Scene服务器", serverEntry);
        if (NULL == sceneClient)
        {
            Fir::logger->error("没有足够内存，不能建立Scene服务器客户端实例");
            return false;
        }
        if(sceneClientPool->put(sceneClient))
        {
            if(!sceneClient->connect())
            {
                Fir::logger->error("connect scen err");
                return false;
            }
            sceneClientPool->addCheckwait(sceneClient);
        }

#ifdef _PQQ_DEBUG
        Fir::logger->debug("[重连场景] 网关重新连接场景 ip=%s,name=%s,port=%d"
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
void SceneClientManager::setTaskReconnect(const std::string& ip, unsigned short port, bool reconn)
{
    if (sceneClientPool)
    {
        sceneClientPool->setTaskReconnect(ip, port, reconn);
    }
}

SceneClient* SceneClientManager::getSceneByID(DWORD id)
{
    SceneClient_map::iterator iter = allClients.find(id);
    if (iter!=allClients.end())
        return (SceneClient*)iter->second;

    return NULL;

}

SceneClient* SceneClientManager::getMinScene()
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(handle == NULL)
        return NULL;
    SceneClient* pMinScene = NULL;
    DWORD MinNum = 0;
    for(auto iter = allClients.begin(); iter != allClients.end();iter++)
    {
        if(pMinScene == NULL)
        {
            pMinScene = iter->second;
            MinNum = handle->getInt("playerscene",iter->first,"secnenum");
            continue;
        }
        DWORD tepNum = handle->getInt("playerscene",iter->first,"secnenum");
        if(tepNum < MinNum)
        {
            pMinScene =iter->second;
            MinNum = tepNum;
        }

    }
    return pMinScene;

}

bool SceneClientManager::isAllStartOK()
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
