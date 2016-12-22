#ifndef _TRADE_CMD_DISPATCHER
#define _TRADE_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "TradeCommand.h"
#include "SceneTask.h"
#include "serialize.pb.h"
#include "item.pb.h"
#include "build.pb.h"
#include "warehouse.pb.h"
#include "login.pb.h"
#include "task.pb.h"
#include "atlas.pb.h"
#include "enterkitty.pb.h"
#include "carnival.pb.h"
#include "event.pb.h"
#include "friend.pb.h"
#include "email.pb.h"
#include "produce.pb.h"
#include "usecardbuild.pb.h"
#include "paper.pb.h"
#include "divine.pb.h"
#include "burstevent.pb.h"
#include "littergame.pb.h"
#include "auction.pb.h"
#include "giftpackage.pb.h"
#include "rank.pb.h"
#include "active.pb.h"
#include "toy.pb.h"
#include "room.pb.h"
#include "composite.pb.h"
#include "playeractive.pb.h"

class SceneUser;

class TradeCmdHandle : public zCmdHandle
{
    public:
        TradeCmdHandle()
        {
        }

        void init()
        {
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAddItem>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAddItem>::ProtoFunction(this,&TradeCmdHandle::addItemInWareHouse));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqStoreItem>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqStoreItem>::ProtoFunction(this,&TradeCmdHandle::requireStoreInfo));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSallPutItem>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSallPutItem>::ProtoFunction(this,&TradeCmdHandle::requireSallAddItem));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSellPaper>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSellPaper>::ProtoFunction(this,&TradeCmdHandle::requireSellPaper));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAdvertise>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAdvertise>::ProtoFunction(this,&TradeCmdHandle::advertise));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqUseItem>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqUseItem>::ProtoFunction(this,&TradeCmdHandle::useItem));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBuildUpGrade>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBuildUpGrade>::ProtoFunction(this,&TradeCmdHandle::buildUpGrade));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqUpGrade>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqUpGrade>::ProtoFunction(this,&TradeCmdHandle::upGrade));


            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBuildMovePlace>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBuildMovePlace>::ProtoFunction(this,&TradeCmdHandle::move));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqPickOutBuid>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqPickOutBuid>::ProtoFunction(this,&TradeCmdHandle::pickoutBuild));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBuildBuilding>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBuildBuilding>::ProtoFunction(this,&TradeCmdHandle::build));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAtlas>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAtlas>::ProtoFunction(this,&TradeCmdHandle::reqAtlas));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAllAchieve>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAllAchieve>::ProtoFunction(this,&TradeCmdHandle::reqAchieve));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSubmitAchieve>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSubmitAchieve>::ProtoFunction(this,&TradeCmdHandle::commitAchieve));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqWorker>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqWorker>::ProtoFunction(this,&TradeCmdHandle::reqWorker));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSallSystem>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSallSystem>::ProtoFunction(this,&TradeCmdHandle::reqSallSystem));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqResetWorker>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqResetWorker>::ProtoFunction(this,&TradeCmdHandle::reqResetWorker));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqclearSweetBox>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqclearSweetBox>::ProtoFunction(this,&TradeCmdHandle::reqclearSweetBox));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqClickRewardBuid>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqClickRewardBuid>::ProtoFunction(this,&TradeCmdHandle::reqClickReward));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpCell>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpCell>::ProtoFunction(this,&TradeCmdHandle::reqOpCell));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqRecycleItem>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqRecycleItem>::ProtoFunction(this,&TradeCmdHandle::reqRecycle));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBuildProduce>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBuildProduce>::ProtoFunction(this,&TradeCmdHandle::reqFlushProduce));
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAllBuild>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAllBuild>::ProtoFunction(this,&TradeCmdHandle::requireAllBuild));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOneBuild>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOneBuild>::ProtoFunction(this,&TradeCmdHandle::requireOneBuild));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqPickUpBuid>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqPickUpBuid>::ProtoFunction(this,&TradeCmdHandle::pickUpBuild));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpenArea>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpenArea>::ProtoFunction(this,&TradeCmdHandle::reqOpenArea));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqPayGrid>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqPayGrid>::ProtoFunction(this,&TradeCmdHandle::reqPayGrid));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAllTask>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAllTask>::ProtoFunction(this,&TradeCmdHandle::reqTask));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSubmitTask>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSubmitTask>::ProtoFunction(this,&TradeCmdHandle::commitTask));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAtlas>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAtlas>::ProtoFunction(this,&TradeCmdHandle::reqAtlas));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAllAchieve>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAllAchieve>::ProtoFunction(this,&TradeCmdHandle::reqAchieve));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSubmitAchieve>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSubmitAchieve>::ProtoFunction(this,&TradeCmdHandle::commitAchieve));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqWorker>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqWorker>::ProtoFunction(this,&TradeCmdHandle::reqWorker));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpenCarnical>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpenCarnical>::ProtoFunction(this,&TradeCmdHandle::reqOpenCarnical));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSallSystem>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSallSystem>::ProtoFunction(this,&TradeCmdHandle::reqSallSystem));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqResetWorker>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqResetWorker>::ProtoFunction(this,&TradeCmdHandle::reqResetWorker));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqclearSweetBox>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqclearSweetBox>::ProtoFunction(this,&TradeCmdHandle::reqclearSweetBox));

