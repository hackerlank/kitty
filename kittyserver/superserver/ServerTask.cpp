/**
 * \file
 * \version  $Id: ServerTask.cpp 64 2013-04-23 02:05:08Z  $
 * \author  Songsiliang,
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 实现服务器连接类
 *
 * 
 */


#include <iostream>
#include <vector>
#include <list>
#include <iterator>
#include <unordered_map>

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "ServerTask.h"
#include "SuperCommand.h"
#include "ServerManager.h"
#include "Fir.h"
#include "zDBConnPool.h"
#include "SuperServer.h"
#include "zString.h"
#include "SuperServer.h"
#include "FLCommand.h"
#include "FLClient.h"
#include "FLClientManager.h"
#include "TimeTick.h"
#include "zServerInfo.h"
#include "extractProtoMsg.h"
#include "GmToolClientManager.h"
#include "ResourceCommand.h"
#include "ResourceClientManager.h"

/**
 * \brief 验证一个服务器连接是否合法
 *
 * 每一台服务器的信息在数据库中都有记录，如果数据库中没有相应的记录，那么这个服务器连接任务就是不合法的，需要立即断开连接<br>
 * 这样保证了一个区中的服务器之间的信任关系
 *
 * \param wdType 服务器类型
 * \param pstrIP 服务器地址
 * \return 验证是否成功
 */
bool ServerTask::verify(WORD wdType, const char *pstrIP)
{
    char where[64];

    this->wdServerType = wdType;
    strncpy(this->pstrIP, pstrIP, sizeof(this->pstrIP) - 1);
    Fir::logger->debug("%s \t%u, %s", __FUNCTION__, wdType, pstrIP);

    connHandleID handle = SuperService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return false;
    }

    bzero(where, sizeof(where));
    std::string escapeIP;
    snprintf(where, sizeof(where) - 1, "`TYPE`=%u AND `IP`='%s'", wdType, SuperService::dbConnPool->escapeString(handle,pstrIP,escapeIP).c_str());
    static const dbCol col_define[] =
    {
        {"ID",zDBConnPool::DB_WORD,sizeof(WORD)},
        {"NAME",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE])},
        {"PORT",zDBConnPool::DB_WORD,sizeof(WORD)},
        {"EXTIP",zDBConnPool::DB_STR,sizeof(char[MAX_IP_LENGTH])},
        {"EXTPORT",zDBConnPool::DB_WORD,sizeof(WORD)},
        {"NETTYPE",zDBConnPool::DB_WORD,sizeof(WORD)},
        {"DBTABLE",zDBConnPool::DB_STR,sizeof(char[MAX_TABLE_LIST])},
        {NULL, 0, 0}
    };
    struct
    {
        WORD wdServerID;
        char pstrName[MAX_NAMESIZE];
        WORD wdPort;
        char pstrExtIP[MAX_IP_LENGTH];
        WORD wdExtPort;
        WORD wdNetType;
        char pstrTable[MAX_TABLE_LIST];
    } __attribute__ ((packed))
    *pData = NULL;

    unsigned int retcode = SuperService::dbConnPool->exeSelect(handle, "`t_serverlist`", col_define, where, "`ID`",(unsigned char **)&pData);
    if (retcode == (unsigned int)-1 || retcode == 0 || NULL == pData)
    {
        Fir::logger->error("数据库中没有相应的服务器记录");
        SuperService::dbConnPool->putHandle(handle);
        return false;
    }

    SuperService::dbConnPool->putHandle(handle);

    //某些类型服务器在一个区中只能由一台
    if (retcode > 1 && (wdType == BILLSERVER) )
    {
        SAFE_DELETE_VEC(pData);
        Fir::logger->error("这种类型的服务器只能有一台 %u", wdType);
        return false;
    }

    //从数据库中取数据成功，需要从这些数据中取得一个可用项
    unsigned int i;
    for(i = 0; i < retcode; i++)
    {
        if (ServerManager::getMe().uniqueVerify(pData[i].wdServerID))
        {
            wdServerID = pData[i].wdServerID;
            strncpy(pstrName, pData[i].pstrName, sizeof(pstrName) - 1);
            wdPort = pData[i].wdPort;
            strncpy(pstrExtIP, pData[i].pstrExtIP, sizeof(pstrExtIP) - 1);
            wdExtPort = pData[i].wdExtPort;
            wdNetType = pData[i].wdNetType;
            strncpy(pstrTable, pData[i].pstrTable, sizeof(pstrTable) - 1);
            break;
        }
    }
    SAFE_DELETE_VEC(pData);
    if (i == retcode)
    {
        Fir::logger->error("服务器已经启动完成了，没有可用记录(%u)",wdType);
        return false;
    }

    //返回服务器信息到服务器

    using namespace CMD::SUPER;
    t_Startup_Response tCmd;
    tCmd.wdServerID = wdServerID;
    tCmd.wdPort = wdPort;
    bcopy(pstrExtIP, tCmd.pstrExtIP, sizeof(pstrExtIP));
    tCmd.wdExtPort = wdExtPort;
    tCmd.wdNetType = wdNetType;
    bcopy(pstrTable, tCmd.pstrTable, sizeof(pstrTable));

    std::string ret;
    encodeMessage(&tCmd,sizeof(tCmd),ret);
    if (!sendCmd(ret.c_str(),ret.size()))
    {
        Fir::logger->error("向服务器发送指令失败");
        return false;
    }

    return true;
}

