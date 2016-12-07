#ifndef GM_TOOL_TASK_H
#define GM_TOOL_TASK_H 

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "zMisc.h"
#include "zDBConnPool.h"
#include "zTCPTask.h"
#include "zTime.h"
#include "GmToolServer.h"
#include "dispatcher.h"
#include "extractProtoMsg.h"
#include "gmtool.pb.h"
#include "GmTool.h"

#define TCP_TYPE			0

class GmToolTask;
typedef ProtoDispatcher<GmToolTask> GmToolCmdDispatcher;

class GmToolTask: public zTCPTask
{
	public:
		GmToolTask( zTCPTaskPool *pool, const int sock);
		~GmToolTask() {};
		int verifyConn();
		int recycleConn();
		bool uniqueAdd();
		bool uniqueRemove();
        
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);

        inline void genTempID()
		{
			m_tempid = (((uniqueID % (GmToolService::getMe().getMaxPoolSize() * 4)) + 1) << 1) + TCP_TYPE;
            ++uniqueID;
		}

		inline const DWORD getTempID() const
		{
			return m_tempid;
		}

		inline bool timeout(const zTime &ct)
		{
            return m_lifeTime.elapse(ct) >= 90 ? true : false;
		}
        inline const char* getAccount()
        {
            return m_account;
        }
        inline const char* getPasswd()
        {
            return m_passwd;
        }
        inline void setPasswd(const char *passwd)
        {
            bzero(m_passwd,sizeof(m_passwd));
            strncpy(m_passwd,passwd,sizeof(m_passwd));
        }
        inline void setPermission(const DWORD permission)
        {
            m_permission = permission;
        }
        //存档
        bool save(const bool &forbid = false);
    private:
        void getClientIP(char *clientIP);
    public:
        static GmToolCmdDispatcher gm_tool_dispatcher;
    public:
        //登录
        bool login(const char *account,const char *passwd);
        //修改密码
        bool motifyPasswd(const char *account,const char *passwd,const char *newPasswd);
        //修改GM密码
        bool motifyPasswd(const char *account,const char *newPasswd);
        //删除GM用户
        bool delGmUser(const char *account);
        //修改GM操作权限
        bool motifyPermission(const char *account,const DWORD permission);
        //显示所有GM用户
        bool showAllGm();
        //批量修改GM用户信息
        bool modifyGm(const HelloKittyMsgData::ReqModityGmData *message);
        //批量修改角色属性
        bool modifyUserAttr(const HelloKittyMsgData::ReqModifyUserAttr *message);
        //批量修改建筑属性
        bool modifyUserBuild(const HelloKittyMsgData::ReqModifyUserBuild *message);
        //批量封禁号
        bool reqForbid(const HelloKittyMsgData::ReqForbid *message);
        //请求发送邮件
        bool reqSendEmail(const HelloKittyMsgData::ReqGmToolSendEmail *message);
        //请求处理公告
        bool reqOpNotice(const HelloKittyMsgData::ReqOpNotice *message);
        //请求修改单号
        bool modifyCashDelivery(const HelloKittyMsgData::ReqModifyCash *message);
        //修改礼品库存
        bool modifyGiftStore(const HelloKittyMsgData::ReqModifyGiftStore *message);
        //删除照片
        bool delUserPicture(const HelloKittyMsgData::ReqDelUserPicture *message);
        //活动
        bool ReqAddPlayerActive(const HelloKittyMsgData::ReqAddPlayerActive *message);
        bool ReqModifyPlayerActive(const HelloKittyMsgData::ReqModifyPlayerActive *message);
        bool ReqOpenActive(const HelloKittyMsgData::ReqOpenActive *message);
        bool sendGlobalEmail(const HelloKittyMsgData::ReqSendGlobalEmail *message);
        bool opGiftInfo(const HelloKittyMsgData::ReqModifyGiftInfo *message);
        bool modifyUserVerify(const HelloKittyMsgData::ReqModifyUserVerify *message);
        bool reqAddActiveCode(const HelloKittyMsgData::ReqAddActiveCode *message);
        static bool randDigist(std::string &ret,const DWORD num);
        static bool randChar(std::string &ret,const DWORD num);
        static bool initAddActiveCode();
        static bool AddActiveCode(const std::map<DWORD,DWORD> &rewardMap,const std::string &name,const DWORD num,const DWORD type);
    private:
		zTime m_lifeTime;
		DWORD m_tempid;
        char m_account[MAX_NAMESIZE];
        char m_passwd[MAX_NAMESIZE];
        char m_des[MAX_NAMESIZE];
        bool m_save;
        DWORD m_permission;  
		static DWORD uniqueID;
        //指令流水号
        static DWORD actionID;
    public:
        std::map<DWORD,std::set<QWORD>> m_opIDMap;
        HelloKittyMsgData::AckModifyUserAttr m_modifyAttrAck;
        HelloKittyMsgData::AckModifyUserBuild m_modifyBuildAck;
        HelloKittyMsgData::AckForbid m_forbidAck;
        HelloKittyMsgData::AckSendEmail m_emailAck;
        HelloKittyMsgData::AckModifyCash m_giftCashAck;
        HelloKittyMsgData::AckDelUserPicture m_delPicture;
        HelloKittyMsgData::AckSendGlobalEmail m_sendGlobalEmail;
        HelloKittyMsgData::AckModifyGiftInfo m_modifyGiftInfo;
        HelloKittyMsgData::AckModifyUserVerify m_modifyVerify;
};

#endif


