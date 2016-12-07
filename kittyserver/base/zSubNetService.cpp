/**
 * \file
 * \version  $Id: zSubNetService.cpp 23 2013-03-30 04:56:22Z  $
 * \author  ,@163.com
 * \date 2004年11月29日 17时19分12秒 CST
 * \brief 实现网络服务器的框架代码
 *
 * 
 */

#include <iostream>
#include <string>
#include <deque>
#include <ext/numeric>

#include "zService.h"
#include "zThread.h"
#include "zSocket.h"
#include "zTCPServer.h"
#include "zNetService.h"
#include "zSubNetService.h"
#include "Fir.h"
#include "SuperCommand.h"
#include "extractProtoMsg.h"
#include "dataManager.h"

zSubNetService *zSubNetService::subNetServiceInst = NULL;

/**
 * \brief 服务器管理器的连接客户端类
 *
 */
class SuperClient : public zTCPBufferClient
{

    public:

        friend class zSubNetService;

        /**
         * \brief 构造函数
         *
         */
        SuperClient() : zTCPBufferClient("服务器管理器客户端"), verified(false)
    {
        //Fir::logger->debug(__PRETTY_FUNCTION__);
    }

        /**
         * \brief 析构函数
         *
         */
        ~SuperClient() {};
        void run();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen); 
    private:
        //处理superCmd消息
        bool msgParseSuperCmd(const CMD::SUPER::SuperServerNull *superNull,const DWORD nCmdLen);
    private:
        bool verified;			/**< 是否已经通过了服务器管理器的验证 */

};

/**
 * \brief 重载zThread中的纯虚函数，是线程的主回调函数，用于处理接收到的指令
 *
 */
void SuperClient::run()
{
    zTCPBufferClient::run();

    //与服务器管理器之间的连接断开，需要关闭服务器
    zSubNetService::subNetServiceInstance()->Terminate();
}


bool SuperClient::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    Fir::logger->debug("SuperClient::msgParseProto protobuf 消息没有处理");
    return true;
}


