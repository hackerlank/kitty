#ifndef _TRAINORDER_CMD_DISPATCHER
#define _TRAINORDER_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "trainorder.pb.h"
class SceneUser;

class TrainOrderCmdHandle : public zCmdHandle
{
    public:
        TrainOrderCmdHandle()
        {
        }

        void init(){
#define REGISTERTRAINORDER(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &TrainOrderCmdHandle::SELFFUN));
#define REGISTERTRAINORDERSAME(PROTOCFUN) REGISTERTRAINORDER(PROTOCFUN,PROTOCFUN) 
            REGISTERTRAINORDERSAME(ReqLoadCarriage);
            REGISTERTRAINORDERSAME(ReqAskHelpLoadCarriage);
            REGISTERTRAINORDERSAME(ReqAnswerHelpLoadCarriage);
            REGISTERTRAINORDERSAME(ReqGetTrainAward);
            REGISTERTRAINORDERSAME(ReqOpenNewTrain);
REGISTERTRAINORDERSAME(ReqClearTrainCD);




        }
        bool ReqLoadCarriage(SceneUser* u, const HelloKittyMsgData::ReqLoadCarriage* cmd);
        bool ReqAskHelpLoadCarriage(SceneUser* u, const HelloKittyMsgData::ReqAskHelpLoadCarriage* cmd);
        bool ReqAnswerHelpLoadCarriage(SceneUser* u, const HelloKittyMsgData::ReqAnswerHelpLoadCarriage* cmd);
        bool ReqGetTrainAward(SceneUser* u, const HelloKittyMsgData::ReqGetTrainAward* cmd);
        bool ReqOpenNewTrain(SceneUser* u, const HelloKittyMsgData::ReqOpenNewTrain* cmd);
        bool ReqClearTrainCD(SceneUser* u, const HelloKittyMsgData::ReqClearTrainCD* cmd);
    private:
        void getAwardForLoad(DWORD trainNo,HelloKittyMsgData::vecAward &rvecAward,const HelloKittyMsgData::Award& rneeditem,SceneUser* u,bool bself);



};

#endif

