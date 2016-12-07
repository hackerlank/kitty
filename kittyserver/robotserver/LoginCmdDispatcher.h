/**
 * \file
 * \version  $Id: LoginCmdDispatcher.h 42 2013-04-10 07:33:59Z  $
 * \author   ,
 * \date 2013年03月27日 12时14分48秒 CST
 * \brief 定义用户登录相关命令处理文件，注册给dispatcher
 *
 */

#ifndef _LOGIN_USER_CMD_DISPATCHER
#define _LOGIN_USER_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "LoadClient.h"
#include "login.pb.h"
#include "kittygarden.pb.h" 
#include "chat.pb.h"
#include "gm.pb.h"
#include "family.pb.h"
#include "guide.pb.h"
#include "toy.pb.h"
#include "system.pb.h"
#include "unitbuild.pb.h"
#include "rank.pb.h"

class LoginCmdHandle : public zCmdHandle
{
    public:
        LoginCmdHandle()
        {

        }

        void init()
        {
#define RIGSTER(Fun) { \
    LoadClient::login_user_dispatcher.func_reg<HelloKittyMsgData::Fun>(ProtoCmdCallback<LoadClient,HelloKittyMsgData::Fun>::ProtoFunction(this, &LoginCmdHandle::Fun));}
            RIGSTER(AckVersion);
            RIGSTER(AckLoginFailReturn);
            RIGSTER(AckLoginSuccessReturn);
            RIGSTER(AckFlushUserInfo);
            RIGSTER(AckKittyGarden);
            RIGSTER(EventBuildNotice);
            RIGSTER(AckEnterGarden);
            RIGSTER(EventNotice);
            RIGSTER(returnEventAward);
            RIGSTER(AckopBuilding);
            RIGSTER(AckNoticeChat);
            RIGSTER(AckGM);
            RIGSTER(AckReqCreateFamily);
            RIGSTER(AckReqAddFamily);
            RIGSTER(AckReqselfFamilyInfo);
            RIGSTER(AckFinishFamilyOrder);
            RIGSTER(AckGetlastFamilyAward);
            RIGSTER(AckServerNotice);
            RIGSTER(AckAddServerNotice);
            RIGSTER(AckDelServerNotice);
            RIGSTER(AckSetRoleName);
            RIGSTER(Acksetguidefinish);
            RIGSTER(AckLeaveMessage);
            RIGSTER(AckAddMessage);
            RIGSTER(AckDelMessage);
            RIGSTER(AckRandToy);
            RIGSTER(AckHeartBeat); 

            RIGSTER(ACKAllUnitBuildInfo);
            RIGSTER(ACKOpUnitBuild);
            RIGSTER(ACKUpdateUnitBuild);
            RIGSTER(AckUnitbuildRank);
            RIGSTER(AckSysNotice);


        }
        bool AckVersion(LoadClient* task,const HelloKittyMsgData::AckVersion *message);
        bool AckLoginFailReturn(LoadClient* task,const HelloKittyMsgData::AckLoginFailReturn *message);
        bool AckLoginSuccessReturn(LoadClient* task,const HelloKittyMsgData::AckLoginSuccessReturn *message);
        bool AckFlushUserInfo(LoadClient* task,const HelloKittyMsgData::AckFlushUserInfo *message);
        bool AckKittyGarden(LoadClient* task,const HelloKittyMsgData::AckKittyGarden *message);
        bool EventBuildNotice(LoadClient* task,const HelloKittyMsgData::EventBuildNotice *message);
        bool AckEnterGarden(LoadClient* task,const HelloKittyMsgData::AckEnterGarden *message);
        bool EventNotice(LoadClient* task,const HelloKittyMsgData::EventNotice *message);
        bool returnEventAward(LoadClient* task,const HelloKittyMsgData::returnEventAward *message);
        bool AckopBuilding(LoadClient* task,const HelloKittyMsgData::AckopBuilding *message);
        bool AckNoticeChat(LoadClient* task,const HelloKittyMsgData::AckNoticeChat *message);
        bool AckGM(LoadClient* task,const HelloKittyMsgData::AckGM *message);
        bool AckReqCreateFamily(LoadClient* task,const HelloKittyMsgData::AckReqCreateFamily *message);
        bool AckReqAddFamily(LoadClient* task,const HelloKittyMsgData::AckReqAddFamily *message);
        bool AckReqselfFamilyInfo(LoadClient* task,const HelloKittyMsgData::AckReqselfFamilyInfo *message);
        bool AckFinishFamilyOrder(LoadClient* task,const HelloKittyMsgData::AckFinishFamilyOrder *message);
        bool AckGetlastFamilyAward(LoadClient* task,const HelloKittyMsgData::AckGetlastFamilyAward *message);
        bool AckServerNotice(LoadClient* task,const HelloKittyMsgData::AckServerNotice *message);
        bool AckAddServerNotice(LoadClient* task,const HelloKittyMsgData::AckAddServerNotice *message);
        bool AckDelServerNotice(LoadClient* task,const HelloKittyMsgData::AckDelServerNotice *message);
        bool AckSetRoleName(LoadClient* task,const HelloKittyMsgData::AckSetRoleName *message);
        bool Acksetguidefinish(LoadClient* task,const HelloKittyMsgData::Acksetguidefinish *message);
        bool AckLeaveMessage(LoadClient* task,const HelloKittyMsgData::AckLeaveMessage *message);
        bool AckAddMessage(LoadClient* task,const HelloKittyMsgData::AckAddMessage *message);
        bool AckDelMessage(LoadClient* task,const HelloKittyMsgData::AckDelMessage *message);
        bool AckRandToy(LoadClient* task,const HelloKittyMsgData::AckRandToy *message);
        bool AckHeartBeat(LoadClient* task,const HelloKittyMsgData::AckHeartBeat *message);

        bool ACKAllUnitBuildInfo(LoadClient* task,const HelloKittyMsgData::ACKAllUnitBuildInfo *message);
        bool ACKOpUnitBuild(LoadClient* task,const HelloKittyMsgData::ACKOpUnitBuild *message);
        bool ACKUpdateUnitBuild(LoadClient* task,const HelloKittyMsgData::ACKUpdateUnitBuild *message);
        bool AckUnitbuildRank(LoadClient* task,const HelloKittyMsgData::AckUnitbuildRank *message);
        bool AckSysNotice(LoadClient* task,const HelloKittyMsgData::AckSysNotice *message);


        void show(const HelloKittyMsgData::EventBuildNotice *message);
        void show(const HelloKittyMsgData::EventNotice *message);
        void show(const HelloKittyMsgData::FamilyInfo &rInfo);
        void show(const HelloKittyMsgData::ClientChatMessage &rInfo);



};

#endif
