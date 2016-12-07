#ifndef _UNITYBUILD_CMD_DISPATCHER
#define _UNITYBUILD_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "unitbuild.pb.h"

class SceneUser;

class unitbuildCmdHandle : public zCmdHandle
{
    public:
        unitbuildCmdHandle()
        {
        }

        void init()
        {
#define REGISTERUNITYBUILD(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &unitbuildCmdHandle::SELFFUN));
#define REGISTERUNITYBUILDSAME(PROTOCFUN) REGISTERUNITYBUILD(PROTOCFUN,PROTOCFUN) 
            REGISTERUNITYBUILDSAME(ReqAllUnitBuildInfo);
            REGISTERUNITYBUILDSAME(ReqResetColId);
            REGISTERUNITYBUILDSAME(ReqOpenColId);
            REGISTERUNITYBUILDSAME(ReqUnitBuild);
            REGISTERUNITYBUILDSAME(ReqAgreeUnitBuild);
            REGISTERUNITYBUILDSAME(ReqStopBuild);
            REGISTERUNITYBUILDSAME(ReqAddSpeedBuild);
            REGISTERUNITYBUILDSAME(ReqActiveBuild);
            REGISTERUNITYBUILDSAME(ReqCancelInvite);

        }


        //.........................新手........................
        bool ReqAllUnitBuildInfo(SceneUser* User,const HelloKittyMsgData::ReqAllUnitBuildInfo *message);
        bool ReqOpenColId(SceneUser* User,const HelloKittyMsgData::ReqOpenColId *message);
        bool ReqResetColId(SceneUser* User,const HelloKittyMsgData::ReqResetColId *message);
        bool ReqUnitBuild(SceneUser* User,const HelloKittyMsgData::ReqUnitBuild *message);
        bool ReqAgreeUnitBuild(SceneUser* User,const HelloKittyMsgData::ReqAgreeUnitBuild *message);
        bool ReqStopBuild(SceneUser* User,const HelloKittyMsgData::ReqStopBuild *message);
        bool ReqAddSpeedBuild(SceneUser* User,const HelloKittyMsgData::ReqAddSpeedBuild *message);
        bool ReqActiveBuild(SceneUser* User,const HelloKittyMsgData::ReqActiveBuild *message);
        bool ReqCancelInvite(SceneUser* User,const HelloKittyMsgData::ReqCancelInvite *message);
    private:
        void Addunitbuild(SceneUser* User,HelloKittyMsgData::UnitRunInfo &binary);
        void Updateunitbuild(SceneUser* User,HelloKittyMsgData::UnitRunInfo &binary);
        void Delunitbuild(SceneUser* User,HelloKittyMsgData::UnitRunInfo &binary);
        void ReturnResult(SceneUser* User,HelloKittyMsgData::UnitBuildResult result,DWORD param = 0);



};

#endif