/**
 * \brief 等待接受验证指令并进行验证
 *
 * 实现虚函数<code>zTCPTask::verifyConn</code>
 *
 * \return 验证是否成功，或者超时
 */
int ServerTask::verifyConn()
{
    int retcode = mSocket.recvToBuf_NoPoll();
    if (retcode > 0)
    {
        BYTE acceptCmd[zSocket::MAX_DATASIZE];
        int nCmdLen = mSocket.recvToCmd_NoPoll(acceptCmd, sizeof(acceptCmd));
        //这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
        if (nCmdLen <= 0)
        {
            return 0;
        }

        using namespace CMD::SUPER;
        BYTE messageType = *(BYTE*)acceptCmd;
        nCmdLen -= sizeof(BYTE);
        if(messageType != STRUCT_TYPE || nCmdLen <= 0)
        {
            Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__, messageType,nCmdLen-1);
            return -1;
        }

        BYTE *pstrCmd = acceptCmd + sizeof(BYTE);
        t_Startup_Request *ptCmd = (t_Startup_Request *)pstrCmd;
        if (SUPERCMD == ptCmd->cmd && PARA_STARTUP_REQUEST == ptCmd->para)
        {
            if (verify(ptCmd->wdServerType, ptCmd->pstrIP))
            {
                Fir::logger->debug("客户端连接通过验证");
                return 1;
            }
        }
        Fir::logger->error("客户端连接验证失败"); 
        return -1;
    }
    else
        return retcode;
}

/**
 * \brief 保存服务器之间的依赖关系
 *
 */
//static std::unordered_map<int, std::vector<int> > serverSequence;
//
///**
// * \brief 初始化服务器之间的依赖关系
// *
// */
//static void initServerSequence() __attribute__ ((constructor));
//void initServerSequence()
//{
//	serverSequence[UNKNOWNSERVER]	=	std::vector<int>();
//	serverSequence[SUPERSERVER]	=	std::vector<int>();
//	serverSequence[LOGINSERVER]	=	std::vector<int>();
//	serverSequence[RECORDSERVER]	=	std::vector<int>();
//	serverSequence[GATEWAYSERVER] = std::vector<int>(); //测试用
//
//	int data0[] = { RECORDSERVER};
//	//会话依赖档案服务器
//	serverSequence[SESSIONSERVER]   =   std::vector<int>(data0, data0 + sizeof(data0) / sizeof(int));
//	std::cout<<"初始化serverSequence[SESSIONSERVER]:"<<serverSequence[SESSIONSERVER].size()<<std::endl;
//	int data1[] = { RECORDSERVER, SESSIONSERVER};
//	//场景服务器依赖档案服务器，会话服务器
//	serverSequence[SCENESSERVER]	=	std::vector<int>(data1, data1 + sizeof(data1) / sizeof(int));
//	int data2[] = { RECORDSERVER, SESSIONSERVER, SCENESSERVER};
//	//网关依赖档案服务器，会话服务器，场景服务器
//	serverSequence[GATEWAYSERVER]	=	std::vector<int>(data2, data2 + sizeof(data2) / sizeof(int));
//	std::cout<<"初始化serverSequence[SESSIONSERVER]:"<<serverSequence[SESSIONSERVER].size()<<std::endl;
//}

