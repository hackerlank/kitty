#ifndef _FAMILY_CMD_DISPATCHER
#define _FAMILY_CMD_DISPATCHER

#include <string.h>
#include "Fir.h"
#include "Command.h"
#include "SceneServer.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "family.pb.h"
#include "RecordCommand.h"

class SceneUser;
class RecordClient;

class FamilyCmdHandle : public zCmdHandle
{
    public:
        FamilyCmdHandle()
        {
        }

        void init()
        {
#define REGISTERFAMILY(PROTOCFUN,SELFFUN) \
            SceneTask::scene_user_dispatcher.func_reg<HelloKittyMsgData::PROTOCFUN>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::PROTOCFUN>::ProtoFunction(this, &FamilyCmdHandle::SELFFUN));
#define REGISTERFAMILYSAME(PROTOCFUN) REGISTERFAMILY(PROTOCFUN,PROTOCFUN) 
            REGISTERFAMILYSAME(ReqGetFamilyList);
            REGISTERFAMILYSAME(ReqAddFamily);
            REGISTERFAMILYSAME(ReqCancelApply);  
            REGISTERFAMILYSAME(ReqCreateFamily);
            REGISTERFAMILYSAME(ReqAgreeJoin);
            REGISTERFAMILYSAME(ReqselfFamilyInfo);
            REGISTERFAMILYSAME(ReqQuitFamily);
            REGISTERFAMILYSAME(ReqUpdateOtherFamilyJob);
            REGISTERFAMILYSAME(ReqKickFamilyMember);
            REGISTERFAMILYSAME(ReqSetFamilyBaseInfo);
            REGISTERFAMILYSAME(ReqFamilyRanking);
            REGISTERFAMILYSAME(ReqFinishFamilyOrder);
            REGISTERFAMILYSAME(ReqGetlastFamilyAward);


        }


        //.........................家族........................
        bool ReqGetFamilyList(SceneUser* User,const HelloKittyMsgData::ReqGetFamilyList *message);
        bool ReqAddFamily(SceneUser* User,const HelloKittyMsgData::ReqAddFamily *message);
        bool ReqCreateFamily(SceneUser* User,const HelloKittyMsgData::ReqCreateFamily *message); 
        bool ReqAgreeJoin(SceneUser* User,const HelloKittyMsgData::ReqAgreeJoin *message); 
        bool ReqselfFamilyInfo(SceneUser* User,const HelloKittyMsgData::ReqselfFamilyInfo *message); 
        bool ReqQuitFamily(SceneUser* User,const HelloKittyMsgData::ReqQuitFamily *message); 
        bool ReqUpdateOtherFamilyJob(SceneUser* User,const HelloKittyMsgData::ReqUpdateOtherFamilyJob *message); 
        bool ReqKickFamilyMember(SceneUser* User,const HelloKittyMsgData::ReqKickFamilyMember *message); 
        bool ReqSetFamilyBaseInfo(SceneUser* User,const HelloKittyMsgData::ReqSetFamilyBaseInfo *message); 
        bool ReqCancelApply(SceneUser* User,const HelloKittyMsgData::ReqCancelApply *message); 
        bool ReqFamilyRanking(SceneUser* User,const HelloKittyMsgData::ReqFamilyRanking *message); 
        bool ReqFinishFamilyOrder(SceneUser* User,const HelloKittyMsgData::ReqFinishFamilyOrder *message);
        bool ReqGetlastFamilyAward(SceneUser* User,const HelloKittyMsgData::ReqGetlastFamilyAward *message);
    public:
        static void DocreateReturn(const CMD::RECORD::t_WriteFamily_RecordScene_Create_Return *cmd); 
        bool ReqAddFamilyByGm(SceneUser* User,const HelloKittyMsgData::ReqAddFamily *message);
        bool ReqCreateFamilyByGm(SceneUser* User,const HelloKittyMsgData::ReqCreateFamily *message);
        static void UpdateFamily(QWORD qwFamilyId);
        void NoticeKick(QWORD charid);

    private:
        void DoReqAddFamily(SceneUser* User,const HelloKittyMsgData::ReqAddFamily *message,HelloKittyMsgData::AckReqAddFamily &ack,bool bIsGm);
        HelloKittyMsgData::FamilyOpResult DoCreateFamily(SceneUser* User,const HelloKittyMsgData::ReqCreateFamily *message,bool bIsGm);
        bool checkFamilyBulid(SceneUser* User);
        HelloKittyMsgData::FamilyOpResult   checkFamilyName(const string & strName);
        HelloKittyMsgData::FamilyOpResult   checkFamilyNotice(const string & strNotice);
        static bool delCreateSource(SceneUser* User);
        static void returnCreateSource(SceneUser* User);

};

#endif

