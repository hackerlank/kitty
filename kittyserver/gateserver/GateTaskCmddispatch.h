#ifndef GATE_TASK_CMD_DISPATCH_H
#define GATE_TASK_CMD_DISPATCH_H

#include <string.h>
#include "Fir.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "GatewayTask.h"
#include "login.pb.h"

class GateTaskCmdHandle : public zCmdHandle
{
    public:
        GateTaskCmdHandle()
        {
        }

        virtual void init()
        {
            GatewayTask::gate_task_cmd_dispatcher.func_reg<HelloKittyMsgData::ReqVersion>(ProtoCmdCallback<GatewayTask,HelloKittyMsgData::ReqVersion>::ProtoFunction(this, &GateTaskCmdHandle::vertifyVresion));
            
            GatewayTask::gate_task_cmd_dispatcher.func_reg<HelloKittyMsgData::ReqLoginGateway>(ProtoCmdCallback<GatewayTask,HelloKittyMsgData::ReqLoginGateway>::ProtoFunction(this, &GateTaskCmdHandle::verifyACCID));

            GatewayTask::gate_task_cmd_dispatcher.func_reg<HelloKittyMsgData::ReqReconnectGateway>(ProtoCmdCallback<GatewayTask,HelloKittyMsgData::ReqReconnectGateway>::ProtoFunction(this, &GateTaskCmdHandle::reqReconnect));

        }
        //验证版本号
        bool vertifyVresion(GatewayTask *task,const HelloKittyMsgData::ReqVersion *message);
        //验证accid
        bool verifyACCID(GatewayTask *task,const HelloKittyMsgData::ReqLoginGateway *message);
        //断线重连
        bool reqReconnect(GatewayTask *task,const HelloKittyMsgData::ReqReconnectGateway*message);
};

#endif
          