/**
 * \brief 验证某种类型的所有服务器是否完全启动完成
 *
 * \param wdType 服务器类型
 * \param sv 容器，容纳启动成功的服务器列表
 * \return 验证是否成功
 */
bool ServerTask::verifyTypeOK(const WORD wdType, std::vector<ServerTask *> &sv)
{
    static const dbCol col_define[] =
    {
        {"ID",zDBConnPool::DB_WORD,sizeof(WORD)},
        {NULL, 0, 0}
    };
    char where[64];
    WORD *ID = NULL;

    if (0 == wdType)
        return true;

    bzero(where, sizeof(where));
    snprintf(where, sizeof(where) - 1, "`TYPE`=%u", wdType);

    connHandleID handle = SuperService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return false;
    }

    unsigned int retcode = SuperService::dbConnPool->exeSelect(handle, "`t_serverlist`", col_define, where, "`ID`",(unsigned char **)&ID);
    if (retcode == (unsigned int)-1
            || retcode == 0
            || NULL == ID)
    {
        Fir::logger->error("数据库中没有相应的服务器记录");
        SuperService::dbConnPool->putHandle(handle);
        return false;
    }
    SuperService::dbConnPool->putHandle(handle);

    bool retval = true;
    for(unsigned int i = 0; i < retcode; i++)
    {
        //这个容器里面的肯定是OK状态
        ServerTask * pServer = ServerManager::getMe().getServer(ID[i]);
        if (NULL == pServer)
        {
            Fir::logger->info("%u need %u ok",wdServerID,ID[i]);
            retval = false;
            break;
        }
        else
        {
            Fir::logger->info("%u get %u ok",wdServerID,ID[i]);
            sv.push_back(pServer);
        }
    }
    SAFE_DELETE_VEC(ID);

    return retval;
}

/**
 * \brief 通知所有依赖的服务器
 * \return 通知是否成功
 */
bool ServerTask::notifyOther(WORD dstID)
{
    using namespace CMD::SUPER;
    t_Startup_ServerEntry_NotifyOther cmd;
    bzero(&cmd.entry, sizeof(cmd.entry));
    cmd.entry.wdServerID = wdServerID;
    cmd.entry.wdServerType = wdServerType;
    strncpy(cmd.entry.pstrName, pstrName,sizeof(cmd.entry.pstrName));
    strncpy(cmd.entry.pstrIP, pstrIP, sizeof(cmd.entry.pstrIP));
    cmd.entry.wdPort = wdPort;
    strncpy(cmd.entry.pstrExtIP, pstrExtIP, sizeof(cmd.entry.pstrExtIP));
    cmd.entry.wdExtPort = wdExtPort;
    cmd.entry.state = state;
    strncpy(cmd.entry.pstrTable, pstrTable, sizeof(cmd.entry.pstrTable));

    std::string ret;
    encodeMessage(&cmd,sizeof(cmd),ret);
    for(Container::iterator it = ses.begin(); it != ses.end(); ++it)
    {
        if(dstID == it->first.wdServerID)
        {
            ServerTask * pDst = ServerManager::getMe().getServer(dstID);
            if(pDst)
            {
                pDst->sendCmd(ret.c_str(),ret.size());
            }
            break;
        }
    }

    return true;
}
/**
 * \brief 通知所有依赖的服务器
 * \return 通知是否成功
 */
