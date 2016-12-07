/**
 * \file
 * \version  $Id: SceneClientToOtherManager.h 42 2013-04-10 07:33:59Z  $
 * \author  王海军, wanghaijun@ztgame.com 
 * \date 2006年01月04日 16时55分37秒 CST
 * \brief 网关到场景数据缓冲发送
 *
 * 
 */


#ifndef _SceneClientToOtherMANAGER_H_
#define _SceneClientToOtherMANAGER_H_

#include <map>
#include <set>

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "SceneClientToOther.h"
#include "zTime.h"
#include "zRWLock.h"
#include "extractProtoMsg.h"

/**
 ** \brief 定义服务器信息采集连接的客户端管理容器
 **/
class SceneClientToOtherManager : public Singleton<SceneClientToOtherManager>
{
	friend class Singleton<SceneClientToOtherManager>;
	public:

		~SceneClientToOtherManager();

		bool init();
		void timeAction(const zTime &ct);
		void add(SceneClientToOther *SceneClientToOther);
		void remove(SceneClientToOther *SceneClientToOther);
		bool broadcastOne(const void *pstrCmd, int nCmdLen);
		bool sendTo(const DWORD tempid, const void *pstrCmd, int nCmdLen);
		void setUsleepTime(int time)
		{
			SceneClientToOtherPool->setUsleepTime(time);
		}
		bool reConnectScene(const CMD::SUPER::ServerEntry *serverEntry);
		void setTaskReconnect(const std::string& ip, unsigned short port, bool reconn);
		SceneClientToOther* getSceneByID(DWORD id);
        void removebyId(const DWORD tempid);
	
		bool isAllStartOK();
        bool SendMsgToOtherScene(const DWORD ServerId,const void *pstrCmd, int nCmdLen);
        bool msgParseOtherSceneCmd(const DWORD ServerId,const CMD::SCENE::SceneNull *sceneCmd,const DWORD nCmdLen);
        void final();
        void execEvery();

        //给某个charid发送消息(charid在别的进程中)
        bool SendMsgToOtherSceneCharID(const QWORD charid,const _null_cmd_ *message,const DWORD len);
        bool SendMsgToOtherSceneCharID(const QWORD charid,const google::protobuf::Message *message);
        bool SendMsgToOtherSceneCharID(const QWORD charid,const void *data,const DWORD len);
	private:

		SceneClientToOtherManager();
		static SceneClientToOtherManager *instance;

		/**
		 ** \brief 客户端连接管理池
		 **/
		zTCPClientTaskPool *SceneClientToOtherPool;
		/**
		 ** \brief 进行断线重连检测的时间记录
		 **/
		zTime actionTimer;

		/**
		 ** \brief 存放连接已经成功的连接容器类型
		 **/
		typedef std::map<const DWORD, SceneClientToOther *> SceneClientToOther_map;
		typedef SceneClientToOther_map::iterator iter;
		typedef SceneClientToOther_map::const_iterator const_iter;
		typedef SceneClientToOther_map::value_type value_type;
		/**
		 ** \brief 存放连接已经成功的连接容器
		 **/
		SceneClientToOther_map allClients;


		/**
		 ** \brief 容器访问读写锁
		 **/
		zRWLock rwlock;

};

#endif

