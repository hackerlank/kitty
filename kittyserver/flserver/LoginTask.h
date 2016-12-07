/**
 * \file
 * \version  $Id: LoginTask.h 2877 2005-09-12 12:16:19Z whj $
 * \author  Songsiliang,
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 定义登陆连接任务
 *
 */

#ifndef _LoginTask_h_
#define _LoginTask_h_

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "zMisc.h"
#include "zDBConnPool.h"
#include "LoginCommand.h"
#include "zTCPTask.h"
#include "zTime.h"
#include "FLServer.h"
#include "SlaveCommand.h"
#include "dispatcher.h"
#include "extractProtoMsg.h"
#include "login.pb.h"
#include "AccountMgr.h"

#define TCP_TYPE			0
/**
 * \brief 服务器连接任务
 *
 */

class LoginTask;
typedef ProtoDispatcher<LoginTask> LoginUserCmdDispatcher;

class LoginTask : public zTCPTask
{

	public:

		LoginTask( zTCPTaskPool *pool, const int sock);
		/**
		 * \brief 虚析构函数
		 *
		 */
		~LoginTask() {};

		int verifyConn();
		int recycleConn();
		bool uniqueAdd();
		bool uniqueRemove();
        
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);

        inline void genTempID()
		{
			tempid = (((uniqueID % (FLService::getMe().getMaxPoolSize() * 4)) + 1) << 1) + TCP_TYPE;
			uniqueID++;
		}

		inline const DWORD getTempID() const
		{
			return tempid;
		}

		/**
		 * \brief 登陆错误，返回错误代码到客户端显示
		 *
		 * \param retcode 错误代码
		 * \param tm 是否断开连接
		 */
		inline void LoginReturn(const BYTE retcode, const bool tm = true)
		{
#if 0
			using namespace CMD;
            login::ServerReturnLoginFailedCmd message;
            message.set_byreturncode(retcode);
            
            std::string buffer;
            if(encodeMessage(&message,buffer))
            {
                sendCmd(buffer.c_str(),buffer.size());
            }
#endif 
            //由于登陆错误，需要断开连接
			//whj 可能导致coredown,屏蔽测试
			if (tm)  Terminate();
		}
        
        void LoginReturnMsg(const std::string& err_msg, const HelloKittyMsgData::LoginFailReason reason,const bool tm = true);

		inline void LoginReturnMtcard(const CMD::stServerReturnLoginFailedCmd *tCmd, const bool tm = true)
		{
			
			sendCmd(tCmd, sizeof(CMD::stServerReturnLoginFailedCmd) + tCmd->size);

			//由于登陆错误，需要断开连接
			//whj 可能导致coredown,屏蔽测试
			if (tm)  Terminate();
		}

		/**
		 * \brief 判断登陆连接是否过长
		 * 如果登陆连接太长，登陆服务器应该主动断开连接
		 * \param ct 当前时间
		 * \return 登陆时间是否过长
		 */
		inline bool timeout(const zTime &ct)
		{
            return lifeTime.elapse(ct) >= 90 ? true : false;
		}

		inline bool checkACCNAME(const char *name) const { return 0 == strncasecmp(acc_name, name, sizeof(acc_name)); }
		inline bool checkClientIP(const char *clientIP) const { return 0 == strncmp(clientIP, getIP(), strlen(clientIP)); }
		inline const char *getACCNAME() const { return acc_name; }
		inline void setAccount(const char *name) 
		{
			bzero(acc_name, sizeof(acc_name));
			strncpy(acc_name, name, sizeof(acc_name));
			return;
		}
		void GetPassTmp( unsigned char *pszPass,std::string &passwd, int iLen )
		{

			char szTmp[65];
			BYTE len = pszPass[0];
			{
				memset( szTmp, 0, sizeof(char) * 65 );

				for ( int i = 0, j = 0; j < len && j < iLen - 1 ; i++, j += 2 )
				{
					DecryChar(szTmp + i, (unsigned char*)pszPass + 1 + j);
				}
				szTmp[len / 2] = 0;
			}
			passwd = szTmp;
			return;
		}
		void DecryChar(char *pszDes,unsigned  char *pszSrc )
		{
			BYTE array;
			char btmp, btmp1;

			BYTE keyData[8]={210, 41, 182, 141, 14, 242, 120, 178};//?ü??
			//BYTE keyData[8];
			//size_t keySize;
			array = (pszSrc[0] >> 4) & 0x0F;
			btmp = pszSrc[0] ^ keyData[array];
			btmp1 = pszSrc[1] ^ keyData[array];
			btmp = (btmp << 4) & 0xF0;
			btmp1 = (btmp1 >> 4) & 0x0F;
			btmp |= btmp1;

			pszDes[0] = btmp - 2;
		}

		/**
		 * \brief 解密
		 * \param pszKey 密钥
		 * \param pszSrc 要解密的字符串已经解密的结果
		 * \param iNum 要解密的字符串长度
		 */
		void UnUseIPEncry( const char *pszKey, unsigned char *pszSrc, int iNum )
		{
			BYTE nKey = strlen( pszKey ), rkey = 0;
			
			for ( int i = 0; i < iNum; i++ )
			{
				pszSrc[i]--;
				pszSrc[i] ^= pszKey[rkey];
				
				if ( ++rkey >= nKey )
					rkey = 0;
			}
		}

		/**
		 * \brief 解密
		 * \param src 源密码
		 * \param dest 解密后的密码
		 */
		void DecMatirxPasswd(char *dest, unsigned char *src)
		{
			for ( int i = 0, j = 0; j < 12; i++, j += 2 )
				DecryChar( dest + i ,(unsigned char*)src + j );
		}

        bool requireLogin(const HelloKittyMsgData::ReqLogin *message);
        void setClientVersion(const float version);
        bool ReqRegister(const HelloKittyMsgData::ReqRegister *meaasge);
        bool DoReqLogin(bool bSuc = true);
#if 0
        bool verifyToken(const login::IphoneUserRequestLoginCmd *message);
#endif  
    public:
        static LoginUserCmdDispatcher login_user_dispatcher;
        const AccountInfo & account(){return m_accountinfo;}

    private:
        
        void getClientIP(char *clientIP);
		void verify_login(const DWORD loginTempID, const t_NewLoginSession& session);
            private:

		/**
		 * \brief 校验客户端版本号
		 */
		float verify_client_version;
		
		/**
		 * \brief 生命期时间
		 */
		zTime lifeTime;
		/**
		 * \brief 临时唯一编号
		 *
		 */
		DWORD tempid;
		char acc_name[48];
		/**
		 * \brief 临时唯一编号分配器
		 *
		 */
		static DWORD uniqueID;
        AccountInfo  m_accountinfo;
};

#endif