bool ServerTask::notifyOther()
{
    using namespace CMD::SUPER;
    bool retval = true;
    t_Startup_ServerEntry_NotifyOther cmd;
    bzero(&cmd.entry, sizeof(cmd.entry));
    cmd.entry.wdServerID = wdServerID;
    cmd.entry.wdServerType = wdServerType;
    strncpy(cmd.entry.pstrName, pstrName, sizeof(cmd.entry.pstrName));
    strncpy(cmd.entry.pstrIP, pstrIP, sizeof(cmd.entry.pstrIP));
    cmd.entry.wdPort = wdPort;
    strncpy(cmd.entry.pstrExtIP, pstrExtIP, sizeof(cmd.entry.pstrExtIP));
    cmd.entry.wdExtPort = wdExtPort;
    cmd.entry.state = state;
    strncpy(cmd.entry.pstrTable, pstrTable, sizeof(cmd.entry.pstrTable));

    for(Container::iterator it = ses.begin(); it != ses.end(); ++it)
    {
        cmd.srcID = it->first.wdServerID;
        std::string ret;
        encodeMessage(&cmd,sizeof(cmd),ret);
        retval &= ServerManager::getMe().broadcastByID(cmd.srcID,ret.c_str(),ret.size());
    }


    /*//Fir::logger->debug("%u", wdServerType);
      std::vector<int> sequence = serverSequence[wdServerType];
      for(std::vector<int>::const_iterator it = sequence.begin(); it != sequence.end(); it++)
      {
    //广播指令
    //Fir::logger->debug("%u", (*it));
    Cmd.srcID = wdServerID;
    retval &= ServerManager::getMe().broadcastByType(*it, &Cmd, sizeof(Cmd));
    }*/

    return retval;
}

/**
 * \brief 收到notifyOther回复
 * \param wdServerID 目的服务器编号
 */
void ServerTask::responseOther(const WORD reswdServerID)
{
    for(Container::iterator it = ses.begin(); it != ses.end(); ++it)
    {
        if (it->first.wdServerID == reswdServerID)
        {
            Fir::logger->info("%u get %u respon",wdServerID ,reswdServerID);
            it->second = true;

        }
    }
}

/**
 * \brief 通知服务器其依赖的服务器信息列表
 * \return 通知是否成功
 */
bool ServerTask::notifyMe()
{
    if (hasNotifyMe) return true;
    
    using namespace CMD::SUPER;
    BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
    t_Startup_ServerEntry_NotifyMe *ptCmd = (t_Startup_ServerEntry_NotifyMe *)(pBuffer);
    constructInPlace(ptCmd);
    ptCmd->size = 0;

    //check for notify other response
    for(Container::iterator it = ses.begin(); it != ses.end(); ++it)
    {
        if (it->second)
        {
            bzero(&ptCmd->entry[ptCmd->size], sizeof(ptCmd->entry[ptCmd->size]));
            ptCmd->entry[ptCmd->size].wdServerID = it->first.wdServerID;
            ptCmd->entry[ptCmd->size].wdServerType = it->first.wdServerType;
            strncpy(ptCmd->entry[ptCmd->size].pstrName, it->first.pstrName, MAX_NAMESIZE - 1);
            strncpy(ptCmd->entry[ptCmd->size].pstrIP, it->first.pstrIP, MAX_IP_LENGTH - 1);
            ptCmd->entry[ptCmd->size].wdPort = it->first.wdPort;
            strncpy(ptCmd->entry[ptCmd->size].pstrExtIP, it->first.pstrExtIP, MAX_IP_LENGTH - 1);
            ptCmd->entry[ptCmd->size].wdExtPort = it->first.wdExtPort;
            ptCmd->entry[ptCmd->size].state = it->first.state;
            strncpy(ptCmd->entry[ptCmd->size].pstrTable, it->first.pstrTable, MAX_TABLE_LIST - 1);

            ptCmd->size++;
        }
        else
        {
            Fir::logger->info("%u  wait %u respond",wdServerID,it->first.wdServerID);
            return false;
        }
    }
    
    std::string ret;
    encodeMessage(ptCmd,sizeof(t_Startup_ServerEntry_NotifyMe) + ptCmd->size * sizeof(ServerEntry),ret);
    if (sendCmd(ret.c_str(),ret.size()))
    {
        hasNotifyMe = true;
    }

    Fir::logger->info("%u servercaninit",wdServerID);
    return hasNotifyMe;
}

/**
 * \brief 处理服务器的依赖关系，也就是启动顺序
 * \return 是否所有所依赖的服务器已经启动完成
 */
