#include "OrdersystemCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "extractProtoMsg.h"
#include "SceneTaskManager.h"
#include "SceneCommand.h"
#include "Misc.h"
#include "TimeTick.h"
#include "buffer.h"
bool ordersystemCmdHandle::ReqAddOrderSystem(SceneUser* u, const HelloKittyMsgData::ReqAddOrderSystem* cmd)
{
    HelloKittyMsgData::AckAddOrderSystem ack;
    HelloKittyMsgData::OrderSystemResult result = HelloKittyMsgData::OrderSystemResult_Suc;
    do{
        if(cmd->colid() == 0)
        {
            result = HelloKittyMsgData::OrderSystemResult_Lock;
            break;
        }
        //根据人气值找到最大摊位ID
        DWORD MaxID = pb::Conf_t_BagValet::getMaxIdbyPopular(u->m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Now));
        if(cmd->colid() > MaxID)
        {
            result = HelloKittyMsgData::OrderSystemResult_Lock;
            break;
        }
        //除了第一个摊位外，判定前一个栏位是否打开
        //HelloKittyMsgData::OrderSystemItem *getOrderByID(DWORD colidID);
        HelloKittyMsgData::OrderSystemItem *pItem = u->m_managerordersystem.getOrderByID(cmd->colid());
        if(pItem !=NULL)
        {
            result = HelloKittyMsgData::OrderSystemResult_AlreadyOpen;
            break;
        }
        u->m_managerordersystem.Initcolid(cmd->colid());
        DWORD precolid = cmd->colid() - 1;
        while(precolid != 0)
        {
            HelloKittyMsgData::OrderSystemItem *preItem = u->m_managerordersystem.getOrderByID(precolid);
            if(preItem != NULL)
                break;
            u->m_managerordersystem.Initcolid(precolid);
            precolid--;
        }
        u->m_managerordersystem.Initcolid(cmd->colid());
    }while(0);
    ack.set_result(result);
    std::string ret; 
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool ordersystemCmdHandle::ReqRunOrderSystem(SceneUser* u, const HelloKittyMsgData::ReqRunOrderSystem* cmd)
{
    HelloKittyMsgData::AckRunOrderSystem ack;
    HelloKittyMsgData::OrderSystemResult result = HelloKittyMsgData::OrderSystemResult_Suc;
    do{
        HelloKittyMsgData::OrderSystemItem *pItem = u->m_managerordersystem.getOrderByID(cmd->colid());
        if(pItem == NULL)
        {
            result = HelloKittyMsgData::OrderSystemResult_NoOpen;
            break;
        }
        if(pItem->state() != HelloKittyMsgData::OrderSystemState_Empty)
        {
            result = HelloKittyMsgData::OrderSystemResult_NotEmpty;
            break;
        }
        const pb::Conf_t_OrderGoods *pgoods = tbx::OrderGoods().get_base(cmd->itemid());
        if(pgoods == NULL)
        {
            result = HelloKittyMsgData::OrderSystemResult_ItemIdErr;
            break;
        }
        if(!u->checkLevel(pgoods->OrderGoods->level()))
        {
            result = HelloKittyMsgData::OrderSystemResult_ItemUnLock;
            break;
        }
        //资源扣除
        std::map<DWORD,DWORD> materialMap;
        materialMap[HelloKittyMsgData::Attr_Gold] = pgoods->OrderGoods->gold();
        if(!u->reduceMaterialMap(materialMap,"ordersystemRun  reduce"))
        {
            result = HelloKittyMsgData::OrderSystemResult_MoneyLimitForOrdering;
            break;
        }
        //设置
        u->m_managerordersystem.Runcolid(pItem,cmd->itemid(),getOrderTimer(u,pgoods->OrderGoods->time()*60));
    
        u->charbin.mutable_dailydata()->set_orderaccept(u->charbin.dailydata().orderaccept() + 1);
        TaskArgue arg(Target_Add_Source,Attr_Order_Accept_Num,Attr_Order_Accept_Num,u->charbin.dailydata().orderaccept());
        u->m_taskManager.target(arg);

    }while(0);
    ack.set_result(result);
    std::string ret; 
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

DWORD ordersystemCmdHandle::getOrdernum(SceneUser* u)
{
    DWORD nowNum = 1;
    std::vector<DWORD> bufferVec;
    getBufferList(u,bufferVec,BufferID_Add_Order_Goods);
    for(auto iter = bufferVec.begin();iter != bufferVec.end();++iter)
    {
        const pb::Conf_t_buffer *bufferConf = tbx::buffer().get_base(*iter);
        if(bufferConf == NULL)
            continue;
        nowNum += bufferConf->buffer->count();

    }
    return nowNum;

}
DWORD ordersystemCmdHandle::getOrderTimer(SceneUser* u,DWORD oldTimer)
{
    DWORD nowTimer = oldTimer;
    std::vector<DWORD> bufferVec;
    getBufferList(u,bufferVec,BufferID_Reduce_Order_CD);
    for(auto iter = bufferVec.begin();iter != bufferVec.end();++iter)
    {
        const pb::Conf_t_buffer *bufferConf = tbx::buffer().get_base(*iter);
        if(bufferConf == NULL)
            continue;
        if(bufferConf->buffer->ratio() > 0 && bufferConf->buffer->ratio() <= 100)
        {
            nowTimer = oldTimer*(100-bufferConf->buffer->ratio())/(float)(100);
        }

    }
    return nowTimer;


}
bool ordersystemCmdHandle::ReqFinishOrderSystem(SceneUser* u, const HelloKittyMsgData::ReqFinishOrderSystem* cmd)
{
    HelloKittyMsgData::AckFinishOrderSystem ack;
    HelloKittyMsgData::OrderSystemResult result = HelloKittyMsgData::OrderSystemResult_Suc;
    do{
        HelloKittyMsgData::OrderSystemItem *pItem = u->m_managerordersystem.getOrderByID(cmd->colid());
        if(pItem == NULL)
        {
            result = HelloKittyMsgData::OrderSystemResult_NoOpen;
            break;
        }
        if(pItem->state() != HelloKittyMsgData::OrderSystemState_Finish)
        {
            return true;
            result = HelloKittyMsgData::OrderSystemResult_NotArrive;
            break;
        }
        const pb::Conf_t_OrderGoods *pgoods = tbx::OrderGoods().get_base(pItem->itemid());
        if(pgoods == NULL)
        {
            result = HelloKittyMsgData::OrderSystemResult_ItemIdErr;
            break;
        }
        //检查包裹状态
        HelloKittyMsgData::vecAward rvecAward;
        HelloKittyMsgData::Award* pawardarrive = rvecAward.add_award();
        if(pawardarrive != NULL)
        {
            pawardarrive->set_awardtype(pgoods->OrderGoods->itemid());
            pawardarrive->set_awardval(1);
            if(!u->checkPush(rvecAward))
            {
                result = HelloKittyMsgData::OrderSystemResult_NoPaket;
                break;
            }
            pawardarrive->set_awardval(getOrdernum(u));

        }
        else
        {
            result = HelloKittyMsgData::OrderSystemResult_NoPaket;
            break;
        }
        pawardarrive = rvecAward.add_award();
        if(pawardarrive == NULL)
        {
            result = HelloKittyMsgData::OrderSystemResult_NoPaket;
            break;

        }
        else
        {
            pawardarrive->set_awardtype(HelloKittyMsgData::Attr_Exp);
            pawardarrive->set_awardval(pgoods->OrderGoods->itemexp());
        }
        u->pushItemWithoutCheck(rvecAward,"add for ReqFinishOrderSystem");

        //设置
        u->m_managerordersystem.Initcolid(cmd->colid());
        u->charbin.mutable_dailydata()->set_ordervalue(u->charbin.dailydata().ordervalue() + 1);
        TaskArgue arg(Target_Add_Source,Attr_Order_Num,Attr_Order_Num,u->charbin.dailydata().ordervalue());
        u->m_taskManager.target(arg);
        u->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Purchase_All,1);
        u->opItemResourMap(Item_SysOrder,pgoods->OrderGoods->itemid(),1,true);
    }while(0);
    ack.set_result(result);
    std::string ret; 
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool ordersystemCmdHandle::ReqClearOrderSystemCD(SceneUser* u, const HelloKittyMsgData::ReqClearOrderSystemCD* cmd)
{
    HelloKittyMsgData::AckClearOrderSystemCD ack;
    HelloKittyMsgData::OrderSystemResult result = HelloKittyMsgData::OrderSystemResult_Suc;
    do{
        HelloKittyMsgData::OrderSystemItem *pItem = u->m_managerordersystem.getOrderByID(cmd->colid());
        if(pItem == NULL)
        {
            result = HelloKittyMsgData::OrderSystemResult_NoOpen;
            break;
        }
        if(pItem->state() != HelloKittyMsgData::OrderSystemState_Running)
        {
            result = HelloKittyMsgData::OrderSystemResult_NotWaitState;
            break;
        }
        //剩余时间
        DWORD NowTimer = SceneTimeTick::currentTime.sec();
        DWORD ReserverTimer = pItem->timer() > NowTimer ? pItem->timer() - NowTimer : 0;

        DWORD needdiamand = MiscManager::getMe().getMoneyForReduceTimer(eTimerReduce_First,ReserverTimer);
        std::map<DWORD,DWORD> materialMap;
        materialMap[HelloKittyMsgData::Attr_Gem] = needdiamand;//扣钻石
        if(!u->reduceMaterialMap(materialMap,"ordersytemclearcd  reduce"))
        {
            result = HelloKittyMsgData::OrderSystemResult_MoneyLimitForClearCD;
            break;

        }
        //设置
        u->m_managerordersystem.ClearOrderSystemCD(pItem);

    }while(0);
    ack.set_result(result);
    std::string ret; 
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

