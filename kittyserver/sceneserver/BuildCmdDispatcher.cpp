#include "TradeCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "TradeCommand.h"
#include "serialize.pb.h"
#include "extractProtoMsg.h"
#include "buildItemProduce.h"
#include "buildTypeProduceGold.h"
#include "buildItemComposite.h"
#include "TimeTick.h"

bool TradeCmdHandle::requireAllBuild(SceneUser* user, const HelloKittyMsgData::ReqAllBuild* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.flushAllBuild();
}

bool TradeCmdHandle::requireOneBuild(SceneUser* user, const HelloKittyMsgData::ReqOneBuild* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.flushOneBuild(cmd->tempid());
}

bool TradeCmdHandle::upGrade(SceneUser* user, const HelloKittyMsgData::ReqUpGrade* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.upBuildGrade(cmd->tempid());
}
 

bool TradeCmdHandle::buildUpGrade(SceneUser* user, const HelloKittyMsgData::ReqBuildUpGrade* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    if(cmd->tempid() == eParam_OpenFirstTrain || cmd->tempid() == eParam_OpenSecondTrain || cmd->tempid() == eParam_OpenThirdTrain)
    {
        //火车升级
        user->m_managertrain.upGrade(cmd->tempid(),cmd->effectid());
        return true;
    }
    return user->m_buildManager.upGrade(cmd->tempid(),cmd->effectid());
}

bool TradeCmdHandle::build(SceneUser* user, const HelloKittyMsgData::ReqBuildBuilding* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.build(cmd->type(),1,cmd->moveflg(),cmd->point(),SceneTimeTick::currentTime.sec());
}

bool TradeCmdHandle::move(SceneUser* user, const HelloKittyMsgData::ReqBuildMovePlace* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.move(cmd);
}

bool TradeCmdHandle::pickUpBuild(SceneUser* user, const HelloKittyMsgData::ReqPickUpBuid* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.pickBuildInWare(cmd->tempid());
}

bool TradeCmdHandle::pickoutBuild(SceneUser* user, const HelloKittyMsgData::ReqPickOutBuid* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.pickoutBuild(cmd);
}

bool TradeCmdHandle::reqWorker(SceneUser* user, const HelloKittyMsgData::ReqWorker* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.flushWorker();
}

bool TradeCmdHandle::reqResetWorker(SceneUser* user, const HelloKittyMsgData::ReqResetWorker* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.resetWorkerCD(cmd->workerid());
}

bool TradeCmdHandle::reqClickReward(SceneUser* user, const HelloKittyMsgData::ReqClickRewardBuid* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    if(!temp || !temp->isTypeBuild(Build_Type_Gold_Produce))
    {
        return false;
    }
    BuildTypeProduceGold *buildType = dynamic_cast<BuildTypeProduceGold*>(temp);
    if(!buildType)
    {
        return false;
    }
    if(buildType->clickReward())
    {
        buildType->updateProduce();
    }
    return true;
}

bool TradeCmdHandle::reqRecycle(SceneUser* user, const HelloKittyMsgData::ReqRecycleItem* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.recycle(cmd);
}

bool TradeCmdHandle::reqFlushProduce(SceneUser* user, const HelloKittyMsgData::ReqBuildProduce* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.flushBuildProduce(cmd);
}

bool TradeCmdHandle::reqProduceCell(SceneUser* user, const HelloKittyMsgData::ReqProduceCell* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    if(!temp)
    {
        return false;
    }
    BuildTypeProduceItem *buildType = dynamic_cast<BuildTypeProduceItem*>(temp);
    if(!buildType)
    {
        return false;
    }
    buildType->updateProduceCell(cmd->placeid());
    return true;
}

bool TradeCmdHandle::reqCompositeCell(SceneUser* user, const HelloKittyMsgData::ReqCompositeCell* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    if(!temp)
    {
        return false;
    }
    BuildTypeCompositeItem*buildType = dynamic_cast<BuildTypeCompositeItem*>(temp);
    if(!buildType)
    {
        return false;
    }
    buildType->update(DWORD(0));
    return true;
}