bool ServerTask::processSequence()
{
    using namespace CMD::SUPER;

    ses.clear();

    Fir::logger->debug("%s,servertype=%u", __PRETTY_FUNCTION__, wdServerType);

    std::vector<int> sequence = SuperService::getMe().getServerSes(wdServerType);
    for(std::vector<int>::const_iterator it = sequence.begin(); it != sequence.end(); it++)
    {
        std::vector<ServerTask *> sv;
        if (verifyTypeOK(*it, sv))
        {
            for(std::vector<ServerTask *>::const_iterator sv_it = sv.begin(); sv_it != sv.end(); sv_it++)
            {
                ServerEntry se;
                bzero(&se, sizeof(se));
                se.wdServerID = (*sv_it)->wdServerID;
                se.wdServerType = (*sv_it)->wdServerType;
                strncpy(se.pstrName, (*sv_it)->pstrName, MAX_NAMESIZE - 1);
                strncpy(se.pstrIP, (*sv_it)->pstrIP, MAX_IP_LENGTH - 1);
                se.wdPort = (*sv_it)->wdPort;
                strncpy(se.pstrExtIP, (*sv_it)->pstrExtIP, MAX_IP_LENGTH - 1);
                se.wdExtPort = (*sv_it)->wdExtPort;
                se.state = (*sv_it)->state;
                strncpy(se.pstrTable, (*sv_it)->pstrTable, MAX_TABLE_LIST - 1);
                ses.insert(Container::value_type(se, false));
            }
        }
        else
        {
            Fir::logger->info("%s,false,serverid=%u,err type %d", __PRETTY_FUNCTION__,wdServerID,*it);
            return false;
        }
    }

    Fir::logger->info("%s,true,serverid=%u", __PRETTY_FUNCTION__, wdServerID);
    return true;
}

/**
 * \brief 等待服务器启动完成
 *
 * 服务器接受到服务器管理器返回的自身信息，会进行自身的初始化，初始化完毕发送启动完成指令到服务器管理器<br>
 * 当服务器管理器接受到确认启动完成的指令就会把这个服务器加入到完成启动的队列中，这样服务器就完成了整个启动过程<br>
 * 实现了虚函数<code>zTCPTask::waitSync</code>
 *
 * \return 服务器启动是否通过，如果超时还要继续等待
 */
int ServerTask::waitSync()
{
    int retcode = mSocket.checkIOForRead();
    if (-1 == retcode)
    {
        Fir::logger->error("%s", __PRETTY_FUNCTION__);
        return -1;
    }
    else if (retcode > 0)
    {
        //套接口准备好了接收数据，接收数据到缓冲，并尝试处理指令
        retcode = mSocket.recvToBuf_NoPoll();
        if (-1 == retcode)
        {
            Fir::logger->error("%s", __PRETTY_FUNCTION__);
            return -1;
        }
    }

    BYTE acceptCmd[zSocket::MAX_DATASIZE];
    int nCmdLen = mSocket.recvToCmd_NoPoll(acceptCmd, sizeof(acceptCmd));
    if (nCmdLen > 0)
    {
        BYTE messageType = *(BYTE*)acceptCmd;
        nCmdLen -= sizeof(BYTE);
        if(nCmdLen > 0)
        {
            BYTE *pstrCmd = acceptCmd + sizeof(BYTE);
            if(messageType != STRUCT_TYPE)
            {
                Fir::logger->debug("%u, %u", wdServerID,nCmdLen);
                return -1;
            }
            using namespace CMD::SUPER;
            t_Startup_OK *ptCmd = (t_Startup_OK *)pstrCmd;
            if (((SUPERCMD == ptCmd->cmd && PARA_STARTUP_OK == ptCmd->para) && (wdServerID == ptCmd->wdServerID)))
            {
                Fir::logger->debug("客户端连接同步验证成功");
                Fir::logger->info("%u serverstart ok",wdServerID);
                //通知别人来连我
                return 1;
            }
            Fir::logger->error("%u,客户端连接同步验证失败(%u,%u,%u )",wdServerID,ptCmd->cmd,ptCmd->para,ptCmd->wdServerID);
            return 0;
        }
    }

    //首先检查处理启动顺序
    if (checkSequenceTime() && processSequence() && notifyOther())
    {
        sequenceOK = true;
    }
    if (sequenceOK)
    {
        notifyMe();

    }

    //等待超时
    return 0;
}
bool ServerTask::checkSequenceTime()
{
    //启动顺序处理已经完成了，不需要再次处理
    if (sequenceOK)
        return false;

    //检测两次处理的间隔时间
    zTime currentTime;
    if (lastSequenceTime.elapse(currentTime) > 2)
    {
        Fir::logger->info("%u  checkSequenceTime",wdServerID);
        lastSequenceTime = currentTime;
        return true;
    }
    Fir::logger->info("%u wait next checkSequenceTime",wdServerID);
    return false;
}
/**
 * \brief 确认一个服务器连接的状态是可以回收的
 *
 * 当一个连接状态是可以回收的状态，那么意味着这个连接的整个生命周期结束，可以从内存中安全的删除了：）<br>
 * 实现了虚函数<code>zTCPTask::recycleConn</code>
 *
 * \return 是否可以回收
 */
