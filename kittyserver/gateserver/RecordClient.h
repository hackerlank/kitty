/**
 * \file
 * \version  $Id: RecordClient.h 24 2013-03-30 08:04:25Z  $
 * \author  ,@163.com
 * \date 2005年04月01日 11时56分04秒 CST
 * \brief 定义网关服务器到档案服务器连接客户端
 */

#ifndef _RECORDCLIENT_H_
#define _RECORDCLIENT_H_

#include "zTCPClient.h"
#include "zMutex.h"
#include "RecordCommand.h"
#include "zTCPClientTask.h"
#include "SuperCommand.h"
#include "zTCPClientTaskPool.h"
/**
 * \brief 网关与档案服务器的连接
 *
 */
class RecordClient : public zTCPClientTask
{

	public:
		
		/**
		 * \brief 构造函数
		 *
		 * \param name 名称
		 * \param ip 服务器地址
		 * \param port 服务器端口
		 */
		RecordClient(
				const std::string &ip, 
				const unsigned short port,WORD ServerID)
			: zTCPClientTask(ip, port),wdServerID(ServerID) {};
        virtual ~RecordClient()
        {
        }
        void addToContainer();
        void removeFromContainer(); 
		bool connectToRecordServer();
        bool connect();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
		bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
        QWORD playerNum();
        const WORD getServerID() const
		{
			return wdServerID;
		}
    public:
        //同步活动信息
        //bool broadcastActive(const CMD::RECORD::t_ActiveInfo *active);

    private:
        //处理recordCmd 消息
        bool msgParseRecordCmd(const CMD::RECORD::RecordNull *recordNull,const DWORD nCmdLen);
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

#endif
