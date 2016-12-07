#ifndef _RECHARGE_CMD_DISPATCHER
#define _RECHARGE_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "recharge.pb.h"
class SceneUser;

class RechargeCmdHandle : public zCmdHandle
{
    public:
        RechargeCmdHandle()
        {
        }

        void init(){
#define REGISTERRECHARGE(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &RechargeCmdHandle::SELFFUN));
#define REGISTERRECHARGESAME(PROTOCFUN) REGISTERRECHARGE(PROTOCFUN,PROTOCFUN) 
            REGISTERRECHARGESAME(ReqChangeMoney);
            REGISTERRECHARGESAME(ReqChangeMoneyList);




        }
        bool ReqChangeMoney(SceneUser* u, const HelloKittyMsgData::ReqChangeMoney* cmd);
        bool ReqChangeMoneyList(SceneUser* u, const HelloKittyMsgData::ReqChangeMoneyList* cmd);



};

#endif

