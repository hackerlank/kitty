#ifndef _PLAYER_ACTIVE_CMD_DISPATCHER
#define _PLAYER_ACTIVE_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "playeractive.pb.h"

class SceneUser;

class PlayerActiveCmdHandle : public zCmdHandle
{
    public:
        PlayerActiveCmdHandle()
        {
        }

        void init(){
#define REGISTERPLAYERACTIVE(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &PlayerActiveCmdHandle::SELFFUN));
#define REGISTERPLAYERACTIVESAME(PROTOCFUN) REGISTERPLAYERACTIVE(PROTOCFUN,PROTOCFUN) 
            REGISTERPLAYERACTIVESAME(ReqgetPlayerActiveList);
            REGISTERPLAYERACTIVESAME(ReqgetActiveAward);

        }
        //获取所有活动
        bool ReqgetPlayerActiveList(SceneUser* u, const HelloKittyMsgData::ReqgetPlayerActiveList* cmd);
        //获取活动奖励
        bool ReqgetActiveAward(SceneUser* u, const HelloKittyMsgData::ReqgetActiveAward* cmd);



};

#endif