#define REGISTER(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &TradeCmdHandle::SELFFUN));
#define REGISTERSAME(PROTOCFUN) REGISTER(PROTOCFUN,PROTOCFUN) 
            REGISTERSAME(ReqAddFriend);
            REGISTERSAME(ReqKickFriend); 
            REGISTERSAME(ReqRelationList); 
            REGISTERSAME(ReqEnterGarden);
            REGISTERSAME(opBuilding);
            REGISTERSAME(ReqPurchase);
            REGISTERSAME(ReqGetOnePerson);


            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqEmail>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqEmail>::ProtoFunction(this,&TradeCmdHandle::reqEmail));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSendEmail>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSendEmail>::ProtoFunction(this,&TradeCmdHandle::reqSendEmail));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpEmail>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpEmail>::ProtoFunction(this,&TradeCmdHandle::reqOpEmail));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqProduceCell>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqProduceCell>::ProtoFunction(this,&TradeCmdHandle::reqProduceCell));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqCompositeCell>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqCompositeCell>::ProtoFunction(this,&TradeCmdHandle::reqCompositeCell));


            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqProduceCellWork>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqProduceCellWork>::ProtoFunction(this,&TradeCmdHandle::reqProduceCellWork));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqProduceOp>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqProduceOp>::ProtoFunction(this,&TradeCmdHandle::reqProduceOp));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqCompositeOp>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqCompositeOp>::ProtoFunction(this,&TradeCmdHandle::reqCompositeOp));


            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpCard>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpCard>::ProtoFunction(this,&TradeCmdHandle::reqOpCard));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqUserCard>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqUserCard>::ProtoFunction(this,&TradeCmdHandle::reqUseCard));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBuildRoad>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBuildRoad>::ProtoFunction(this,&TradeCmdHandle::reqBuildRoad));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqClearRoad>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqClearRoad>::ProtoFunction(this,&TradeCmdHandle::reqClearRoad));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqDress>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqDress>::ProtoFunction(this,&TradeCmdHandle::reqDress));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpDress>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpDress>::ProtoFunction(this,&TradeCmdHandle::reqOpDress));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqPaper>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqPaper>::ProtoFunction(this,&TradeCmdHandle::reqPaper));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpPaper>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpPaper>::ProtoFunction(this,&TradeCmdHandle::reqOpPaper));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqDivineInfo>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqDivineInfo>::ProtoFunction(this,&TradeCmdHandle::reqDivineInfo));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqDivine>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqDivine>::ProtoFunction(this,&TradeCmdHandle::reqDivine));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqDivineVerify>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqDivineVerify>::ProtoFunction(this,&TradeCmdHandle::reqDivineVerify));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpenCarnical>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpenCarnical>::ProtoFunction(this,&TradeCmdHandle::reqOpenCarnical));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqClickCarnicalBox>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqClickCarnicalBox>::ProtoFunction(this,&TradeCmdHandle::reqClickCarnival));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBuyCarnicalBox>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBuyCarnicalBox>::ProtoFunction(this,&TradeCmdHandle::reqBuyCarnival));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqPurchaseItem>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqPurchaseItem>::ProtoFunction(this,&TradeCmdHandle::reqPurchaseItem));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBurstEvent>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBurstEvent>::ProtoFunction(this,&TradeCmdHandle::reqBurstEvent));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpBurstEvent>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpBurstEvent>::ProtoFunction(this,&TradeCmdHandle::reqOpBurstEvent));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqDivineNotice>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqDivineNotice>::ProtoFunction(this,&TradeCmdHandle::reqDivineFristNotice));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBeginStar>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBeginStar>::ProtoFunction(this,&TradeCmdHandle::reqBeginStar));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpCenter>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpCenter>::ProtoFunction(this,&TradeCmdHandle::reqOpCenter));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpRoom>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpRoom>::ProtoFunction(this,&TradeCmdHandle::reqOpRoom));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBid>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBid>::ProtoFunction(this,&TradeCmdHandle::reqBid));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAutoBid>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAutoBid>::ProtoFunction(this,&TradeCmdHandle::reqAutoBid));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAllConstructBuild>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAllConstructBuild>::ProtoFunction(this,&TradeCmdHandle::reqAllConstructBuild));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBuyNormalGift>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBuyNormalGift>::ProtoFunction(this,&TradeCmdHandle::reqParchaseNormalGift));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpGift>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpGift>::ProtoFunction(this,&TradeCmdHandle::reqGiftOp));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqUpdate>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqUpdate>::ProtoFunction(this,&TradeCmdHandle::reqUpdate));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAddress>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAddress>::ProtoFunction(this,&TradeCmdHandle::reqAddress));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqChangeAddress>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqChangeAddress>::ProtoFunction(this,&TradeCmdHandle::reqChangeAddress));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqCashGift>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqCashGift>::ProtoFunction(this,&TradeCmdHandle::reqCashGift));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqCommitGift>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqCommitGift>::ProtoFunction(this,&TradeCmdHandle::reqCommitGift));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBeginSlot>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBeginSlot>::ProtoFunction(this,&TradeCmdHandle::reqBeginSlot));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBeginMacro>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBeginMacro>::ProtoFunction(this,&TradeCmdHandle::reqBeginMacro));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqRank>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqRank>::ProtoFunction(this,&TradeCmdHandle::reqRank));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBeginSuShi>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBeginSuShi>::ProtoFunction(this,&TradeCmdHandle::reqBeginSuShi));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqCommitStep>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqCommitStep>::ProtoFunction(this,&TradeCmdHandle::reqCommitStepData));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqUnLockBuild>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqUnLockBuild>::ProtoFunction(this,&TradeCmdHandle::reqUnLockBuild));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqStarCommitStep>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqStarCommitStep>::ProtoFunction(this,&TradeCmdHandle::reqCommitStar));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqJoinActive>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqJoinActive>::ProtoFunction(this,&TradeCmdHandle::reqJoinActive));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpToy>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpToy>::ProtoFunction(this,&TradeCmdHandle::reqToy));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAckActiveInfo>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAckActiveInfo>::ProtoFunction(this,&TradeCmdHandle::reqActiveInfo));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAdvertiseTime>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAdvertiseTime>::ProtoFunction(this,&TradeCmdHandle::reqAdvertiseTime));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqPurchaseAdvertiseCD>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqPurchaseAdvertiseCD>::ProtoFunction(this,&TradeCmdHandle::reqPurchaseAdvertiseCD));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqModifyPresent>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqModifyPresent>::ProtoFunction(this,&TradeCmdHandle::reqModifyPresent));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqModifyVoice>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqModifyVoice>::ProtoFunction(this,&TradeCmdHandle::reqModifyVoice));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqEnterRoom>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqEnterRoom>::ProtoFunction(this,&TradeCmdHandle::reqEnterRoom));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqNoCenter>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqNoCenter>::ProtoFunction(this,&TradeCmdHandle::reqNoCenter));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqClickActiveBuild>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqClickActiveBuild>::ProtoFunction(this,&TradeCmdHandle::reqClickActiveBuild));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqParseBuildCD>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqParseBuildCD>::ProtoFunction(this,&TradeCmdHandle::reqParseBuildCD));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqParseAreaGridCD>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqParseAreaGridCD>::ProtoFunction(this,&TradeCmdHandle::reqParseAreaGridCD));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqPhyCondInfo>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqPhyCondInfo>::ProtoFunction(this,&TradeCmdHandle::reqPhyCondInfo));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqCompositeWork>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqCompositeWork>::ProtoFunction(this,&TradeCmdHandle::reqCompositeWork));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqExchangeGiftNum>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqExchangeGiftNum>::ProtoFunction(this,&TradeCmdHandle::reqExchangeGiftNum));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSendFlower>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSendFlower>::ProtoFunction(this,&TradeCmdHandle::reqSendFlower));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSendVirtualGift>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSendVirtualGift>::ProtoFunction(this,&TradeCmdHandle::reqSendVirtualGift));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqStartGame>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqStartGame>::ProtoFunction(this,&TradeCmdHandle::reqStartGame));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqJoinStartGame>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqJoinStartGame>::ProtoFunction(this,&TradeCmdHandle::reqJoinStartGame));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqCancelStar>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqCancelStar>::ProtoFunction(this,&TradeCmdHandle::reqCancelStartGame));

           SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqContribute>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqContribute>::ProtoFunction(this,&TradeCmdHandle::reqContribute));
            
           SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSyncStar>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSyncStar>::ProtoFunction(this,&TradeCmdHandle::reqSyncStar));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqReadyGame>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqReadyGame>::ProtoFunction(this,&TradeCmdHandle::reqReadyGame));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOpLike>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOpLike>::ProtoFunction(this,&TradeCmdHandle::reqOpLike));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqPersonInalInfo>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqPersonInalInfo>::ProtoFunction(this,&TradeCmdHandle::reqPersonInalInfo));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqRoomAndPersonalInfo>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqRoomAndPersonalInfo>::ProtoFunction(this,&TradeCmdHandle::reqRoomAndPersonalInfo));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqEditPersonInalInfo>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqEditPersonInalInfo>::ProtoFunction(this,&TradeCmdHandle::reqEditPersonInalInfo));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqNeonMark>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqNeonMark>::ProtoFunction(this,&TradeCmdHandle::reqNeonMark));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqAddPicture>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqAddPicture>::ProtoFunction(this,&TradeCmdHandle::reqAddPicture));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqMovePicture>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqMovePicture>::ProtoFunction(this,&TradeCmdHandle::reqMovePicture));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSetPictureHead>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSetPictureHead>::ProtoFunction(this,&TradeCmdHandle::reqSetPictureHead));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqSellWareHouseBuild>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqSellWareHouseBuild>::ProtoFunction(this,&TradeCmdHandle::reqSellWareHouseBuild));
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqOutRoom>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqOutRoom>::ProtoFunction(this,&TradeCmdHandle::reqOutRoom));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqBuyloginLast>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqBuyloginLast>::ProtoFunction(this,&TradeCmdHandle::reqBuyloginLast));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqViewWechat>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqViewWechat>::ProtoFunction(this,&TradeCmdHandle::reqViewWechat));

            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::ReqRewardActiveCode>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqRewardActiveCode>::ProtoFunction(this,&TradeCmdHandle::reqRewardActiveCode));


        }




        //.......................道具解析函数开始......................

        //使用道具
        bool useItem(SceneUser* u, const HelloKittyMsgData::ReqUseItem* cmd);
        bool reqContribute(SceneUser* user,const HelloKittyMsgData::ReqContribute *message);

        //.......................道具解析函数结束...................... 


        //.......................仓库解析函数开始...................... 

        //请求仓库添加道具
        bool addItemInWareHouse(SceneUser* u, const HelloKittyMsgData::ReqAddItem* cmd);
        //请求道具容器信息
        bool requireStoreInfo(SceneUser* u, const HelloKittyMsgData::ReqStoreItem* cmd);
        //购买仓库格子
        bool reqPayGrid(SceneUser* u, const HelloKittyMsgData::ReqPayGrid* cmd);
        //.......................仓库解析函数结束...................... 


        //.......................摊位解析函数开始...................... 

        //请求摊位添加道具
        bool requireSallAddItem(SceneUser* u, const HelloKittyMsgData::ReqSallPutItem *cmd);
        //请求寄售报纸信息
        bool requireSellPaper(SceneUser* u, const HelloKittyMsgData::ReqSellPaper *cmd);
        //打广告
        bool advertise(SceneUser* u, const HelloKittyMsgData::ReqAdvertise *cmd);
        //出售道具给系统
        bool reqSallSystem(SceneUser* u, const HelloKittyMsgData::ReqSallSystem *cmd);
        //点击摊位单元格操作
        bool reqOpCell(SceneUser* u, const HelloKittyMsgData::ReqOpCell *cmd);
        //请求打广告时间
        bool reqAdvertiseTime(SceneUser* user,const HelloKittyMsgData::ReqAdvertiseTime* message);
        //购买cd
        bool reqPurchaseAdvertiseCD(SceneUser* user,const HelloKittyMsgData::ReqPurchaseAdvertiseCD *message);
        //.......................摊位解析函数结束...................... 


        //.......................建筑解析函数开始......................

        //升级(升星)
        bool buildUpGrade(SceneUser* user, const HelloKittyMsgData::ReqBuildUpGrade* cmd);

        //升级
        bool upGrade(SceneUser* user, const HelloKittyMsgData::ReqUpGrade* cmd);

        //建筑
        bool build(SceneUser* user, const HelloKittyMsgData::ReqBuildBuilding* cmd);
        //移动
        bool move(SceneUser* user, const HelloKittyMsgData::ReqBuildMovePlace* cmd);
        //刷新所有建筑
        bool requireAllBuild(SceneUser* user, const HelloKittyMsgData::ReqAllBuild* cmd);
        //刷新单个建筑
        bool requireOneBuild(SceneUser* user, const HelloKittyMsgData::ReqOneBuild* cmd);
        //收起建筑
        bool pickUpBuild(SceneUser* user, const HelloKittyMsgData::ReqPickUpBuid* cmd);
        //拿出仓库建筑
        bool pickoutBuild(SceneUser* user, const HelloKittyMsgData::ReqPickOutBuid* cmd);
        //请求工人数据
        bool reqWorker(SceneUser* user, const HelloKittyMsgData::ReqWorker* cmd);
        //重置工人cd
        bool reqResetWorker(SceneUser* user, const HelloKittyMsgData::ReqResetWorker* cmd);
        //请求点击建筑产出
        bool reqClickReward(SceneUser* user, const HelloKittyMsgData::ReqClickRewardBuid* cmd);
        //请求回收站回收道具
        bool reqRecycle(SceneUser* user, const HelloKittyMsgData::ReqRecycleItem* cmd);
        //请求道具产出信息
        bool reqFlushProduce(SceneUser* user, const HelloKittyMsgData::ReqBuildProduce* cmd);
        //请求生产建筑功能槽
        bool reqProduceCell(SceneUser* user, const HelloKittyMsgData::ReqProduceCell* cmd);
        //请求合成建筑功能槽
        bool reqCompositeCell(SceneUser* user, const HelloKittyMsgData::ReqCompositeCell* cmd);
        //请求工作槽开始工作
        bool reqProduceCellWork(SceneUser* user, const HelloKittyMsgData::ReqProduceCellWork* cmd);
        //请求对工作槽操作
        bool reqProduceOp(SceneUser* user, const HelloKittyMsgData::ReqProduceOp* cmd);
        //请求操作合成建筑
        bool reqCompositeOp(SceneUser* user, const HelloKittyMsgData::ReqCompositeOp* cmd);
        //请求建筑卡牌操作
        bool reqOpCard(SceneUser* user, const HelloKittyMsgData::ReqOpCard* cmd);
        //使用建筑卡牌
        bool reqUseCard(SceneUser* user, const HelloKittyMsgData::ReqUserCard* cmd);
        //建筑道路
        bool reqBuildRoad(SceneUser* user,const HelloKittyMsgData::ReqBuildRoad *cmd);
        //清除道路
        bool reqClearRoad(SceneUser* user,const HelloKittyMsgData::ReqClearRoad *cmd);
        //请求所有生产道具的建筑信息
        bool reqAllConstructBuild(SceneUser* user,const HelloKittyMsgData::ReqAllConstructBuild *cmd);
        //解锁建筑
        bool reqUnLockBuild(SceneUser* user,const HelloKittyMsgData::ReqUnLockBuild *cmd);
        //点击激活建筑
        bool reqClickActiveBuild(SceneUser* user,const HelloKittyMsgData::ReqClickActiveBuild *cmd);
        //购买cd
        bool reqParseBuildCD(SceneUser* user,const HelloKittyMsgData::ReqParseBuildCD *cmd);
        //放入合成材料
        bool reqCompositeWork(SceneUser* user,const HelloKittyMsgData::ReqCompositeWork *cmd);
        //.......................建筑解析函数结束......................


        //.......................乐园地图解析函数开始......................

        //请求开放区域
        bool reqOpenArea(SceneUser* user,const HelloKittyMsgData::ReqOpenArea *cmd);
        //买cd
        bool reqParseAreaGridCD(SceneUser* user,const HelloKittyMsgData::ReqParseAreaGridCD *cmd);

        //.......................乐园解析函数结束......................

        //.......................任务解析函数开始......................

        //请求所有任务
        bool reqTask(SceneUser* u, const HelloKittyMsgData::ReqAllTask* cmd);
        //提交任务
        bool commitTask(SceneUser* u, const HelloKittyMsgData::ReqSubmitTask* cmd);

        //.......................任务解析函数结束......................


        //.......................图鉴解析函数开始......................

        //请求图鉴
        bool reqAtlas(SceneUser* user,const HelloKittyMsgData::ReqAtlas *cmd);
        //.......................图鉴解析函数结束......................


        //.......................成就解析函数开始......................
        //请求成就数据
        bool reqAchieve(SceneUser* u, const HelloKittyMsgData::ReqAllAchieve* cmd);
        //提交成就
        bool commitAchieve(SceneUser* u, const HelloKittyMsgData::ReqSubmitAchieve* cmd);
        //.......................成就解析函数开始......................

        //.......................邮件解析函数开始......................
        //请求邮件
        bool reqEmail(SceneUser* u, const HelloKittyMsgData::ReqEmail *cmd);
        //请求发送邮件
        bool reqSendEmail(SceneUser* u, const HelloKittyMsgData::ReqSendEmail *cmd);
        //请求操作邮件
        bool reqOpEmail(SceneUser* u, const HelloKittyMsgData::ReqOpEmail *cmd);
        //.......................邮件解析函数结束......................

        //.......................清空糖果罐......................
        bool reqclearSweetBox(SceneUser* user,const HelloKittyMsgData::ReqclearSweetBox *cmd);
        //.........................好友........................
        bool ReqAddFriend(SceneUser* User,const HelloKittyMsgData::ReqAddFriend *message);
        bool ReqKickFriend(SceneUser* User,const HelloKittyMsgData::ReqKickFriend *message);
        bool ReqRelationList(SceneUser* User,const HelloKittyMsgData::ReqRelationList *message);
        bool ReqGetOnePerson(SceneUser* User,const HelloKittyMsgData::ReqGetOnePerson *message);

        //........................家园访问..........................

        bool ReqEnterGarden(SceneUser* User,const HelloKittyMsgData::ReqEnterGarden *message);
        bool opBuilding(SceneUser* User,const HelloKittyMsgData::opBuilding *message);
        //请求购买道具
        bool ReqPurchase(SceneUser* User,const HelloKittyMsgData::ReqPurchase *message);
        //商店购买道具
        bool reqPurchaseItem(SceneUser* user,const HelloKittyMsgData::ReqPurchaseItem *message);

        //.......................时装解析函数开始......................
        //请求时装
        bool reqDress(SceneUser* u, const HelloKittyMsgData::ReqDress* cmd);
        //时装操作
        bool reqOpDress(SceneUser* user,const HelloKittyMsgData::ReqOpDress*cmd);
        //.......................时装解析函数开始......................

        //.......................图纸解析函数开始......................
        //请求时装
        bool reqPaper(SceneUser* u, const HelloKittyMsgData::ReqPaper* cmd);
        //图纸操作
        bool reqOpPaper(SceneUser* user,const HelloKittyMsgData::ReqOpPaper *cmd);
        //.......................图纸解析函数开始......................

        //.......................占卜解析函数开始......................
        //请求占卜数据
        bool reqDivineInfo(SceneUser* u, const HelloKittyMsgData::ReqDivineInfo* cmd);
        //请求第一个提示
        bool reqDivineFristNotice(SceneUser* user,const HelloKittyMsgData::ReqDivineNotice *cmd);
        //占卜
        bool reqDivine(SceneUser* user,const HelloKittyMsgData::ReqDivine*cmd);
        //验证占卜
        bool reqDivineVerify(SceneUser* user,const HelloKittyMsgData::ReqDivineVerify *cmd);
        //.......................占卜解析函数开始......................

        //.......................嘉年华解析函数开始......................
        //开启嘉年华
        bool reqOpenCarnical(SceneUser* user,const HelloKittyMsgData::ReqOpenCarnical *cmd);
        //点击嘉年华宝盒
        bool reqClickCarnival(SceneUser* user,const HelloKittyMsgData::ReqClickCarnicalBox *cmd);
        //购买嘉年华宝盒中的道具
        bool reqBuyCarnival(SceneUser* user,const HelloKittyMsgData::ReqBuyCarnicalBox *cmd);
        //.......................嘉年华解析函数开始......................


        //.......................突发事件华解析函数开始......................
        //请求突发事件列表
        bool reqBurstEvent(SceneUser* user,const HelloKittyMsgData::ReqBurstEvent *cmd);
        //请求操作突发事件
        bool reqOpBurstEvent(SceneUser* user,const HelloKittyMsgData::ReqOpBurstEvent *cmd);
        //.......................突发事件解析函数开始......................

        //.......................小游戏解析函数开始......................
        //请求开始猜星座
        bool reqBeginStar(SceneUser* user,const HelloKittyMsgData::ReqBeginStar *cmd);
        //提交关卡数据
        bool reqCommitStar(SceneUser* user,const HelloKittyMsgData::ReqStarCommitStep *cmd);
        //开始进行摇杆
        bool reqBeginSlot(SceneUser* user,const HelloKittyMsgData::ReqBeginSlot *cmd);
        //开始万家乐
        bool reqBeginMacro(SceneUser* user,const HelloKittyMsgData::ReqBeginMacro *cmd);
        //开始请求做寿司
        bool reqBeginSuShi(SceneUser* user,const HelloKittyMsgData::ReqBeginSuShi *cmd);
        //请求验证阶段数据
        bool reqCommitStepData(SceneUser* user,const HelloKittyMsgData::ReqCommitStep *cmd);

        bool reqStartGame(SceneUser* user,const HelloKittyMsgData::ReqStartGame *cmd);
        bool reqJoinStartGame(SceneUser* user,const HelloKittyMsgData::ReqJoinStartGame *cmd);
        bool reqCancelStartGame(SceneUser* user,const HelloKittyMsgData::ReqCancelStar *cmd);
        bool reqSyncStar(SceneUser* user,const HelloKittyMsgData::ReqSyncStar *cmd);
        bool reqReadyGame(SceneUser* user,const HelloKittyMsgData::ReqReadyGame *cmd);

        //.......................小游戏解析函数结束......................


        //.......................拍卖解析函数开始......................
        //请求操作游艺中心
        bool reqOpCenter(SceneUser* user,const HelloKittyMsgData::ReqOpCenter* cmd);
        //请求操作拍卖房间
        bool reqOpRoom(SceneUser* user,const HelloKittyMsgData::ReqOpRoom *cmd);
        //请求举牌
        bool reqBid(SceneUser* user,const HelloKittyMsgData::ReqBid *cmd);
        //请求设置自动举牌
        bool reqAutoBid(SceneUser* user,const HelloKittyMsgData::ReqAutoBid *cmd);
        //请求购买礼品信息
        bool reqParchaseNormalGift(SceneUser* user,const HelloKittyMsgData::ReqBuyNormalGift *cmd);
        //请求兑换礼品数量
        bool reqExchangeGiftNum(SceneUser* user,const HelloKittyMsgData::ReqExchangeGiftNum *cmd);
        //.......................拍卖解析函数结束......................


        //.......................礼品解析函数开始......................
        //请求更新
        bool reqUpdate(SceneUser* user,const HelloKittyMsgData::ReqUpdate *cmd);
        //请求操作礼品
        bool reqGiftOp(SceneUser* user,const HelloKittyMsgData::ReqOpGift* cmd);
        //请求刷新地址
        bool reqAddress(SceneUser* user,const HelloKittyMsgData::ReqAddress* cmd);
        //请求更改地址
        bool reqChangeAddress(SceneUser* user,const HelloKittyMsgData::ReqChangeAddress *cmd);
        //请求兑换实物
        bool reqCashGift(SceneUser* user,const HelloKittyMsgData::ReqCashGift* cmd);
        //请求确认收货
        bool reqCommitGift(SceneUser* user,const HelloKittyMsgData::ReqCommitGift *cmd);
        //请求库存信息
        bool reqPhyCondInfo(SceneUser* user,const HelloKittyMsgData::ReqPhyCondInfo *cmd);
        //送鲜花
        bool reqSendFlower(SceneUser* user,const HelloKittyMsgData::ReqSendFlower *cmd);
        //赠送虚拟商店礼品
        bool reqSendVirtualGift(SceneUser* user,const HelloKittyMsgData::ReqSendVirtualGift *cmd);
        //.......................礼品解析函数开始......................

        //.......................排行榜解析函数开始......................
        //请求排行榜
        bool reqRank(SceneUser* user,const HelloKittyMsgData::ReqRank *cmd);
        //.......................排行榜解析函数结束......................

        //.......................活动解析函数开始......................
        //参加活动
        bool reqJoinActive(SceneUser* user,const HelloKittyMsgData::ReqJoinActive* cmd);
        //单播活动
        bool reqActiveInfo(SceneUser* user,const HelloKittyMsgData::ReqAckActiveInfo* cmd);
        //.......................活动解析函数结束......................


        //.......................扭蛋解析函数开始......................
        //参加扭蛋
        bool reqToy(SceneUser* u, const HelloKittyMsgData::ReqOpToy* cmd);
        //.......................扭蛋解析函数结束......................


        //.......................空间解析函数开始......................
        //修改自我介绍
        bool reqModifyPresent(SceneUser* user, const HelloKittyMsgData::ReqModifyPresent *cmd);
        //修改语音
        bool reqModifyVoice(SceneUser* user, const HelloKittyMsgData::ReqModifyVoice *cmd);
        //请求进入空间
        bool reqEnterRoom(SceneUser* User,const HelloKittyMsgData::ReqEnterRoom *message);
        //请求霓虹广场
        bool reqNoCenter(SceneUser* user, const HelloKittyMsgData::ReqNoCenter *cmd);
        //点赞
        bool reqOpLike(SceneUser* user, const HelloKittyMsgData::ReqOpLike *cmd);
        //查看资料
        bool reqPersonInalInfo(SceneUser* user, const HelloKittyMsgData::ReqPersonInalInfo *cmd);
        bool reqRoomAndPersonalInfo(SceneUser* user, const HelloKittyMsgData::ReqRoomAndPersonalInfo *cmd);
        bool reqEditPersonInalInfo(SceneUser* user, const HelloKittyMsgData::ReqEditPersonInalInfo *cmd);
        bool reqNeonMark(SceneUser* user, const HelloKittyMsgData::ReqNeonMark *cmd);
        bool reqAddPicture(SceneUser* user, const HelloKittyMsgData::ReqAddPicture *cmd);
        bool reqMovePicture(SceneUser* user,const HelloKittyMsgData::ReqMovePicture *message);
        bool reqSetPictureHead(SceneUser* user, const HelloKittyMsgData::ReqSetPictureHead *cmd);
        bool reqSellWareHouseBuild(SceneUser* user, const HelloKittyMsgData::ReqSellWareHouseBuild *cmd);
        bool reqOutRoom(SceneUser* User,const HelloKittyMsgData::ReqOutRoom *message);
        bool reqBuyloginLast(SceneUser* user,const HelloKittyMsgData::ReqBuyloginLast *message);
        bool reqViewWechat(SceneUser* user,const HelloKittyMsgData::ReqViewWechat *message);
        //.......................空间解析函数结束......................


        //兑换激活码
        bool reqRewardActiveCode(SceneUser* user,const HelloKittyMsgData::ReqRewardActiveCode *message);



};

#endif

