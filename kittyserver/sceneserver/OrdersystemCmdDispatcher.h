#ifndef _ORDERSYSTEM_CMD_DISPATCHER
#define _ORDERSYSTEM_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "ordersystem.pb.h"
class SceneUser;
class ordersystemCmdHandle : public zCmdHandle
{
    public:
        ordersystemCmdHandle()
        {
        }

        void init(){
#define REGISTERORDERSYSTEM(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &ordersystemCmdHandle::SELFFUN));
#define REGISTERORDERSYSTEMSAME(PROTOCFUN) REGISTERORDERSYSTEM(PROTOCFUN,PROTOCFUN) 
            REGISTERORDERSYSTEMSAME(ReqAddOrderSystem);
            REGISTERORDERSYSTEMSAME(ReqRunOrderSystem);
            REGISTERORDERSYSTEMSAME(ReqFinishOrderSystem);
            REGISTERORDERSYSTEMSAME(ReqClearOrderSystemCD);

        }
        bool ReqAddOrderSystem(SceneUser* u, const HelloKittyMsgData::ReqAddOrderSystem* cmd);
        bool ReqRunOrderSystem(SceneUser* u, const HelloKittyMsgData::ReqRunOrderSystem* cmd);
        bool ReqFinishOrderSystem(SceneUser* u, const HelloKittyMsgData::ReqFinishOrderSystem* cmd);
        bool ReqClearOrderSystemCD(SceneUser* u, const HelloKittyMsgData::ReqClearOrderSystemCD* cmd);
    private:
        DWORD getOrdernum(SceneUser* u);
        DWORD getOrderTimer(SceneUser* u,DWORD oldTimer);



};

#endif

