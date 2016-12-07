/**
 * \file
 * \version  $Id: GateUser.h 65 2013-04-23 09:34:49Z  $
 * \author  ,@163.com
 * \date 2005年04月01日 11时54分48秒 CST
 * \brief 定义网关用户类
 */

#ifndef _GATEUSER_H_
#define _GATEUSER_H_

#include <set>
#include "Command.h"
#include "RecordCommand.h"
#include "LoginUserCommand.h"
#include "Fir.h"
#include "zRWLock.h"
#include "CmdAnalysis.h"
#include "nullcmd.h"
#include "login.pb.h"
#include "extractProtoMsg.h"

class GateUserManager;
class GatewayTask;
class SceneClient;
/**
 * \brief 网关用户
 *
 */
class GateUser
{
	friend class GatewayTask;
	friend class GateUserAccountID;
	friend class RecordClient;

	public:
		DWORD accid;
		QWORD charid;
		std::string strMacAddr; //MAC地址
		std::string strFlat; // 平台
		DWORD heroid; // 选中的英雄id
		std::string strPhoneUuid; // 手机设备id
		std::string account;	//平台数字帐号
        BYTE lang;
        //重连
        bool reconnect;

		void lock()
		{   
			mlock.lock();
		}   

		void unlock()
		{   
			mlock.unlock();
		}   
		char nickname[MAX_NAMESIZE+1];
	private:
		zMutex mlock;
        //有多少次没有接收到心跳包请求
        DWORD heartTime;
	public:

	// 游戏状态
	enum Systemstate
	{
		SYSTEM_STATE_INITING,		/// 初始状态
		SYSTEM_STATE_CREATING,		/// 创建角色状态
		SYSTEM_STATE_PLAY,			/// 游戏状态
		SYSTEM_WAIT_STATE_PLAY,		/// 等待游戏状态
		SYSTEM_WAIT_STATE_UNREG		/// 等待退出角色流程
	};
	volatile Systemstate systemstate;
	void initState(){systemstate = SYSTEM_STATE_INITING;}
	bool isInitState() const {return SYSTEM_STATE_INITING == systemstate;}
	void createState(){systemstate = SYSTEM_STATE_CREATING;}
	bool isCreateState() const{return SYSTEM_STATE_CREATING == systemstate;}
	void playState(SceneClient *s=NULL , DWORD scene_tempid=0);
	bool isPlayState() const{return SYSTEM_STATE_PLAY == systemstate;}
	void waitPlayState(){systemstate = SYSTEM_WAIT_STATE_PLAY;}
	bool isWaitPlayState() const{return SYSTEM_WAIT_STATE_PLAY == systemstate;}
	void waitUnregState(){systemstate = SYSTEM_WAIT_STATE_UNREG;}
	bool isWaitUnregState() const{return (SYSTEM_WAIT_STATE_UNREG == systemstate);}
	int getState(){return systemstate;}


	SceneClient *scene;
	GatewayTask *gatewaytask;
	DWORD scene_id;
	BYTE m_byCreate; 

	DWORD acctype;
	bool needReceiveWorldChat;
	GateUser(DWORD accID,GatewayTask *histask);
	GateUser(const std::string& UUID, GatewayTask* thistask);
	GateUser(WORD wdlogintype, const std::string& strPlatAccount, GatewayTask* thistask);
	~GateUser();

	void TerminateWait();
	bool isTerminateWait();
	void Terminate();
	bool isTerminate();
	

	/**
	 * \brief 通知客户端没有创建的角色
	 *
	 */
	void noCharInfo()
	{
        HelloKittyMsgData::AckLoginFailReturn message;
        message.set_failreason(HelloKittyMsgData::NotRole);
        
        std::string ret;
        if(encodeMessage(&message,ret))
        {
            sendCmd(ret.c_str(),ret.size());
        }
	}

	/**
	 * \brief 通知客户端名字重复
	 *
	 */
	void nameRepeat()
	{
        HelloKittyMsgData::AckLoginFailReturn message;
        message.set_failreason(HelloKittyMsgData::NameAgain);
        
        std::string ret;
        if(encodeMessage(&message,ret))
        {
            sendCmd(ret.c_str(),ret.size());
        }
	}
	
	bool beginSelect();
	bool reg();
	void unreg();
	bool sendCmd(const void *pstrCmd, const unsigned int nCmdLen,const bool hasPacked=false);
	void final();

	public:
        GatewayTask* getGatewayTask() { return gatewaytask; }
        bool syncMemDBBase();
        void addHeartTime()
        {
            ++heartTime;
            //Fir::logger->debug("[心跳包] (%s,%lu,%u)",account.c_str(),charid,heartTime);
        }
        void resetHearTime()
        {
            heartTime = 0;
        }
        //一分钟就算断开
        bool clentIsTerminate()
        {
            return heartTime >= HeartTime;
        }
    public:
        static DWORD HeartTime;

};
#endif
