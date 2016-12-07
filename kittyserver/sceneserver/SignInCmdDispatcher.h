#ifndef _SIGNIN_CMD_DISPATCHER
#define _SIGNIN_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "signin.pb.h"
class SceneUser;

class SignInCmdHandle : public zCmdHandle
{
    public:
        SignInCmdHandle()
        {
        }

        void init()
        {
#define REGISTERSIGNIN(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &SignInCmdHandle::SELFFUN));
#define REGISTERSIGNINSAME(PROTOCFUN) REGISTERSIGNIN(PROTOCFUN,PROTOCFUN) 
            REGISTERSIGNINSAME(ReqGetSignInData);
            REGISTERSIGNINSAME(ReqSignIn);
            REGISTERSIGNINSAME(ReqGetTotalAward);
        }
        bool ReqGetSignInData(SceneUser* u, const HelloKittyMsgData::ReqGetSignInData* cmd);
        bool ReqSignIn(SceneUser* u, const HelloKittyMsgData::ReqSignIn* cmd);
        bool ReqGetTotalAward(SceneUser* u, const HelloKittyMsgData::ReqGetTotalAward* cmd);



};

#endif

