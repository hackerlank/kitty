/**
 * \file
 * \version  $Id: LoadClientManager.cpp 42 2013-04-10 07:33:59Z  $
 * \author  王海军, wanghaijun@ztgame.com 
 * \date 2006年01月04日 16时56分05秒 CST
 * \brief 网关到场景数据缓冲发送
 *
 * 
 */

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "LoadClient.h"
#include "LoadClientManager.h"
#include "extractProtoMsg.h"
#include "login.pb.h"  
#include "zMisc.h"

/**
 ** \brief 类的唯一实例指针
 **/
LoadClientManager *LoadClientManager::instance = NULL;

/**
 ** \brief 构造函数
 **/
LoadClientManager::LoadClientManager()
{
    pLoadClientPool = NULL;
    pspecailloadClient = NULL;
    pspecailGameClient = NULL;

}

/**
 ** \brief 析构函数
 **/
LoadClientManager::~LoadClientManager()
{
    SAFE_DELETE(pLoadClientPool);
}

void  LoadClientManager::final()
{
    SAFE_DELETE(pLoadClientPool);
}

void LoadClientManager::visit(DWORD selfcont,DWORD otherconut)
{
    zRWLock_scope_wrlock scope_wrlock(rwlock);

    auto it = allgameClients.find(selfcont);
    if(it == allgameClients.end())
    {
        return ;
    }
    LoadClient * self = it->second;

    it = allgameClients.find(otherconut);
    if(it == allgameClients.end())
    {
        return ;
    }
    LoadClient * other = it->second;
    self->visit(other->getcharid());

}

void LoadClientManager::visiteach(DWORD visitnum)
{
    zRWLock_scope_wrlock scope_wrlock(rwlock);

    if(allgameClients.size() <= 1)
    {
        return;   
    }
    DWORD start = 0;
    LoadClient* pstart = NULL;
    for(auto it = allgameClients.begin();it != allgameClients.end();it++)
    {
        if(start == 0)
        {
            pstart = it->second;
            start++;
        }else if(start == visitnum)
        {
            it->second->visit(pstart->getcharid());
            start = 0;
        }
        else
        {
            it->second->visit(pstart->getcharid());
            start++;
        }


    }

}
void LoadClientManager::visitstep(BYTE rate)
{
    m_autovisitrate = rate;
    if(m_autovisitrate == 0)
    {
        visitself();

    }
}

void LoadClientManager::visitself()
{
    zRWLock_scope_wrlock scope_wrlock(rwlock);

    for(auto it = allgameClients.begin();it != allgameClients.end();it++)
    {
        it->second->visit(it->second->getcharid());

    }


}

void LoadClientManager::opbuild(DWORD selfcont,DWORD buildid,DWORD isicon)
{
    zRWLock_scope_wrlock scope_wrlock(rwlock);

    auto it = allgameClients.find(selfcont);
    if(it == allgameClients.end())
    {
        return ;
    }
    LoadClient * self = it->second;
    self->opbuild(buildid,isicon);

}

/**
 ** \brief 初始化管理器
 ** \return 初始化是否成功
 **/
bool LoadClientManager::Load(DWORD loadNum,BYTE lang)
{
    std::string flIp =  Fir::global["login_ip"];
    SDWORD flPort = atoi(Fir::global["login_port"].c_str());

    for(DWORD i =0; i < loadNum;i++)
    {
        LoadClient *pLoadClient = new LoadClient(flIp,flPort,m_NextAccount,lang);
        if (NULL == pLoadClient)
        {
            Fir::logger->error("没有足够内存，不能建立Load服务器客户端实例");
            return false;
        }
        pLoadClient->setDeleteFlag(true);
        pLoadClientPool->put(pLoadClient);
        pLoadClient->connect();
        pLoadClientPool->addCheckwait(pLoadClient);
        m_NextAccount++;
    }
    return true;
}
bool LoadClientManager::Loadspecail(const std::string account, BYTE plattype,const std::string& strpwd,BYTE lang)
{
    std::string flIp =  Fir::global["login_ip"];
    SDWORD flPort = atoi(Fir::global["login_port"].c_str());

    LoadClient *pLoadClient = new LoadClient(flIp,flPort, account,plattype,lang,strpwd);
    if (NULL == pLoadClient)
    {
        Fir::logger->error("没有足够内存，不能建立Load服务器客户端实例");
        return false;
    }
    pLoadClient->setDeleteFlag(true);
    pLoadClientPool->put(pLoadClient);
    pLoadClient->connect();
    pLoadClientPool->addCheckwait(pLoadClient);
    return true;


}

bool LoadClientManager::init()
{
    pLoadClientPool = new zTCPClientTaskPool(10000);
    if (NULL == pLoadClientPool
            || !pLoadClientPool->init())
        return false;
    m_NextAccount = 10000;
    m_stepBase = 0;
    m_stepnumUper = 0;
    m_stepnum = 0;
    m_autovisitrate = 0;
    return true;
}

/**
 ** \brief 周期间隔进行连接的断线重连工作
 ** \param ct 当前时间
 **/
