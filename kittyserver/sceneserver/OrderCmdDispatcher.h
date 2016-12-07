#ifndef _Order_CMD_DISPATCHER
#define _Order_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "order.pb.h"

class SceneUser;

class OrderCmdHandle : public zCmdHandle
{
    public:
        OrderCmdHandle()
        {
        }

        void init(){
#define REGISTERORDER(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &OrderCmdHandle::SELFFUN));
#define REGISTERORDERSAME(PROTOCFUN) REGISTERORDER(PROTOCFUN,PROTOCFUN) 
            REGISTERORDERSAME(ReqOrderList);
            REGISTERORDERSAME(ReqFinishOrder);
            REGISTERORDERSAME(ReqFlushOrder);
            REGISTERORDERSAME(ReqClearCD);




        }
        //获取订单列表
        bool ReqOrderList(SceneUser* u, const HelloKittyMsgData::ReqOrderList* cmd);
        //完成订单
        bool ReqFinishOrder(SceneUser* u, const HelloKittyMsgData::ReqFinishOrder* cmd);
        //刷新订单
        bool ReqFlushOrder(SceneUser* u, const HelloKittyMsgData::ReqFlushOrder* cmd);
        //秒CD
        bool ReqClearCD(SceneUser* u, const HelloKittyMsgData::ReqClearCD* cmd);



};

#endif