int ServerTask::recycleConn()
{
    return 1;
}

/**
 * \brief 添加到全局容器中
 *
 * 实现了虚函数<code>zTCPTask::addToContainer</code>
 *
 */
void ServerTask::addToContainer()
{

    ServerManager::getMe().addServer(this);
    CMD::SUPER::t_ZoneID send;
    send.zone = SuperService::getMe().getZoneID();
    strncpy(send.name,SuperService::getMe().getZoneName().c_str(),sizeof(send.name));
    std::string ret;
    encodeMessage(&send,sizeof(send),ret);
    sendCmd(ret.c_str(),ret.size());
    //把当前的其他服务器发给我
    //1，允许 别人连接我
    //2，我 去连接别人
    //无依赖
    if (this->wdServerType == SCENESSERVER || this->wdServerType == RECORDSERVER)
    {
        std::set<WORD> setType;
        setType.insert(GATEWAYSERVER);
        setType.insert(SCENESSERVER);
        ServerManager::getMe().sendOtherserverToMe(this,setType);
    }
    //我可以被连接了
    ServerManager::getMe().sendOtherMeStart(this);
}

/**
 * \brief 从全局容器中删除
 *
 * 实现了虚函数<code>zTCPTask::removeToContainer</code>
 *
 */
void ServerTask::removeFromContainer()
{
    //如果是网关服务器关闭，首先通知所有的登陆服务器网关关闭
    if (GATEWAYSERVER == wdServerType)
    {
        CMD::FL::t_GYList_FL tCmd;
        tCmd.wdServerID = wdServerID;
        bzero(tCmd.pstrIP, sizeof(tCmd.pstrIP));
        tCmd.wdPort = 0;
        tCmd.wdNumOnline = 0;
        tCmd.state = state_maintain;
        tCmd.zoneGameVersion = 0;
        Fir::logger->trace("GatewayServer停机 发送维护状态到FLServer!");

        std::string ret;
        encodeMessage(&tCmd,sizeof(tCmd),ret);
        FLClientManager::getMe().broadcast(ret.c_str(),ret.size());
    }
    if (SCENESSERVER == wdServerType)//通知其他场景服务器，我已经关闭
    {
        ServerManager::getMe().sendOtherSceneClose(this);
    }

    ServerManager::getMe().removeServer(this);
}

/**
 * \brief 添加到唯一性验证容器中
 *
 * 实现了虚函数<code>zTCPTask::uniqueAdd</code>
 *
 */
bool ServerTask::uniqueAdd()
{
    return ServerManager::getMe().uniqueAdd(this);
}

/**
 * \brief 从唯一性验证容器中删除
 *
 * 实现了虚函数<code>zTCPTask::uniqueRemove</code>
 *
 */
bool ServerTask::uniqueRemove()
{
    return ServerManager::getMe().uniqueRemove(this);
}


/*
 *\brief 判断是不是死锁了
 */
void ServerTask::checkDeadLock(DWORD now)
{
    if(tickFlag)
    {
        if(m_tickTime && now > m_tickTime + 120)
        {
            tickFlag = false;
            m_tickTime = now + 10 * 60;
        }
    }
    else if(now > m_tickTime + HEARTBEAT_TICK)
    {
        tickFlag = true;
        m_tickTime = now;
    }
}


/**
 * \brief 解析来自服务器管理器的关于启动的指令
 *
 * \param ptNullCmd 待处理的指令
 * \param nCmdLen 指令长度
 * \return 解析是否成功
 */
