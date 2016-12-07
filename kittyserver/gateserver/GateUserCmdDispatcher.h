/**
 * \file
 * \author   ,
 * \date 2013年03月27日 12时14分48秒 CST
 * \brief 定义用户登录相关命令处理文件，注册给dispatcher
 *
 */

#ifndef _GATE_USER_CMD_DISPATCHER
#define _GATE_USER_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "GatewayServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "LoginUserCommand.h"
#include "SuperCommand.h"
#include "login.pb.h"
#include "enterkitty.pb.h"
#include "system.pb.h"

class GateUser;
class GatewayTask;

enum LoginState
{
	LOGIN_NONE = 0, // 初始状态
	LOGIN_WAIT, // 通过平台验证，等待玩家登录
	LOGIN_LOGINING, //正在登录，其它网关，不再允许重复登录,登录流程结束，回到初始状态,或者删除这个KEY
};


class GateUserCmdHandle : public zCmdHandle
{
	public:

		virtual void init()
		{
			Fir::logger->debug("%s", __PRETTY_FUNCTION__);
			GatewayTask::gate_user_cmd_dispatcher.func_reg<HelloKittyMsgData::ReqCreateRole>(ProtoCmdCallback<GateUser,HelloKittyMsgData::ReqCreateRole>::ProtoFunction(this, &GateUserCmdHandle::create_role));

			GatewayTask::gate_user_cmd_dispatcher.func_reg<HelloKittyMsgData::ReqOpReceiveWorldChatCmd>(ProtoCmdCallback<GateUser,HelloKittyMsgData::ReqOpReceiveWorldChatCmd>::ProtoFunction(this, &GateUserCmdHandle::op_receive_world_chat));

            GatewayTask::gate_user_cmd_dispatcher.func_reg<HelloKittyMsgData::ReqHeartBeat>(ProtoCmdCallback<GateUser,HelloKittyMsgData::ReqHeartBeat>::ProtoFunction(this, &GateUserCmdHandle::reqHeartBeat));

            GatewayTask::gate_user_cmd_dispatcher.func_reg<HelloKittyMsgData::ReqLogout>(ProtoCmdCallback<GateUser,HelloKittyMsgData::ReqLogout>::ProtoFunction(this, &GateUserCmdHandle::reqLoginOut));


		}

        bool op_receive_world_chat(GateUser* u,const HelloKittyMsgData::ReqOpReceiveWorldChatCmd *message);
		bool create_role(GateUser* u, const HelloKittyMsgData::ReqCreateRole *message);
        //心跳包
        bool reqHeartBeat(GateUser* u,const HelloKittyMsgData::ReqHeartBeat *message);
        //注销
        bool reqLoginOut(GateUser* u,const HelloKittyMsgData::ReqLogout *message); 
};

#endif
