#include "TrainOrderCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "extractProtoMsg.h"
#include "SceneTaskManager.h"
#include "SceneCommand.h"
#include "RecordFamily.h"
#include "SceneToOtherManager.h"
#include "TimeTick.h"
#include "Misc.h"
bool TrainOrderCmdHandle::ReqLoadCarriage(SceneUser* u, const HelloKittyMsgData::ReqLoadCarriage* cmd)
{

    //如果在自己家
    HelloKittyMsgData::TrainOrderResult result = HelloKittyMsgData::TrainOrderResult_Suc;
    zMemDB* handle = NULL ;
    char buftrian[255];
    snprintf(buftrian,255,"%d",cmd->trainid());
    std::map<DWORD,DWORD> materialMap;
    HelloKittyMsgData::AckLoadCarriage ack;
    ack.set_trainid(cmd->trainid());
    ack.set_trainpos(cmd->trainpos());
    HelloKittyMsgData::vecAward *pinfo = ack.mutable_getaward();
    if(pinfo == NULL)
    {
        return false; 
    }

    if(u->getvisit() == 0)
    {
        do{
            HelloKittyMsgData::Train* ptrain = u->m_managertrain.getTrainByID(cmd->trainid());
            if(ptrain == NULL)
            {
                result = HelloKittyMsgData::TrainOrderResult_NoTrain;
                break;
            }
            //查看火车状态
            if(ptrain->state() != HelloKittyMsgData::TrainState_Load)
            {
                result = HelloKittyMsgData::TrainOrderResult_NoLoad;
                break;
            }

            //查看车厢状态
            HelloKittyMsgData::CarriageLoad* pselectload = NULL;
            for(int i= 0 ; i != ptrain->carriageload_size();i++)
            {
                HelloKittyMsgData::CarriageLoad* pload = ptrain->mutable_carriageload(i);
                if(pload && pload->pos() == cmd->trainpos())
                {
                    pselectload = pload;
                    break;

                }

            }
            if(pselectload  == NULL)
            {
                result = HelloKittyMsgData::TrainOrderResult_NoTrainPos;
                break;
            }
            if(pselectload->state() != HelloKittyMsgData::CarriageLoadState_empty)
            {
                result = HelloKittyMsgData::TrainOrderResult_TrainPosFull;
                break;
            }
            //查看自身物品数量
            materialMap[pselectload->needitem().awardtype()] = pselectload->needitem().awardval();
            if(!u->checkMaterialMap(materialMap))
            {
                result = HelloKittyMsgData::TrainOrderResult_ItemLimitForLoad;
                break;
            }
            //如果是共享，加锁
            bool bneedlock = false;
            if( ptrain->helpinfo().pos() == pselectload->pos())
            {
                if(ptrain->helpinfo().state() != HelloKittyMsgData::TrainHelpState_Req)
                {
                    result = HelloKittyMsgData::TrainOrderResult_OtherERR;
                    break;

                }
                bneedlock = true;

            }
            if(bneedlock)
            {
                handle = zMemDBPool::getMe().getMemDBHandle(u->charid);
                if(handle == NULL)
                {
                    result = HelloKittyMsgData::TrainOrderResult_OtherERR;  
                    break;
                }

                if(!handle->getLock("trianorder",u->charid,buftrian,3))
                {
                    result = HelloKittyMsgData::TrainOrderResult_TrainPosFull;
                    break;//加锁失败，那么有人已经在做了

                }
            }
            //物品扣除
            u->reduceMaterialMap(materialMap,"trainorderload  reduce");
            //--------------------
            //发装载奖品TO m_rUser.charid 
            getAwardForLoad(cmd->trainid(),*pinfo,pselectload->needitem(),u,true);
            //-------------------
            //更新状态
            u->m_managertrain.loadselftrain(ptrain,pselectload);
            //如果是共享，清除共享
            if(bneedlock)
            {
                QWORD trainhelpnum = handle->getInt("playerinfo",u->charid,"playertrainhelp");
                if(trainhelpnum > 0)
                    trainhelpnum--;
                handle->setInt("playerinfo",u->charid,"playertrainhelp",trainhelpnum);
                if(trainhelpnum == 0)
                {
                    handle->delSet("playerinfo",u->charid,"playerflag",EPLAYERFLAG_TRAINHELP);
                }
                handle->del("trianorder",u->charid,buftrian,"needitem");
                handle->delLock("trianorder",u->charid,buftrian);
            }

        }while(0);
    }
    else
    { 

        //如果在别人家
        QWORD PlayerId = u->getvisit();
        do{
            //确认共享
            handle = zMemDBPool::getMe().getMemDBHandle(PlayerId);
            if(handle == NULL)
            {
                result = HelloKittyMsgData::TrainOrderResult_OtherERR;
                break;
            }
            char buftrian[255];
            snprintf(buftrian,255,"%d",cmd->trainid());
            char tepbuf[zSocket::MAX_DATASIZE];
            DWORD size = handle->getBin("trianorder",PlayerId,buftrian,"needitem",tepbuf);
            HelloKittyMsgData::Award needitem;
            if(size > 0 )
            {
                needitem.ParseFromArray(tepbuf,size);
            }
            else
            {
                result = HelloKittyMsgData::TrainOrderResult_TrainsNoAskHelp;
                break;

            }
            //查看自身物品数量
            materialMap[needitem.awardtype()] = needitem.awardval();
            if(!u->checkMaterialMap(materialMap))
            {
                result = HelloKittyMsgData::TrainOrderResult_ItemLimitForLoad;
                break;
            }
            //如果在本服务器
            SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerId);
            //加锁
            if(!handle->getLock("trianorder",PlayerId,buftrian,3))
            {
                result = HelloKittyMsgData::TrainOrderResult_TrainPosFull;
                break;//加锁失败，那么有人已经在做了

            }
            if(user != NULL)
            {

                //物品扣除
                u->reduceMaterialMap(materialMap,"trainorderload  reduce");
                //更新状态
                user->m_managertrain.loadothertrain(u->charid,cmd->trainid());
                //奖励发送
                getAwardForLoad(cmd->trainid(),*pinfo,needitem,u,false);
            }
            else
            {
                //再次确认共享，防止其他进程清除共享
                size = handle->getBin("trianorder",PlayerId,buftrian,"needitem",tepbuf);
                if(size == 0)
                {
                    //解锁
                    handle->delLock("trianorder",PlayerId,buftrian);
                    result = HelloKittyMsgData::TrainOrderResult_TrainsNoAskHelp;
                    break;//加锁失败，那么有人已经在做了
                }
                //物品扣除
                u->reduceMaterialMap(materialMap,"trainorderload  reduce");
                //奖励发送
                getAwardForLoad(cmd->trainid(),*pinfo,needitem,u,false); 
                //发消息让其他进程改变状态，清除共享，解锁
                CMD::SCENE::t_TrainLoad sendCmd;
                DWORD SenceId = handle->getInt("playerscene",PlayerId,"sceneid");
                if(SenceId != 0)
                {
                    sendCmd.charID = PlayerId;
                    sendCmd.trainID = cmd->trainid();
                    sendCmd.friendID = u->charid;
                    std::string ret;
                    encodeMessage(&sendCmd,sizeof(sendCmd),ret);
                    if(!SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
                    {
                        Fir::logger->error(" 找不到对应server，id %d",SenceId);
                    }
                }
                else
                {
                    //解锁
                    handle->delLock("trianorder",PlayerId,buftrian);
                    result = HelloKittyMsgData::TrainOrderResult_OtherERR;
                    break;

                }

            }
        }while(0);

    }
    ack.set_result(result);
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());

    return true;
}

