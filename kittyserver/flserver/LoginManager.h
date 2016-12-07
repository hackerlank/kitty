/**
 * \file
 * \version  $Id: LoginManager.h 2536 2005-08-22 06:52:38Z whj $
 * \author  Songsiliang,
 * \date 2004年12月17日 13时17分28秒 CST
 * \brief 登陆连接管理容器
 *
 * 
 */


#ifndef _LoginManager_h_
#define _LoginManager_h_

#include <ext/hash_map>

#include "zEntry.h"
#include "LoginTask.h"
#include "zMisc.h"
#include "zRWLock.h"
#include "login.pb.h"

/**
 * \brief 登陆连接管理容器
 *
 * 管理所有的登陆连接的容器，方便查找连接
 *
 */
class LoginManager : public Singleton<LoginManager>
{

	public:

		/**
		 ** \brief 网关最大容纳用户数目
		 **
		 **/
		static DWORD maxGatewayUser;

		/**
		 * \brief 定义回调函数类
		 *
		 */
		typedef zEntryCallback<LoginTask, void> LoginTaskCallback;

		bool add(LoginTask *task);
		void remove(LoginTask *task);
		bool broadcast(const DWORD loginTempID, const void *pstrCmd, int nCmdLen);
		bool broadcastNewSession(const DWORD loginTempID, const t_NewLoginSession &session);
		void loginReturn(const DWORD loginTempID, const BYTE retcode, const bool tm = true);
		void loginReturnMtcard(const DWORD loginTempID,const char *name, CMD::stServerReturnLoginFailedCmd *tCmd, const bool tm = true);
		void execAll(LoginTaskCallback &cb);

		/**
		 * \brief sdk返回接着登陆
		 */
		bool CallBackLogin(const DWORD loginTempID, bool bsuc);
        bool SendVertifyToPlat(LoginTask *pTask);
        static void httpcallback(const unsigned int & serverid, const unsigned int &loginTempID, const std::string &ret_data);
#if 0
		/**
		 * \brief sdk返回失败
		 */
		void callBackLoginFail(const DWORD loginTempID, const HelloKittyMsgData::LoginFailReason error);
#endif
	private:

		friend class Singleton<LoginManager>;
		LoginManager(){};
		~LoginManager() {};

		/**
		 * \brief 定义容器类型
		 *
		 */
		typedef __gnu_cxx::hash_map<DWORD, LoginTask *> LoginTaskHashmap;
		/**
		 * \brief 定义容器迭代器类型
		 *
		 */
		typedef LoginTaskHashmap::iterator LoginTaskHashmap_iterator;
		/**
		 * \brief 定义容器常量迭代器类型
		 *
		 */
		typedef LoginTaskHashmap::const_iterator LoginTaskHashmap_const_iterator;
		/**
		 * \brief 定义容器键值对类型
		 *
		 */
		typedef LoginTaskHashmap::value_type LoginTaskHashmap_pair;
		/**
		 * \brief 读写锁，保证原子访问容器
		 *
		 */
		zRWLock rwlock;
		/**
		 * \brief 子连接管理容器类型
		 *
		 */
		LoginTaskHashmap loginTaskSet;



};

#endif