void LoadClientManager::timeAction()
{
    if(m_stepnumUper  < m_stepnum + m_stepBase )
    {
        DWORD nownum = m_stepnumUper - m_stepBase;
        DWORD NeedNum = m_stepnum - nownum;
        DWORD loadNum = NeedNum > 100 ? 100 : NeedNum;
        std::string flIp =  Fir::global["login_ip"];
        SDWORD flPort = atoi(Fir::global["login_port"].c_str());

        for(DWORD i =0; i < loadNum;i++)
        {
            LoadClient *pLoadClient = new LoadClient(flIp,flPort,m_stepnumUper,0);
            if (NULL == pLoadClient)
            {
                Fir::logger->error("没有足够内存，不能建立Load服务器客户端实例");
                continue;
            }
            pLoadClient->setDeleteFlag(true);
            pLoadClientPool->put(pLoadClient);
            pLoadClient->connect();
            pLoadClientPool->addCheckwait(pLoadClient);
            m_stepnumUper++;
        }


    }
    zRWLock_scope_wrlock scope_wrlock(rwlock);

    if(allgameClients.size() <= 1)
    {
        return;   
    }
    if(!m_autovisitrate)
        return ;
    for(auto it = allgameClients.begin();it != allgameClients.end();it++)
    {
        BYTE rate = zMisc::randBetween(0,100);
        if(rate > m_autovisitrate)
        {
            continue;
        }
        DWORD Account =  m_vectorRobot[zMisc::randBetween(0,m_vectorRobot.size() - 1)];
        LoadClient * pOtherclient = getclient(Account,false);
        if(pOtherclient ==NULL)
        {

            continue;
        }
        it->second->visit(pOtherclient->getcharid()); 
    }


}

/**
 ** \brief 向容器中添加已经成功的连接
 ** \param LoadClient 待添加的连接
 **/
void LoadClientManager::addLoad(LoadClient *pLoadClient)
{
    allloadClients[pLoadClient->getAccount()]= pLoadClient;

}

void LoadClientManager::addGame(LoadClient *pLoadClient)
{
    zRWLock_scope_wrlock scope_wrlock(rwlock);

    allgameClients[pLoadClient->getAccount()]= pLoadClient;
    m_vectorRobot.push_back(pLoadClient->getAccount());
}

void LoadClientManager::removeLoad(LoadClient *pLoadClient)
{
    allloadClients.erase(pLoadClient->getAccount());
}

void LoadClientManager::removeGame(LoadClient *pLoadClient)
{
    zRWLock_scope_wrlock scope_wrlock(rwlock);

    allgameClients.erase(pLoadClient->getAccount());
}

bool LoadClientManager::specailconnectToGateway(std::string account,BYTE platType,std::string& strpwd,const std::string& strIP, SDWORD wPort,BYTE lang)
{
    LoadClient *pLoadClient = new LoadClient(strIP,wPort,account,platType,lang,strpwd);
    pLoadClient->setLoadState(flgetGateWay);
    if (NULL == pLoadClient)
    {
        Fir::logger->error("没有足够内存，不能建立Load服务器客户端实例");
        return false;
    }
    pLoadClient->setDeleteFlag(true);
    pLoadClientPool->put(pLoadClient);
    pLoadClient->connect();
    pLoadClientPool->addCheckwait(pLoadClient);
    return true;


}

bool  LoadClientManager::connectToGateway(DWORD id,const std::string& strIP, SDWORD wPort,BYTE lang)
{
    LoadClient *pLoadClient = new LoadClient(strIP,wPort,id,lang);
    pLoadClient->setLoadState(flgetGateWay);
    if (NULL == pLoadClient)
    {
        Fir::logger->error("没有足够内存，不能建立Load服务器客户端实例");
        return false;
    }
    pLoadClient->setDeleteFlag(true);
    pLoadClientPool->put(pLoadClient);
    pLoadClient->connect();
    pLoadClientPool->addCheckwait(pLoadClient);
    return true;

}

void LoadClientManager::showload()
{
    Fir::logger->warn("---------------show load begin----------------");
    Fir::logger->warn("total person %u,command load %u ",DWORD(allgameClients.size() + allloadClients.size()),m_NextAccount - 10000 + (m_stepnumUper- m_stepBase) );
    Fir::logger->warn("wait load person %u........................",DWORD(allloadClients.size()));
    std::map<LoadState,DWORD> ctfl;
    for(auto it = allloadClients.begin();it != allloadClients.end();it++)
    {
        LoadClient *pLoadClient = it->second;
        if(pLoadClient)
        {
            auto pos = ctfl.find(pLoadClient->getLoadState());
            if(pos == ctfl.end())
            {
                ctfl[pLoadClient->getLoadState()] = 1;
            }
            else
            {
                pos ->second++;
            }

        }
    }
    for(auto it = ctfl.begin();it != ctfl.end();it++)
    {
        Fir::logger->warn("state %u : num :%u",it->first,it->second);

    }
    Fir::logger->warn(" game person %u........................",DWORD(allgameClients.size()));
    ctfl.clear();
    zRWLock_scope_wrlock scope_wrlock(rwlock);

    for(auto it = allgameClients.begin();it != allgameClients.end();it++)
    {
        LoadClient *pLoadClient = it->second;
        if(pLoadClient)
        {
            auto pos = ctfl.find(pLoadClient->getLoadState());
            if(pos == ctfl.end())
            {
                ctfl[pLoadClient->getLoadState()] = 1;
            }
            else
            {
                pos ->second++;
            }

        }
    }
    for(auto it = ctfl.begin();it != ctfl.end();it++)
    {
        Fir::logger->warn("state %u : num :%u",it->first,it->second);

    }
    Fir::logger->warn("----------------show load end--------");

}

LoadClient * LoadClientManager::getclient(DWORD dwcount,bool bneedlock)
{
    if(bneedlock)
    {
        zRWLock_scope_wrlock scope_wrlock(rwlock);

        auto it = allgameClients.find(dwcount);
        if(it == allgameClients.end())
        {
            return NULL;
        }
        return  it->second;
    }
    else
    {
        auto it = allgameClients.find(dwcount);
        if(it == allgameClients.end())
        {
            return NULL;
        }
        return  it->second;


    }
return NULL;
}