bool TrainOrderCmdHandle::ReqAskHelpLoadCarriage(SceneUser* u, const HelloKittyMsgData::ReqAskHelpLoadCarriage* cmd)
{
    //火车未开
    HelloKittyMsgData::TrainOrderResult result = HelloKittyMsgData::TrainOrderResult_Suc;
    do{
        HelloKittyMsgData::Train* ptrain = u->m_managertrain.getTrainByID(cmd->trainid());
        if(ptrain == NULL)
        {
            result = HelloKittyMsgData::TrainOrderResult_NoTrain;
            break;
        }
        //查看火车状态
        if(ptrain->state() != HelloKittyMsgData::TrainState_Load)
        {
            result = HelloKittyMsgData::TrainOrderResult_NoLoad;
            break;
        }
        //帮助信息初始
        if(ptrain->helpinfo().state() != HelloKittyMsgData::TrainHelpState_NONE)
        {
            result = HelloKittyMsgData::TrainOrderResult_TrainPosOtherAskHelp;
            break;
        }

        //查看车厢状态
        HelloKittyMsgData::CarriageLoad* pselectload = NULL;
        for(int i= 0 ; i != ptrain->carriageload_size();i++)
        {
            HelloKittyMsgData::CarriageLoad* pload = ptrain->mutable_carriageload(i);
            if(pload && pload->pos() == cmd->trainpos())
            {
                pselectload = pload;
                break;

            }

        }
        if(pselectload  == NULL)
        {
            result = HelloKittyMsgData::TrainOrderResult_NoTrainPos;
            break;
        }
        //车厢空
        if(pselectload->state() != HelloKittyMsgData::CarriageLoadState_empty)
        {
            result = HelloKittyMsgData::TrainOrderResult_TrainPosFull;
            break;
        }
        u->m_managertrain.sethelpinfo(ptrain,pselectload);

    }while(0);
    HelloKittyMsgData::AckAskHelpLoadCarriage ack;
    ack.set_result(result);
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());

    return true;
}

