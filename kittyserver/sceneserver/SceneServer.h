/**
 * \file
 * \version  $Id: SceneServer.h 67 2013-04-23 09:44:20Z  $
 * \author  ,
 * \date 2013年04月07日 15时55分53秒 CST
 * \brief Fir场景服务器
 *
 */

#ifndef _SCENESERVER_H_
#define _SCENESERVER_H_

#include "zSubNetService.h"
#include "Fir.h"
#include "zMisc.h"
#include "zMemDBPool.h"
#include "zCmdHandle.h"
#include "MessageQueue.h"
#include "zDBConnPool.h"
#include "GmToolCommand.h"
#include "ResourceCommand.h"

class SceneUser;

//SUPER指令缓冲
class SuperCmdQueue : public MessageBuffer<100>
{
    private:
        //处理全区来的c++消息
        bool allZoneMsgParseStruct(SceneUser *user,const CMD::t_NullCmd *ptNullCmd,const DWORD cmdLen);
        //处理全区来的proto消息
        bool allZoneMsgParseProto(SceneUser *user,const BYTE *data,const DWORD cmdLen);
        //处理superCmd消息
        bool msgParseSuperCmd(const CMD::SUPER::SuperServerNull *superNull,const DWORD nCmdLen);
        //处理GM工具指令
        bool msgParseGmToolCmd(const CMD::GMTool::GmToolNullCmd *gmToolNull,const DWORD nCmdLen);
        bool msgParseResourceCmd(const CMD::RES::ResNullCmd *resNull,const DWORD nCmdLen);
    public:
        //处理全区来的所有消息(必须是c++消息头，服务器内部消息)
        bool cmdMsgParse(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
        //分发消息c++和proto消息
        bool allzoneMsgParse(SceneUser *user,const BYTE *data, const DWORD cmdLen);
};

/**
 * \brief 定义计费服务类
 *
 * 这个类使用了Singleton设计模式，保证了一个进程中只有一个类的实例
 *
 */
class SceneService : public zSubNetService, public Singleton<SceneService>
{
	friend class Singleton<SceneService>;
	public:
		WORD getZoneID () const
		{
			return zoneID.zone;
		}
		~SceneService()
		{
			instance = NULL;
			//关闭线程池
			if (taskPool)
			{
				taskPool->final();
				SAFE_DELETE(taskPool);
			}
		}
		const int getPoolSize() const
		{
			if(taskPool)
			{
				return taskPool->getSize();
			}
			else
			{
				return 0;
			}
		}
        bool isSequeueTerminate() 
		{
			return taskPool == NULL;
		}
        bool msgParse_SuperService(const CMD::t_NullCmd *ptNullCmd, const unsigned int nCmdLen);
        //重新加载配置文件
		void reloadconfig();
        bool initConfig();
        virtual void getnewServerEntry(const CMD::SUPER::ServerEntry &entry);
        virtual void getOtherseverinfo();
    public:
        //MD5存档验证
        bool md5Verify;
		int writeBackGroup;
		GameZone_t zoneID;
		SuperCmdQueue superCmd;
        static DWORD cmd_record[4];
		static char cmd_recordNew[zSocket::MAX_DATASIZE];
		static DWORD cmd_len;
        zCmdHandleManager cmd_handle_manager;
        static zDBConnPool *dbConnPool;
        static MetaData* metaData;
    private:
        /**
		 * \brief 构造函数
		 *
		 */
		SceneService() : zSubNetService("场景服务器", SCENESSERVER)
		{
			taskPool = NULL;
			md5Verify = false;
			writeBackGroup = 10;
		}

		bool init();
		void newTCPTask(const int sock, const struct sockaddr_in *addr);
		void final();
    private:

		/**
		 * \brief 类的唯一实例指针
		 *
		 */
		static SceneService *instance;

		zTCPTaskPool *taskPool;				/**< TCP连接池的指针 */
};

#endif

