#ifndef _MARKET_CMD_DISPATCHER
#define _MARKET_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "market.pb.h"

class SceneUser;

class marketCmdHandle : public zCmdHandle
{
    public:
        marketCmdHandle()
        {
        }

        void init()
        {
#define REGISTERMARKET(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &marketCmdHandle::SELFFUN));
#define REGISTERMARKETSAME(PROTOCFUN) REGISTERMARKET(PROTOCFUN,PROTOCFUN) 
            REGISTERMARKETSAME(ReqMarketAndServantInfo);
            REGISTERMARKETSAME(ReqBuyMarketItem);
            REGISTERMARKETSAME(ReqFlushMarket);
            REGISTERMARKETSAME(ReqBuyServant);
            REGISTERMARKETSAME(ReqBuyServantItem);
            REGISTERMARKETSAME(ReqGetServantAutoItem);
            REGISTERMARKETSAME(ReqOpenServantBox);

        }


        //.........................新手........................
        bool ReqMarketAndServantInfo(SceneUser* User,const HelloKittyMsgData::ReqMarketAndServantInfo *message);
        bool ReqBuyMarketItem(SceneUser* User,const HelloKittyMsgData::ReqBuyMarketItem *message);
        bool ReqFlushMarket(SceneUser* User,const HelloKittyMsgData::ReqFlushMarket *message);
        bool ReqBuyServant(SceneUser* User,const HelloKittyMsgData::ReqBuyServant *message);
        bool ReqBuyServantItem(SceneUser* User,const HelloKittyMsgData::ReqBuyServantItem *message);
        bool ReqGetServantAutoItem(SceneUser* User,const HelloKittyMsgData::ReqGetServantAutoItem *message);
        bool ReqOpenServantBox(SceneUser* User,const HelloKittyMsgData::ReqOpenServantBox *message);


};

#endif

