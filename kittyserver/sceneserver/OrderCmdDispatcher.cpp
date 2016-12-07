#include "OrderCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "extractProtoMsg.h"
#include "SceneTaskManager.h"


//获取订单列表
bool OrderCmdHandle::ReqOrderList(SceneUser* u, const HelloKittyMsgData::ReqOrderList* cmd)
{
    HelloKittyMsgData::AckOrderList ack;
    u->m_orderManager.getOrderList(ack);
    return true;
}
//完成订单
bool OrderCmdHandle::ReqFinishOrder(SceneUser* u, const HelloKittyMsgData::ReqFinishOrder* cmd)
{
    HelloKittyMsgData::AckFinishOrder ack;
    u->m_orderManager.ReqFinishOrder(ack,cmd->colid(),cmd->finishtype() > 0);
    return true;
}
//刷新订单
bool OrderCmdHandle::ReqFlushOrder(SceneUser* u, const HelloKittyMsgData::ReqFlushOrder* cmd)
{
    HelloKittyMsgData::AckFlushOrder ack;
    u->m_orderManager.ReqFlushOrder(ack,cmd->colid());
    return true;
}
//秒CD
bool OrderCmdHandle::ReqClearCD(SceneUser* u, const HelloKittyMsgData::ReqClearCD* cmd)
{
    HelloKittyMsgData::AckClearCD ack;
    u->m_orderManager.ReqClearCD(ack,cmd->colid());
    return true;
}