bool TrainOrderCmdHandle::ReqAnswerHelpLoadCarriage(SceneUser* u, const HelloKittyMsgData::ReqAnswerHelpLoadCarriage* cmd)
{
    //火车未开
    HelloKittyMsgData::TrainOrderResult result = HelloKittyMsgData::TrainOrderResult_Suc;
    do{
        HelloKittyMsgData::Train* ptrain = u->m_managertrain.getTrainByID(cmd->trainid());
        if(ptrain == NULL)
        {
            result = HelloKittyMsgData::TrainOrderResult_NoTrain;
            break;
        }
        //查看火车状态
        if(ptrain->state() != HelloKittyMsgData::TrainState_Load)
        {
            result = HelloKittyMsgData::TrainOrderResult_NoLoad;
            break;
        }
        //火车没有寻求帮助
        if(ptrain->helpinfo().state() != HelloKittyMsgData::TrainHelpState_Ack)
        {
            result = HelloKittyMsgData::TrainOrderResult_TrainsNoAskHelp;
            break;
        }
        u->m_managertrain.answerHelp(ptrain);

    }while(0);
    HelloKittyMsgData::AckAnswerHelpLoadCarriage ack;
    ack.set_result(result);
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool TrainOrderCmdHandle::ReqGetTrainAward(SceneUser* u, const HelloKittyMsgData::ReqGetTrainAward* cmd)
{
    HelloKittyMsgData::TrainOrderResult result = HelloKittyMsgData::TrainOrderResult_Suc;
    do{
        HelloKittyMsgData::Train* ptrain = u->m_managertrain.getTrainByID(cmd->trainid());
        if(ptrain == NULL)
        {
            result = HelloKittyMsgData::TrainOrderResult_NoTrain;
            break;
        }
        //查看火车状态
        if(ptrain->state() != HelloKittyMsgData::TrainState_Arrive)
        {
            result = HelloKittyMsgData::TrainOrderResult_TrainNoArrive;
            break;
        }
        //查看车厢状态
        HelloKittyMsgData::CarriageAward* pselectaward = NULL;
        for(int i= 0 ; i != ptrain->carriageaward_size();i++)
        {
            HelloKittyMsgData::CarriageAward* paward = ptrain->mutable_carriageaward(i);
            if(paward && paward->pos() == cmd->trainpos())
            {
                pselectaward = paward;
                break;

            }

        }
        if(pselectaward  == NULL)
        {
            result = HelloKittyMsgData::TrainOrderResult_NoTrainPos;
            break;
        }
        //奖励已经被领取
        if(pselectaward->state() == HelloKittyMsgData::CarriageAwardState_Get)
        {
            result = HelloKittyMsgData::TrainOrderResult_TrainPosGet;
            break;
        }
        //检查包裹状态
        HelloKittyMsgData::vecAward rvecAward;
        HelloKittyMsgData::Award* pawardarrive = rvecAward.add_award();
        if(pawardarrive != NULL)
        {
            *pawardarrive = pselectaward->awarditem();
            if(!u->checkPush(rvecAward))
            {
                result = HelloKittyMsgData::TrainOrderResult_PacketFull;
                break;
            }
        }
        else
        {
            result = HelloKittyMsgData::TrainOrderResult_PacketFull;
            break;
        }

        u->pushItem(rvecAward,"trainaward");
        u->m_managertrain.setTrainawardget(ptrain,pselectaward);

    }while(0);
    HelloKittyMsgData::AckGetTrainAward ack;
    ack.set_trainid(cmd->trainid());
    ack.set_trainpos(cmd->trainpos());
    ack.set_result(result);
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());

    return true;
}