bool ServerTask::msgParseSuperCmd(const CMD::SUPER::SuperServerNull *superNull,const DWORD nCmdLen)
{
    using namespace CMD::SUPER;
    switch(superNull->para)
    {
        case PARA_STARTUP_SERVERENTRY_NOTIFYOTHER:
            {
                t_Startup_ServerEntry_NotifyOther *ptCmd = (t_Startup_ServerEntry_NotifyOther *)superNull;
                ServerManager::getMe().responseOther(ptCmd->entry.wdServerID, ptCmd->srcID);
                return true;
            }
            break;
            /*
        case PARA_RESTART_SERVERENTRY_NOTIFYOTHER:
            {
                t_restart_ServerEntry_NotifyOther *notify = (t_restart_ServerEntry_NotifyOther*)superNull;
                ServerTask * pSrc = ServerManager::getMe().getServer(notify->srcID);
                if(pSrc)
                {
                    pSrc->notifyOther(notify->dstID);
                }
                return true;
            }
            */
        case PARA_CHANGE_GAMETIME:
            {
                t_ChangeGameTime *changeTime = (t_ChangeGameTime*)superNull;
                try
                {
                    zRTime temp = zRTime(changeTime->time);
                    if(temp.sec() < SuperTimeTick::currentTime.sec())
                    {
                        return true;
                    }

                    unsigned long long grap = temp.sec() - SuperTimeTick::currentTime.sec();
                    SuperTimeTick::currentTime.setgrap(grap * 1000);

                    CMD::SUPER::t_GameTime tCmd;
                    tCmd.qwGameTime = temp.sec();
                    tCmd.qwStartTime = temp.sec();

                    std::string ret;
                    encodeMessage(&tCmd,sizeof(tCmd),ret);
                    ServerManager::getMe().broadcast(ret.c_str(),ret.size());

                    SuperTimeTick::currentTime.now();
                    Fir::logger->debug("[修改时间]:%s,%s,%llu",SuperTimeTick::currentTime.toString().c_str(),temp.toString().c_str(),grap);
                    return true;
                }
                catch(...)
                {
                    Fir::logger->debug("t_ChangeGameTime 格式不对%s",changeTime->time);
                    return true;
                }
            }
            break;
        case PARA_RELOAD_CONFIG:
            {
                t_ReloadConfig *reloadConf = (t_ReloadConfig*)superNull;
                Fir::logger->debug("[广播重新加载配置文件]");
                std::string ret;
                encodeMessage(reloadConf,sizeof(*reloadConf),ret);
                ServerManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Change_HeartTime:
            {
                t_ChangeHeartTime *changeTime = (t_ChangeHeartTime*)superNull;
                std::string ret;
                encodeMessage(changeTime,sizeof(t_ChangeHeartTime),ret);
                ServerManager::getMe().broadcastByType(GATEWAYSERVER,ret.c_str(),ret.size());
                Fir::logger->debug("[修改心跳包]:%u",changeTime->heartTime);
                return true;
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, superNull->cmd, superNull->para, nCmdLen);
    return false;
}

bool ServerTask::msgParseFlCmd(const CMD::FL::FLNullCmd *flNull,const DWORD nCmdLen)
{
    using namespace CMD::FL;
    switch(flNull->para)
    {
        case PARA_FL_GYLIST:
            {   
                t_GYList_FL *ptCmd = (t_GYList_FL*)flNull;
                OnlineNum = ptCmd->wdNumOnline;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GYList_FL),ret);
                FLClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, flNull->cmd, flNull->para, nCmdLen);
    return false;
}

bool ServerTask::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    Fir::logger->error("ServerTask::msgParseProto 消息处理");
    return true;
}

/**
 * \brief 解析来自各个服务器连接的指令
 *
 * \param ptNullCmd 待处理的指令
 * \param nCmdLen 指令长度
 * \return 处理是否成功
 */
bool ServerTask::msgParse(const BYTE *message, const DWORD nCmdLen)
{
    return MessageQueue::msgPush(message,nCmdLen);
}

bool ServerTask::cmdMsgParse(const BYTE *message, const DWORD nCmdLen)
{
    return zProcessor::msgParse(message,nCmdLen);
}

