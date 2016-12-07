/**
 * \file
 * \version  $Id: LoginCmdDispatcher.h 42 2013-04-10 07:33:59Z  $
 * \author   ,
 * \date 2013年03月27日 12时14分48秒 CST
 * \brief 定义用户登录相关命令处理文件，注册给dispatcher
 *
 */

#ifndef _LOGIN_USER_CMD_DISPATCHER
#define _LOGIN_USER_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "LoginTask.h"
#include "login.pb.h"

class LoginCmdHandle : public zCmdHandle
{
	public:
		LoginCmdHandle()
		{

		}

		void init()
		{
            LoginTask::login_user_dispatcher.func_reg<HelloKittyMsgData::ReqVersion>(ProtoCmdCallback<LoginTask,HelloKittyMsgData::ReqVersion>::ProtoFunction(this, &LoginCmdHandle::setVersion));
            LoginTask::login_user_dispatcher.func_reg<HelloKittyMsgData::ReqLogin>(ProtoCmdCallback<LoginTask,HelloKittyMsgData::ReqLogin>::ProtoFunction(this, &LoginCmdHandle::requireLogin));
            LoginTask::login_user_dispatcher.func_reg<HelloKittyMsgData::ReqRegister>(ProtoCmdCallback<LoginTask,HelloKittyMsgData::ReqRegister>::ProtoFunction(this, &LoginCmdHandle::ReqRegister));

        }
		bool setVersion(LoginTask* task,const HelloKittyMsgData::ReqVersion *meaasge);
        bool ReqRegister(LoginTask* task,const HelloKittyMsgData::ReqRegister *meaasge);
        bool requireLogin(LoginTask* task,const HelloKittyMsgData::ReqLogin *message);
};

#endif
