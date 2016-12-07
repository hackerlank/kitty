/**
 * \file
 * \version  $Id: GatewayTask.h 64 2013-04-23 02:05:08Z  $
 * \author  ,@163.com
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 定义网关连接任务
 *
 */

#ifndef _GatewayTask_h_
#define _GatewayTask_h_

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "zMisc.h"
#include "GateUserManager.h"
#include "zTime.h"
#include "dispatcher.h"
#include "GateUser.h"
#include "login.pb.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <map>
class GatewayTask;

typedef ProtoDispatcher<GateUser> GateUserCmdDispatcher;
typedef ProtoDispatcher<GatewayTask> GateTaskCmdDispatcher;
typedef Dispatcher<GatewayTask> UserLoginCmdDispatcher;

enum
{
	GATEWAY_USER_LOGINSESSION_STATE_NONE,
	GATEWAY_USER_LOGINSESSION_STATE_REG,
	GATEWAY_USER_LOGINSESSION_STATE_OK,
};
/**
 * \brief 服务器连接任务
 *
 */
class GatewayTask : public zTCPTask
{
	public:
		GatewayTask(zTCPTaskPool *pool, const int sock, const struct sockaddr_in *addr = NULL);
		~GatewayTask();

		int verifyConn();
		int waitSync();
		void Terminate(const TerminateMethod method = terminate_passive);
		void addToContainer();
		void removeFromContainer();
		bool uniqueAdd();
		bool uniqueRemove();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
		bool msgParseStruct(const CMD::t_NullCmd *ptNull, const DWORD nCmdLen);
		bool checkTime(const zRTime &ct);
	    
        bool checkNewName(char *_newName);
		bool isInLoginState(){return inlonginstate;}
		void changeLoginState(){inlonginstate = true;}
		void login_return(const HelloKittyMsgData::LoginFailReason code);
		bool sendCmd(const void *pstrCmd, const DWORD nCmdLen);
		bool checkLoginTime();
        bool verifyVersion(const HelloKittyMsgData::ReqVersion *message);
		bool verifyACCID(const HelloKittyMsgData::ReqLoginGateway *message);
        
        static void initProtoDispatch();

		/**
		 * \brief 得到该玩家的帐号id
		 *
		 * \return 得到的id
		 */
		DWORD getACCID() const
		{
			return accid;
		}

		bool setACCID(DWORD _accid)
		{
			if (accid!=0) return false;

			accid=_accid;
			return true;
		}

		void setCharID(DWORD val)
		{
			this->charid = val;
		}
        //转发消息到scene服务器
        bool forwardScene(const google::protobuf::Message *message);
        //重连
        bool reqReconnect(const HelloKittyMsgData::ReqReconnectGateway *message);
    public:
        static GateTaskCmdDispatcher gate_task_cmd_dispatcher;
		static GateUserCmdDispatcher gate_user_cmd_dispatcher;
		static UserLoginCmdDispatcher user_login_dispatcher;
		static DWORD checkTimeInterval;
        static std::map<const google::protobuf::Descriptor*,ServerType> s_protoNameToServerTypeMap;
		DWORD acctype;
		std::string password;
		std::string strMacAddr; //MAC地址
		std::string strFlat; // 平台
		std::string strPhoneUuid; // 手机设备id
		std::string account;	//平台数字帐号
        DWORD lang;
        //重连
        bool reconnect;
		friend class GateUser;
		///使用该连接的玩家
		GateUser* m_pUser;
    private:
        //消息转发枢纽
	    bool forward_cmd_dispatch(GateUser* u,const google::protobuf::Message *message);	
		bool msgParse_Time(const CMD::NullCmd *ptNullCmd, const unsigned int nCmdLen);
		bool checkUserCmd(const CMD::NullCmd *pCmd, const zRTime &ct);
        //判断是否被禁言
        bool isForbid(const QWORD charID);
	private:
		///时间校对定时器
		Timer _retset_gametime;
		///校验客户端时间的间隔
		static const unsigned int sampleInterval = 20000;
		static const unsigned int sampleInterval_sec = sampleInterval/1000;
		static const unsigned int sampleInterval_error_sec = sampleInterval/1000;
		static const unsigned int sampleInterval_error_msecs = sampleInterval;

		///客户端在sampleInterval时间内发送超过maxSamplePPS个数据包则被判断为使用了外挂
#ifdef _ALL_SUPER_GM
		static const unsigned int maxSamplePPS = 200000; // 内网不限制消息数
#else
		static const unsigned int maxSamplePPS = 200;
#endif
		///上次晴空v_samplePackets的时间
		DWORD v_lastSampleTime;
		///统计数据包个数
		unsigned int v_samplePackets;

		///该task初始化的时间
		zRTime initTime;
		
		//小喇叭下次说话的时间
		zRTime nextLoudSpeakerTime;
		//私聊下次说话的时间
		zRTime nextChatTime;	
		//世界聊天下次说话的时间
		zRTime nextWorldChatTime;
	
		///上次检查客户端是否已经校验了时间的时间
		//zRTime lastCheckTime;
		Timer lastCheckTime;
		BYTE  timeout_count; // 校验时间超时次数
		///是否已经校验了时间
		volatile bool haveCheckTime;
		
		DWORD accid;
		QWORD charid;//一个GatewayTask 对应一个 GateUser, 一个GateUser 对应一个  charid,目前不支持多角色

		/**     
		 ** \brief 游戏时间
		 **
		 **/
		QWORD qwGameTime;	//游戏时间
		zRTime GameTimeSyn;
		QWORD dwTimestampServer;

		///登录时分配的临时id
		DWORD loginTempID;
		///是否验证了版本
		bool versionVerified;
        ///是否验证了accid
        bool verifyAccid;
		//是否在登录状态
		bool inlonginstate;

		static unsigned long long lastCmdTime;

		std::map<DWORD,DWORD> analysisCmd;

		//登陆限时
		Timer _login_gametime;
};

#endif