bool SuperClient::msgParseSuperCmd(const CMD::SUPER::SuperServerNull *superNull,const DWORD nCmdLen)
{   
    using namespace CMD::SUPER;
    switch(superNull->para)
    {
        case PARA_GAMETIME:
            {
                t_GameTime *ptCmd = (t_GameTime *)superNull;
                Fir::qwGameTime = ptCmd->qwGameTime;
                Fir::qwStartTime = ptCmd->qwStartTime;
                return zSubNetService::subNetServiceInstance()->msgParse_SuperService(superNull,nCmdLen);
            }
            break;
        case PARA_STARTUP_RESPONSE:
            {
                t_Startup_Response *ptCmd = (t_Startup_Response *)superNull;
                Fir::logger->debug("服务器管理器回应消息 %u %u %u %u", ptCmd->wdServerID, ptCmd->wdPort, ptCmd->wdExtPort, ptCmd->wdNetType);
                zSubNetService::subNetServiceInstance()->setServerInfo(ptCmd);
                return true;
            }
            break;
        case PARA_STARTUP_SERVERENTRY_NOTIFYME2:
        case PARA_STARTUP_SERVERENTRY_NOTIFYME:
            {
                t_Startup_ServerEntry_NotifyMe *ptCmd = (t_Startup_ServerEntry_NotifyMe *)superNull;
                Fir::logger->debug("返回服务器依赖列表");
                for(WORD i = 0; i < ptCmd->size; i++)
                {
                    Fir::logger->debug("%u, %u, %s, %s, %u, %s, %u, %u",
                            ptCmd->entry[i].wdServerID,
                            ptCmd->entry[i].wdServerType,
                            ptCmd->entry[i].pstrName,
                            ptCmd->entry[i].pstrIP,
                            ptCmd->entry[i].wdPort,
                            ptCmd->entry[i].pstrExtIP,
                            ptCmd->entry[i].wdExtPort,
                            ptCmd->entry[i].state);
                    //需要一个容器来管理这些服务器列表
                    zSubNetService::subNetServiceInstance()->addServerEntry(ptCmd->entry[i]);
                }
                if(superNull->para == PARA_STARTUP_SERVERENTRY_NOTIFYME)
                {
                    verified = true;
                }
                else
                {
                    zSubNetService::subNetServiceInstance()->getOtherseverinfo();
                }

                return true;
            }
            break;
        case PARA_STARTUP_SERVERENTRY_NOTIFYOTHER:
            {
                t_Startup_ServerEntry_NotifyOther *ptCmd = (t_Startup_ServerEntry_NotifyOther *)superNull;

                Fir::logger->debug("返回服务器依赖列表");
                Fir::logger->debug("%u, %u, %s, %s, %u, %s, %u, %u",
                        ptCmd->entry.wdServerID,
                        ptCmd->entry.wdServerType,
                        ptCmd->entry.pstrName,
                        ptCmd->entry.pstrIP,
                        ptCmd->entry.wdPort,
                        ptCmd->entry.pstrExtIP,
                        ptCmd->entry.wdExtPort,
                        ptCmd->entry.state);
                //需要一个容器来管理这些服务器列表
                zSubNetService::subNetServiceInstance()->addServerEntry(ptCmd->entry);

                std::string ret;
                encodeMessage(ptCmd,nCmdLen,ret);
                return sendCmd(ret.c_str(),ret.size()); 
            }
            break;
        case  PARA_STARTUP_SERVERENTRY_MeStart:
            {
                t_Startup_ServerEntry_MeStart *ptCmd = (t_Startup_ServerEntry_MeStart *)superNull;

                Fir::logger->debug("新服务启动通知");
                Fir::logger->debug("%u, %u, %s, %s, %u, %s, %u, %u",
                        ptCmd->entry.wdServerID,
                        ptCmd->entry.wdServerType,
                        ptCmd->entry.pstrName,
                        ptCmd->entry.pstrIP,
                        ptCmd->entry.wdPort,
                        ptCmd->entry.pstrExtIP,
                        ptCmd->entry.wdExtPort,
                        ptCmd->entry.state);
                zSubNetService::subNetServiceInstance()->addServerEntry(ptCmd->entry);
                zSubNetService::subNetServiceInstance()->getnewServerEntry(ptCmd->entry);
                return true;

            }
            break;
        default:
            {
                return zSubNetService::subNetServiceInstance()->msgParse_SuperService(superNull,nCmdLen);
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, superNull->cmd, superNull->para, nCmdLen);
    return false;
}

/**
 * \brief 解析来自服务器管理器的指令
 *
 * \param ptNullCmd 待处理的指令
 * \param nCmdLen 指令长度
 * \return 解析是否成功
 */

bool SuperClient::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }
    using namespace CMD::SUPER;
    using namespace CMD::FL;
    switch(ptNullCmd->cmd)
    {
        case SUPERCMD:
            {
                return msgParseSuperCmd((SuperServerNull*)ptNullCmd,nCmdLen);
            }
            break;
        default:
            {
                return zSubNetService::subNetServiceInstance()->msgParse_SuperService(ptNullCmd, nCmdLen);
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return true;
}

/**
 * \brief 构造函数
 * 
 * \param name 名称
 * \param wdType 服务器类型
 */
zSubNetService::zSubNetService(const std::string &name, const WORD wdType) : zNetService(name), superClient(NULL)
{
    subNetServiceInst = this;

    superPort = atoi(Fir::global["port"].c_str());
    bzero(superIP, sizeof(superIP));
    strncpy(superIP, Fir::global["server"].c_str(), sizeof(superIP) - 1);

    superClient = FIR_NEW SuperClient();

    wdServerID = 0;
    wdServerType = wdType;
    bzero(pstrName, sizeof(pstrName));
    bzero(pstrIP, sizeof(pstrIP));
    wdPort = 0;
    bzero(pstrExtIP, sizeof(pstrExtIP));
    wdExtPort = 0;
    bzero(pstrTable, sizeof(pstrTable));

}

/**
 * \brief 虚析构函数
 *
 */
zSubNetService::~zSubNetService()
{
    serverList.clear();

    SAFE_DELETE(superClient);

    subNetServiceInst = NULL;
}

/**
 * \brief 初始化网络服务器程序
 *
 * 实现纯虚函数<code>zService::init</code>
 * 建立到服务器管理器的连接，并得到服务器信息
 *
 * \return 是否成功
 */
bool zSubNetService::init()
{
    //Fir::logger->debug(__PRETTY_FUNCTION__);

    //建立到服务器管理器的连接
    if (!superClient->connect(superIP, superPort))
    {
        Fir::logger->error("连接服务器管理器失败");
        return false;
    }

    //发送登陆服务器管理器的指令
    using namespace CMD::SUPER;
    t_Startup_Request tCmd;
    tCmd.wdServerType = wdServerType;
    strncpy(tCmd.pstrIP, pstrIP, sizeof(tCmd.pstrIP));
    std::string ret;
    encodeMessage(&tCmd,sizeof(tCmd),ret);
     Fir::logger->error("发送验证指令");

    if (!superClient->sendCmd(ret.c_str(),ret.size())) 
    {
        Fir::logger->error("向服务器管理器发送登陆指令失败");
        return false;
    }

    //等待服务器管理器返回信息
    while(!superClient->verified)
    {
        BYTE acceptCmd[zSocket::MAX_DATASIZE];
        int nCmdLen = superClient->pSocket->recvToCmd(acceptCmd, sizeof(acceptCmd), true);
        if (-1 == nCmdLen)
        {
            Fir::logger->error("等待服务器管理器返回信息失败");
            return false;
        }
        else if (nCmdLen > 0)
        {
            superClient->msgParse(acceptCmd,nCmdLen);
        }
    }
    std::ostringstream so;
    so << Fir::logger->getName() << "[" <<wdServerID << "]";
    Fir::logger->setName(so.str());

    //zThread::sleep(1);

    //建立线程与服务器管理器交互
    superClient->start();

    //调用真实的初始化函数
    if (!zNetService::init(wdPort))
        return false;

    return true;
}

/**
 * \brief 确认服务器初始化成功，即将进入主回调函数
 *
 * 向服务器发送t_Startup_OK指令来确认服务器启动成功
 *
 * \return 确认是否成功
 */
bool zSubNetService::validate()
{
    CMD::SUPER::t_Startup_OK tCmd;
    tCmd.wdServerID = wdServerID;
    std::string ret;

    encodeMessage(&tCmd,sizeof(tCmd),ret);
    return superClient->sendCmd(ret.c_str(),ret.size());
}

/**
 * \brief 结束网络服务器
 *
 * 实现纯虚函数<code>zService::final</code>
 *
 */
void zSubNetService::final()
{
    zNetService::final();

    //Fir::logger->debug(__PRETTY_FUNCTION__);

    //关闭到服务器管理器的连接
    superClient->final();
    superClient->join();
    superClient->close();
}

/**
 * \brief 向服务器管理器发送指令
 *
 * \param pstrCmd 待发送的指令
 * \param nCmdLen 待发送指令的大小
 * \return 发送是否成功
 */
bool zSubNetService::sendCmdToSuperServer(const void *pstrCmd, const int nCmdLen)
{
    return superClient->sendCmd(pstrCmd, nCmdLen);
}

/**
 * \brief 根据服务器管理器返回信息，设置服务器的信息
 *
 * \param ptCmd 服务器管理器返回信息
 */
void zSubNetService::setServerInfo(const CMD::SUPER::t_Startup_Response *ptCmd)
{
    wdServerID = ptCmd->wdServerID;
    wdPort = ptCmd->wdPort;
    bcopy(ptCmd->pstrExtIP,pstrExtIP,MAX_IP_LENGTH);
    wdExtPort = ptCmd->wdExtPort;
    wdNetType = ptCmd->wdNetType;
    bcopy(ptCmd->pstrTable,pstrTable,MAX_TABLE_LIST);
}

/**
 * \brief 添加关联服务器信息到一个容器中
 *
 */
void zSubNetService::addServerEntry(const CMD::SUPER::ServerEntry &entry)
{
    mlock.lock();
    //首先查找有没有重复的
    std::deque<CMD::SUPER::ServerEntry>::iterator it;
    bool found = false;
    for(it = serverList.begin(); it != serverList.end(); ++it)
    {
        if (entry.wdServerID == it->wdServerID)
        {
            found = true;
            break;
        }
    }

    Fir::logger->debug("服务器信息：%u, %u", entry.wdServerID, entry.wdServerType);
    if (found)
    {
        //已经存在只是更新
        (*it) = entry;
    }
    else
    {
        //还不存在，需要新建立一个节点
        serverList.push_back(entry);
    }
    mlock.unlock();
}

/**
 * \brief 查找相关服务器信息
 *
 * \param wdServerID 服务器编号
 * \return 服务器信息
 */
const CMD::SUPER::ServerEntry *zSubNetService::getServerEntry(const WORD wdServerID)
{
    CMD::SUPER::ServerEntry *ret = NULL;
    std::deque<CMD::SUPER::ServerEntry>::iterator it;
    mlock.lock();
    for(it = serverList.begin(); it != serverList.end(); ++it)
    {
        if (wdServerID == it->wdServerID)
        {
            ret = &(*it);
            break;
        }
    }
    mlock.unlock();
    return ret;
}

/**
 * \brief 查找相关服务器信息
 *
 * \param wdServerType 服务器类型
 * \return 服务器信息
 */
const CMD::SUPER::ServerEntry *zSubNetService::getServerEntryByType(const WORD wdServerType)
{
    CMD::SUPER::ServerEntry *ret = NULL;
    std::deque<CMD::SUPER::ServerEntry>::iterator it;
    mlock.lock();
    for(it = serverList.begin(); it != serverList.end(); ++it)
    {
        Fir::logger->debug("服务器信息：%u, %u", wdServerType, it->wdServerType);
        if (wdServerType == it->wdServerType)
        {
            ret = &(*it);
            break;
        }
    }
    mlock.unlock();
    return ret;
}

/**
 * \brief 查找相关服务器信息
 *
 * \param wdServerType 服务器类型
 * \param prev 上一个服务器信息
 * \return 服务器信息
 */
const CMD::SUPER::ServerEntry *zSubNetService::getNextServerEntryByType(const WORD wdServerType, const CMD::SUPER::ServerEntry **prev)
{
    CMD::SUPER::ServerEntry *ret = NULL;
    bool found = false;
    std::deque<CMD::SUPER::ServerEntry>::iterator it;
    mlock.lock();
    for(it = serverList.begin(); it != serverList.end(); ++it)
    {
        //Fir::logger->debug("服务器信息：%u, %u", wdServerType, it->wdServerType);
        if (wdServerType == it->wdServerType)
        {
            if (NULL == prev
                    || found)
            {
                ret = &(*it);
                break;
            }
            else if (found == false
                    && (*prev)->wdServerID == it->wdServerID)
            {
                found = true;
            }
        }
    }
    mlock.unlock();
    return ret;
}


bool zSubNetService::hasDBtable(const std::string& tablename)
{

    static std::set<std::string> *selfTable = NULL;
    if(!selfTable)
    {
        std::vector<std::string> retVec;
        pb::parseTagString(pstrTable,",",retVec);
        selfTable = new std::set<std::string>(retVec.begin(),retVec.end());
        if(!selfTable)
        {
            return false;
        }

    }
    return selfTable->find(tablename) != selfTable->end();
}

bool zSubNetService::OtherServerhasDBtable(DWORD serverID,const std::string& tablename)
{
    static std::map<DWORD,std::set<std::string> > *OtherTable = NULL;
    if(!OtherTable)
    {
        OtherTable = new  std::map<DWORD,std::set<std::string> >;
        if(!OtherTable)
        {
            return false;
        }
        for(auto iter = serverList.begin(); iter != serverList.end();iter++)
        {
            CMD::SUPER::ServerEntry &rServerEntry = *(iter);
            std::vector<std::string> retVec;
            pb::parseTagString(rServerEntry.pstrTable,",",retVec);
            OtherTable->insert(std::make_pair(rServerEntry.wdServerID,std::set<std::string>(retVec.begin(),retVec.end())));

        }

    }
    auto it = OtherTable->find(serverID);
    if(it == OtherTable->end())
    {
        return false;
    }
    return it->second.find(tablename) != it->second.end();

}