bool ServerTask::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }

    using namespace CMD::SUPER;
    using namespace CMD::FL;
    using namespace CMD::GMTool;
    using namespace CMD::RES;
    switch(ptNullCmd->cmd)
    {
        case SUPERCMD:
            {
                return msgParseSuperCmd((SuperServerNull*)ptNullCmd,nCmdLen);
            }
            break;
        case FLCMD:
            {
                return msgParseFlCmd((FLNullCmd*)ptNullCmd,nCmdLen);
            }
            break;
        case GMTOOLCMD:
            {
                return msgParseGmToolCmd((GmToolNullCmd*)ptNullCmd,nCmdLen);
            }
        case RESCMD:
            {
                return msgParseResourceCmd((ResNullCmd*)ptNullCmd,nCmdLen);
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return false;
}

bool ServerTask::msgParseResourceCmd(const CMD::RES::ResNullCmd *resNull,const DWORD nCmdLen)
{
    using namespace CMD::RES;
    switch(resNull->para)
    {
        case PARA_ADD_RES:
            {
                const t_AddRes *addRes = (const t_AddRes*)resNull;

                std::string ret;
                encodeMessage(addRes,sizeof(CMD::RES::t_AddRes),ret);
                ResourceClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, resNull->cmd, resNull->para, nCmdLen);
    return false;
}


bool ServerTask::msgParseGmToolCmd(const CMD::GMTool::GmToolNullCmd *gmToolNull,const DWORD nCmdLen)
{
    using namespace CMD::GMTool;
    switch(gmToolNull->para)
    {
        case PARA_Modify_Attr:
            {
                t_GmToolModifyAttr *ptCmd = (t_GmToolModifyAttr*)gmToolNull;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolModifyAttr) + ptCmd->size * sizeof(ModifyAttr),ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Modify_Build:
            {
                t_GmToolModifyBuild *ptCmd = (t_GmToolModifyBuild*)gmToolNull;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolModifyBuild) + ptCmd->size * sizeof(ModifyAttr),ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Forbid_Op:
            {
                t_GmToolForbidOp *ptCmd = (t_GmToolForbidOp*)gmToolNull;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolForbidOp),ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Email_Op:
            {
                t_GmToolEmailOp *ptCmd = (t_GmToolEmailOp*)gmToolNull;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolEmailOp) + ptCmd->size,ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Notice_Op:
            {
                t_GmToolNoticeOp *ptCmd = (t_GmToolNoticeOp*)gmToolNull;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolNoticeOp) + ptCmd->size * sizeof(GmToolNoticeData),ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
        case PARA_Cash_Delivery:
            {
                t_GmToolCashDelivery *ptCmd = (t_GmToolCashDelivery*)gmToolNull;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolCashDelivery) + ptCmd->size * sizeof(DeliveryInfo),ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Gift_Store:
            {
                t_GmToolGiftStore *rev = (t_GmToolGiftStore*)gmToolNull;

                std::string ret;
                encodeMessage(rev,sizeof(t_GmToolGiftStore) + rev->size * sizeof(GiftStoreInfo),ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Operator_Common:
            {
                t_Operator_Common *rev = (t_Operator_Common*)gmToolNull;
                std::string ret;
                encodeMessage(rev,sizeof(t_Operator_Common) + rev->size,ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Del_Picture:
            {
                t_GmToolDelPicture *ptCmd = (t_GmToolDelPicture*)gmToolNull;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolDelPicture) + ptCmd->size * sizeof(DelPicture),ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_GLOBAL_EMAIL:
            {
                t_GmToolGlobalEmail *ptCmd = (t_GmToolGlobalEmail*)gmToolNull;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolGlobalEmail) + ptCmd->size * sizeof(Key32Val32Pair),ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_COMMON:
            {
                t_GmToolCommon *ptCmd = (t_GmToolCommon*)gmToolNull;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolCommon) + ptCmd->size,ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Modify_Verify:
            {
                t_GmToolModifyVerify *ptCmd = (t_GmToolModifyVerify*)gmToolNull;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolModifyVerify) + ptCmd->size * sizeof(Key32Val32Pair),ret);
                GmToolClientManager::getMe().broadcast(ret.c_str(),ret.size());
                return true;
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, gmToolNull->cmd, gmToolNull->para, nCmdLen);
    return false;
}


