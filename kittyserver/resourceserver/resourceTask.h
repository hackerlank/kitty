#ifndef RESOURCE_TASK_H
#define RESOURCE_TASK_H 

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "zMisc.h"
#include "zDBConnPool.h"
#include "zTCPTask.h"
#include "zTime.h"
#include "resourceServer.h"
#include "dispatcher.h"
#include "extractProtoMsg.h"

#define TCP_TYPE			0

class ResourceTask;
typedef ProtoDispatcher<ResourceTask> ResCmdDispatcher;

class ResourceTask: public zTCPTask
{
	public:
		ResourceTask( zTCPTaskPool *pool, const int sock);
		~ResourceTask() {};
		int verifyConn();
		int recycleConn();
		bool uniqueAdd();
		bool uniqueRemove();
        
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);

        inline void genTempID()
		{
			m_tempid = (((uniqueID % (ResourceService::getMe().getMaxPoolSize() * 4)) + 1) << 1) + TCP_TYPE;
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
    private:
        void getClientIP(char *clientIP);
    public:
        static ResCmdDispatcher res_dispatcher;
    public:
        //登录
        bool login();
    private:
		zTime m_lifeTime;
		DWORD m_tempid;
		static DWORD uniqueID;
        //指令流水号
        static DWORD actionID;
};

#endif