bool TrainOrderCmdHandle::ReqOpenNewTrain(SceneUser* u, const HelloKittyMsgData::ReqOpenNewTrain* cmd)
{
    HelloKittyMsgData::TrainOrderResult result = HelloKittyMsgData::TrainOrderResult_Suc;
    do{
        //火车存在
        HelloKittyMsgData::Train* ptrain = u->m_managertrain.getTrainByID(cmd->trainid());
        if(ptrain != NULL)
        {
            result = HelloKittyMsgData::TrainOrderResult_TrainExist;
            break;
        }
        eParam eTrain = u->m_managertrain.getParamByTrain(cmd->trainid());
        if(eTrain == eParam_None)
        {
            result = HelloKittyMsgData::TrainOrderResult_NoTrain;
            break;
        }
        if(!u->checkLevel(ParamManager::getMe().GetSingleParam(eTrain)))
        {
            result = HelloKittyMsgData::TrainOrderResult_LowLevel;
            break;
        }
        //检查资源:
        std::vector<DWORD> vecRes = ParamManager::getMe().GetVecParam(eTrain);
        if(vecRes.size() != 2 || !u->m_store_house.addOrConsumeItem(vecRes[0],vecRes[1],"开启列车去除",false))
        {
            u->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,vecRes[0]);
            result = HelloKittyMsgData::TrainOrderResult_OpenLimitResource;
            break;
        }
        //开启新火车
        u->m_managertrain.newTrain(cmd->trainid());
    }while(0);
    if(result != HelloKittyMsgData::TrainOrderResult_OpenLimitResource)
    {
        HelloKittyMsgData::AckOpenNewTrain ack;
        ack.set_result(result);
        std::string ret;
        encodeMessage(&ack,ret);
        u->sendCmdToMe(ret.c_str(),ret.size());
    }

    return true;
}

bool TrainOrderCmdHandle::ReqClearTrainCD(SceneUser* u, const HelloKittyMsgData::ReqClearTrainCD* cmd)
{
    HelloKittyMsgData::TrainOrderResult result = HelloKittyMsgData::TrainOrderResult_Suc;
    do{
        HelloKittyMsgData::Train* ptrain = u->m_managertrain.getTrainByID(cmd->trainid());
        if(ptrain == NULL)
        {
            result = HelloKittyMsgData::TrainOrderResult_NoTrain;
            break;
        }
        //查看火车状态
        if(ptrain->state() != HelloKittyMsgData::TrainState_Running)
        {
            result = HelloKittyMsgData::TrainOrderResult_NotRuning;
            break;
        }
        //剩余时间
        DWORD needdiamand = 1 ;
        if(ptrain->isfirst())
        {
            needdiamand = ParamManager::getMe().GetSingleParam(eParam_First_TrainLoadAward);
        }
        else
        {
            DWORD NowTimer = SceneTimeTick::currentTime.sec();
            DWORD ReserverTimer = ptrain->timer() > NowTimer ? ptrain->timer() - NowTimer : 0;

            needdiamand = MiscManager::getMe().getMoneyForReduceTimer(eTimerReduce_Fifth,ReserverTimer);
        }
        std::map<DWORD,DWORD> materialMap;
        materialMap[HelloKittyMsgData::Attr_Gem] = needdiamand;//扣钻石
        if(!u->checkMaterialMap(materialMap))
        {
            result = HelloKittyMsgData::TrainOrderResult_ClearCDLimitResource;
            break;
        }
        u->reduceMaterialMap(materialMap,"trainclearcd  reduce");
        u->m_managertrain.clearTrainCD(ptrain);

    }while(0);
    HelloKittyMsgData::AckGetTrainAward ack;
    ack.set_result(result);
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}


void TrainOrderCmdHandle::getAwardForLoad(DWORD trainNo,HelloKittyMsgData::vecAward &rvecAward,const HelloKittyMsgData::Award& rneeditem,SceneUser* u,bool bself)
{

    u->m_managertrain.checkgetAwardForLoad(rvecAward,rneeditem,bself);
    u->m_managertrain.checkEffectGold(trainNo,rvecAward);
    if(bself)
    {
        u->pushItem(rvecAward,"trainload self");
    }
    else
    {
        u->pushItem(rvecAward,"trainload other");
        u->charbin.mutable_dailydata()->set_helptrain(u->charbin.dailydata().helptrain() + 1);
        TaskArgue arg(Target_Add_Source,Attr_Help_Train,Attr_Help_Train,u->charbin.dailydata().helptrain());
        u->m_taskManager.target(arg);
        u->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_helptrainorder_Num,1,"trainload other",true);

    }
    
}

