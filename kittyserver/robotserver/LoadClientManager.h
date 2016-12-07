/**
 * \file
 * \version  $Id: LoadClientManager.h 42 2013-04-10 07:33:59Z  $
 * \author  王海军, wanghaijun@ztgame.com 
 * \date 2006年01月04日 16时55分37秒 CST
 * \brief 网关到场景数据缓冲发送
 *
 * 
 */


#ifndef _LoadCLIENTMANAGER_H_
#define _LoadCLIENTMANAGER_H_

#include <map>
#include <set>

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "LoadClient.h"
#include "zTime.h"
#include "zRWLock.h"

/**
 ** \brief 定义服务器信息采集连接的客户端管理容器
 **/
class LoadClientManager : public Singleton<LoadClientManager>
{
    friend class Singleton<LoadClientManager>;
    public:

    ~LoadClientManager();
    bool Load(DWORD loadNum,BYTE lang); 
    bool init();
    void timeAction();
    void addLoad(LoadClient *pLoadClient);
    void addGame(LoadClient *pLoadClient);
    void removeLoad(LoadClient *pLoadClient);
    void removeGame(LoadClient *pLoadClient);
    void final();
    bool connectToGateway(DWORD id,const std::string& strIP, SDWORD wPort,BYTE lang);
    void showload();
    void visit(DWORD selfcont,DWORD otherconut);
    void opbuild(DWORD selfcont,DWORD buildid,DWORD isicon);
    LoadClient * getclient(DWORD dwcount,bool bneedLock = true);
    void visiteach(DWORD visitnum);
    void visitself();
    bool Loadspecail(const std::string account, BYTE plattype,const std::string& strpwd,BYTE lang);
    void Set_Loadspecail(LoadClient *pclient){ pspecailloadClient = pclient;}
    LoadClient * getLoadspecail(){return pspecailloadClient;}
    void set_Gamespecail(LoadClient *pclient) { pspecailGameClient = pclient;}
    LoadClient * getGamespecail(){return pspecailGameClient;}
    bool specailconnectToGateway(std::string account,BYTE platType,std::string& strpwd,const std::string& strIP, SDWORD wPort,BYTE lang);
    void loadstep(DWORD dwBase,DWORD dwNum)
    {
        m_stepBase = dwBase;
        m_stepnum = dwNum;
        m_stepnumUper = dwBase;
    }
    void visitstep(BYTE rate);


    private:
    LoadClientManager();
    static LoadClientManager *instance;

    /**
     ** \brief 客户端连接管理池
     **/
    zTCPClientTaskPool *pLoadClientPool;
    /**
     ** \brief 存放连接已经成功的连接容器类型
     **/
    typedef std::map<const DWORD, LoadClient *> LoadClient_map;

    typedef LoadClient_map::iterator iter;
    typedef LoadClient_map::const_iterator const_iter;
    typedef LoadClient_map::value_type value_type;
    /**
     ** \brief 存放连接已经成功的连接容器
     **/
    LoadClient_map allgameClients;
    LoadClient_map allloadClients;
    LoadClient * pspecailloadClient;
    LoadClient * pspecailGameClient;
    std::vector<DWORD> m_vectorRobot;
    zRWLock rwlock;


    /**
     ** \brief 容器访问读写锁
     **/
    private:
    DWORD m_NextAccount;
    DWORD m_stepBase;
    DWORD m_stepnum;
    DWORD m_stepnumUper;
    BYTE m_autovisitrate ;

};

#endif

