/**
 * \file
 * \version  $Id: RecordClient.h 37 2013-04-08 01:52:56Z  $
 * \author  ,
 * \date 2013年04月05日 13时58分55秒 CST
 * \brief 定义档案服务器连接客户端
 *
 * 负责与档案服务器交互，存取档案
 * 
 */

#ifndef _RecordClient_h_
#define _RecordClient_h_

#include <unistd.h>
#include <iostream>

#include "zMisc.h"
#include "zMutex.h"
#include "RecordCommand.h"
#include "zTCPClientTask.h"
#include "SuperCommand.h"
#include "zTCPClientTaskPool.h"
#include "MessageQueue.h"

/**
 * brief 定义 档案服务器连接客户端类
 *
 * 负责 与档案服务器交互，存取档案
 * TODO 暂时只有一个档案服务器
 * 
 */
class RecordClient : public zTCPClientTask ,public MessageQueue
{

    public:

        /**
         * \brief 构造函数
         * 由于档案数据已经是压缩过的，故在底层传输的时候就不需要压缩了
         * \param name 名称
         * \param ip 地址
         * \param port 端口
         */
        RecordClient(
                const std::string &ip, 
                const unsigned short port,WORD ServerID)
            : zTCPClientTask(ip, port),wdServerID(ServerID) {};

        void addToContainer();
        void removeFromContainer(); 
        bool connectToRecordServer();
        bool connect();
        //消息处理函数总接口
        virtual bool msgParse(const BYTE *message, const DWORD nCmdLen);
        virtual bool cmdMsgParse(const BYTE *message, const DWORD cmdLen);
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
        const WORD getServerID() const
        {
            return wdServerID;
        }
    private:
        //处理record消息
        bool msgParseRecordCmd(const CMD::RECORD::RecordNull *ptNullCmd, const DWORD nCmdLen);
        //处理auction提交消息
        bool commitAuction(const DWORD auctionID);
        WORD wdServerID;


};

class ManagerRecordClient
{
    public:
        bool init();
        void timeAction(const zTime &ct);
        void add(RecordClient *recordclient);
        void remove(RecordClient *recordclient);
        bool reConnectrecord(const CMD::SUPER::ServerEntry *serverEntry);
        void setTaskReconnect(const std::string& ip, unsigned short port, bool reconn);
        RecordClient* GetRecordByServerId(DWORD wServerId);
        RecordClient* GetMinPlayerRecord();
        void final();
        void execEvery();

        RecordClient* GetRecordByTableName(const std::string &strTableName);
    private:
        /**
         ** \brief 客户端连接管理池
         **/
        zTCPClientTaskPool *recordclientPool;
        /**
         ** \brief 进行断线重连检测的时间记录
         **/
        zTime actionTimer;
        /**
         ** \brief 容器访问读写锁
         **/
        zRWLock rwlock;


    private:
        std::map<DWORD,RecordClient*> m_mapRecord; 

};

extern ManagerRecordClient MgrrecordClient;

//extern RecordClient *recordClient;

#endif

