#ifndef _guide_CMD_DISPATCHER
#define _guide_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "guide.pb.h"

class SceneUser;

class guideCmdHandle : public zCmdHandle
{
    public:
        guideCmdHandle()
        {
        }

        void init()
        {
#define REGISTERGUIDE(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &guideCmdHandle::SELFFUN));
#define REGISTERGUIDESAME(PROTOCFUN) REGISTERGUIDE(PROTOCFUN,PROTOCFUN) 
            REGISTERGUIDESAME(ReqSetRoleName);
            REGISTERGUIDESAME(Reqsetguidefinish);
            REGISTERGUIDESAME(ReqsetTaskguidefinish);
            REGISTERGUIDESAME(ReqSetHead);

        }


        //.........................新手........................
        bool ReqSetRoleName(SceneUser* User,const HelloKittyMsgData::ReqSetRoleName *message);
        bool Reqsetguidefinish(SceneUser* User,const HelloKittyMsgData::Reqsetguidefinish *message);
        bool ReqsetTaskguidefinish(SceneUser* User,const HelloKittyMsgData::ReqsetTaskguidefinish *message);
        bool ReqSetHead(SceneUser* User,const HelloKittyMsgData::ReqSetHead *message);


};

#endif

