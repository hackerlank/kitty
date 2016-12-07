#include "GateTaskCmddispatch.h"

bool GateTaskCmdHandle::vertifyVresion(GatewayTask *task,const HelloKittyMsgData::ReqVersion *message)
{
    if(!task || !message)
    {
        return false;
    }
    return task->verifyVersion(message);
}

bool GateTaskCmdHandle::verifyACCID(GatewayTask *task,const HelloKittyMsgData::ReqLoginGateway *message)
{
    if(!task || !message) 
    {
        return false;
    }

    return task->verifyACCID(message);
}

bool GateTaskCmdHandle::reqReconnect(GatewayTask *task,const HelloKittyMsgData::ReqReconnectGateway*message)
{
    if(!task || !message) 
    {
        return false;
    }

    return task->reqReconnect(message);
}