bool TradeCmdHandle::reqProduceCellWork(SceneUser* user, const HelloKittyMsgData::ReqProduceCellWork* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    if(!temp)
    {
        return false;
    }
    BuildTypeProduceItem *buildType = dynamic_cast<BuildTypeProduceItem*>(temp);
    if(!buildType)
    {
        return false;
    }
    return buildType->workProduce(cmd->placeid(),cmd->produceid());
}

bool TradeCmdHandle::reqProduceOp(SceneUser* user, const HelloKittyMsgData::ReqProduceOp* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    if(!temp)
    {
        return false;
    }
    BuildTypeProduceItem *buildType = dynamic_cast<BuildTypeProduceItem*>(temp);
    if(!buildType)
    {
        return false;
    }
    return buildType->OpBTPType(cmd->optype(),cmd->placeid());
}

bool TradeCmdHandle::reqCompositeOp(SceneUser* user, const HelloKittyMsgData::ReqCompositeOp* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    if(!temp)
    {
        return false;
    }
    BuildTypeCompositeItem*buildType = dynamic_cast<BuildTypeCompositeItem*>(temp);
    if(!buildType)
    {
        return false;
    }
    return buildType->OpBTPType(cmd->optype(),cmd->placeid());
}

bool TradeCmdHandle::reqOpCard(SceneUser* user, const HelloKittyMsgData::ReqOpCard* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    if(!temp)
    {
        return false;
    }
    if(!(temp->isTypeBuild(Build_Type_Item_Produce) || temp->isTypeBuild(Build_Type_Gold_Produce) || temp->isTypeBuild(Build_Type_Item_Composite)))
    {
        return false;
    }
    if(cmd->optype() == HelloKittyMsgData::BCO_Req)
    {
        temp->updateCard();
    }
    else 
    {
        temp->clearCard();
    }
    return true;
}

bool TradeCmdHandle::reqUseCard(SceneUser* user, const HelloKittyMsgData::ReqUserCard* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    if(!temp)
    {
        return false;
    }
    //先放开
    if(user->m_buildManager.findCard(temp->getID()))
    {
        return user->opErrorReturn(HelloKittyMsgData::Card_Is_Used);
    }
    if(!(temp->isTypeBuild(Build_Type_Item_Produce) || temp->isTypeBuild(Build_Type_Gold_Produce) || temp->isTypeBuild(Build_Type_Item_Composite)))
    {
        return user->opErrorReturn(HelloKittyMsgData::Card_Is_Not_Suit);
    }
    return temp->useCard(cmd->item());
}

bool TradeCmdHandle::reqBuildRoad(SceneUser* user,const HelloKittyMsgData::ReqBuildRoad *cmd)
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.buildRoad(cmd);
}

bool TradeCmdHandle::reqClearRoad(SceneUser* user,const HelloKittyMsgData::ReqClearRoad*cmd)
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.clearRoad(cmd);
}

bool TradeCmdHandle::reqAllConstructBuild(SceneUser* user,const HelloKittyMsgData::ReqAllConstructBuild *cmd)
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->m_buildManager.responseBuildProduceItem();
}

bool TradeCmdHandle::reqUnLockBuild(SceneUser* user,const HelloKittyMsgData::ReqUnLockBuild *cmd)
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->unLock(cmd->id());
}

bool TradeCmdHandle::reqClickActiveBuild(SceneUser* user,const HelloKittyMsgData::ReqClickActiveBuild *cmd)
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    return temp ? temp->clickACtive() : false;
}

bool TradeCmdHandle::reqParseBuildCD(SceneUser* user,const HelloKittyMsgData::ReqParseBuildCD *cmd)
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    return temp ? temp->parseBuildCD() : false;
}

bool TradeCmdHandle::reqCompositeWork(SceneUser* user, const HelloKittyMsgData::ReqCompositeWork* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    BuildBase *temp = user->m_buildManager.getBuild(cmd->tempid());
    if(!temp)
    {
        return false;
    }
    BuildTypeCompositeItem *buildType = dynamic_cast<BuildTypeCompositeItem*>(temp);
    if(!buildType)
    {
        return false;
    }
    return buildType->workComposite(cmd->produceid());
}

bool TradeCmdHandle::reqSellWareHouseBuild(SceneUser* user, const HelloKittyMsgData::ReqSellWareHouseBuild* cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    user->m_buildManager.reqSellWareHouseBuild(cmd);
    return  false;
}
