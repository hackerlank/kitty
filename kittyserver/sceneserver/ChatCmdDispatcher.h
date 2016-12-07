#ifndef _Chat_CMD_DISPATCHER
#define _Chat_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "chat.pb.h"

class SceneUser;

class ChatCmdHandle : public zCmdHandle
{
    public:
        ChatCmdHandle()
        {
        }

        void init(){
#define REGISTERChat(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &ChatCmdHandle::SELFFUN));
#define REGISTERChatSAME(PROTOCFUN) REGISTERChat(PROTOCFUN,PROTOCFUN) 
            REGISTERChatSAME(ReqChat);
            REGISTERChatSAME(ReqLeaveMessage);
            REGISTERChatSAME(ReqServerNotice);
            REGISTERChatSAME(ReqDelMessage);




        }
        //请求聊天
        bool ReqChat(SceneUser* u, const HelloKittyMsgData::ReqChat* cmd);
        bool ReqLeaveMessage(SceneUser* u, const HelloKittyMsgData::ReqLeaveMessage* cmd);
        bool ReqServerNotice(SceneUser* u, const HelloKittyMsgData::ReqServerNotice* cmd);
        bool ReqDelMessage(SceneUser* u, const HelloKittyMsgData::ReqDelMessage* cmd);

#if 0 
        static void SendSysMsg(std::string &sysChat);

#endif
};

#endif

