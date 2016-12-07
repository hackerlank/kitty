/**
 * \file
 * \version  $Id: ServerTask.h 35 2013-04-07 07:18:55Z  $
 * \author  Songsiliang,
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 定义服务器连接任务
 *
 * 一个区中的每一个服务器都需要和服务器管理器建立连接
 * 
 */

#ifndef _ServerTask_h_
#define _ServerTask_h_

#include <unordered_map>

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "zMisc.h"
#include "zTime.h"
#include "SuperCommand.h"
#include "MessageQueue.h"
#include "FLCommand.h"
#include "GmToolCommand.h"
#include "ResourceCommand.h"

/**
 * \brief 服务器连接任务
 *
 * 一个区中的每一个服务器都需要和服务器管理器建立连接
 * 
 */
class ServerTask : public zTCPTask , public MessageQueue
{

	public:

		/**
		 * \brief 构造函数
		 *
		 * 用于创建一个服务器连接任务
		 *
		 * \param pool 所属连接池指针
		 * \param sock TCP/IP套接口
		 * \param addr 地址
		 */
		ServerTask(
				zTCPTaskPool *pool,
				const int sock,
				const struct sockaddr_in *addr = NULL) : zTCPTask(pool, sock, addr),lastSequenceTime(0)
		{
			wdServerID = 0;
			wdServerType = UNKNOWNSERVER;
			bzero(pstrName, sizeof(pstrName));
			bzero(pstrIP, sizeof(pstrIP));
			wdPort = 0;
			bzero(pstrExtIP, sizeof(pstrExtIP));
			wdExtPort = 0;

			OnlineNum = 0;
			m_tickTime = 0;	
			tickFlag = true;	
			
			sequenceOK = false;
			hasNotifyMe = false;
		}

		/**
		 * \brief 虚析构函数
		 *
		 */
		virtual ~ServerTask() {};

		int verifyConn();
		int waitSync();
		int recycleConn();
		void addToContainer();
		void removeFromContainer();
		bool uniqueAdd();
		bool uniqueRemove();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
		bool msgParse(const BYTE *data, const DWORD nCmdLen);
		bool cmdMsgParse(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
        void responseOther(const WORD wdServerID);

		void checkDeadLock(DWORD now);
		
		/**
		 * \brief 获取服务器编号
		 *
		 * \return 服务器编号
		 */
		const WORD getID() const
		{
			return wdServerID;
		}

		/**
		 * \brief 获取服务器类型
		 * \return 服务器类型
		 */
		const WORD getType() const
		{
			return wdServerType;
		}

		const char* getIP() const
		{
			return pstrIP;
		}
		
		/**
		 * \brief 获得服务器名字
		 * \return 服务器名字
		 */
		const char* getPstrName()
		{
			return pstrName; 
		}

		/**
		 * \brief 获得服务器端口
		 */
		const WORD getWdPort()
		{
			return wdPort;
		}
		
		/**
		 * \brief 获得服务器外部IP
		 */
		const char* getExtIP()
		{
			return pstrExtIP;
		}

		/**
		 * \breif 获得外部端口
		 */
		const WORD getWdExtPort()
		{
			return wdExtPort;
		}
		/**
		 * \brief 返回服务器在线人数
		 * \return 服务器在线人数
		 */
		const DWORD getOnlineNum() const
		{
			return OnlineNum;
		}

		/**
		 * \brief 检查最后一次处理启动顺序的时间
		 *
		 * \return 检查是否成功
		 */
		bool checkSequenceTime();
    
    private:
   
        bool verify(WORD wdType, const char *pstrIP);
		bool verifyTypeOK(const WORD wdType, std::vector<ServerTask *> &sv);
		bool processSequence();
		bool notifyOther();
		bool notifyOther(WORD dstID);
		bool notifyMe();
        void selfOk();
        
        //处理superCmd消息
        bool msgParseSuperCmd(const CMD::SUPER::SuperServerNull *superNull,const DWORD nCmdLen);
        //处理flCmd消息
        bool msgParseFlCmd(const CMD::FL::FLNullCmd *flNull,const DWORD nCmdLen);
        //处理GM工具指令
        bool msgParseGmToolCmd(const CMD::GMTool::GmToolNullCmd *gmToolNull,const DWORD nCmdLen);
        bool msgParseResourceCmd(const CMD::RES::ResNullCmd *resNull,const DWORD nCmdLen);
    private:
		WORD wdServerID;					/**< 服务器编号，一个区唯一的 */
		WORD wdServerType;					/**< 服务器类型，创建类实例的时候已经确定 */
		char pstrName[MAX_NAMESIZE];		/**< 服务器名称 */
		char pstrIP[MAX_IP_LENGTH];			/**< 服务器内网地址 */
		WORD wdPort;						/**< 服务器内网端口，也就是邦定端口 */
		char pstrExtIP[MAX_IP_LENGTH];		/**< 服务器外网地址，也就是防火墙地址 */
		WORD wdExtPort;						/**< 服务器外网端口，也就是映射到防火墙的端口 */
		WORD wdNetType;						/**< 服务器外网端口，也就是映射到防火墙的端口 */
        char pstrTable[MAX_TABLE_LIST];   // 数据关联表格

		DWORD			OnlineNum;			/**< 在线人数统计 */
	
		DWORD m_tickTime;					//死锁检测
		volatile bool tickFlag;
		
		zTime lastSequenceTime;				/**< 最后一次处理启动顺序的时间 */
		bool sequenceOK;					/**< 是否已经处理完成了启动顺序 */
		bool hasNotifyMe;

	
		struct key_hash
		{   
			size_t operator()(const CMD::SUPER::ServerEntry &x) const
			{   
				std::hash<WORD> H;
				return H(x.wdServerID);
			}   
		};  
		struct key_equal : public std::binary_function<CMD::SUPER::ServerEntry, CMD::SUPER::ServerEntry, bool>
		{   
			bool operator()(const CMD::SUPER::ServerEntry &s1, const CMD::SUPER::ServerEntry &s2) const
			{   
				return s1.wdServerID == s2.wdServerID;
			}   
		}; 

		typedef std::unordered_map<CMD::SUPER::ServerEntry, bool, key_hash, key_equal> Container;
		Container ses;

};

#endif

