#ifndef GM_TOOL_DISPATCHER_H
#define GM_TOOL_DISPATCHER_H 

#include <string.h>
#include "Fir.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "GmToolTask.h"
#include "gmtool.pb.h"
#include "gmtest.pb.h"

class GmToolCmdHandle : public zCmdHandle
{
    public:
        GmToolCmdHandle()
        {

        }

        void init()
        {
            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqGmLogin>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqGmLogin>::ProtoFunction(this, &GmToolCmdHandle::reqGmLogin));

            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqGmModifypasswd>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqGmModifypasswd>::ProtoFunction(this, &GmToolCmdHandle::reqGmModifypasswd));

            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqGmShowGmAccount>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqGmShowGmAccount>::ProtoFunction(this, &GmToolCmdHandle::reqGmShowGmAccount));

            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqModityGmData>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqModityGmData>::ProtoFunction(this, &GmToolCmdHandle::reqModifyGmData));

            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqModifyUserAttr>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqModifyUserAttr>::ProtoFunction(this, &GmToolCmdHandle::reqModifyUserAttr));

            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqTest>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqTest>::ProtoFunction(this, &GmToolCmdHandle::reqTest));

            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqModifyUserBuild>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqModifyUserBuild>::ProtoFunction(this, &GmToolCmdHandle::reqModifyUserBuild));

            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqForbid>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqForbid>::ProtoFunction(this, &GmToolCmdHandle::reqForbid));

            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqGmToolSendEmail>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqGmToolSendEmail>::ProtoFunction(this, &GmToolCmdHandle::reqSendEmail));

            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqOpNotice>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqOpNotice>::ProtoFunction(this, &GmToolCmdHandle::reqOpNotice));

            GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::ReqModifyGiftStore>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::ReqModifyGiftStore>::ProtoFunction(this, &GmToolCmdHandle::reqModifyGiftStore));
#define GmToolTaskSame(Fun) GmToolTask::gm_tool_dispatcher.func_reg<HelloKittyMsgData::Fun>(ProtoCmdCallback<GmToolTask,HelloKittyMsgData::Fun>::ProtoFunction(this, &GmToolCmdHandle::Fun));
            GmToolTaskSame(ReqAddPlayerActive);
            GmToolTaskSame(ReqModifyPlayerActive);
            GmToolTaskSame(ReqOpenActive);
            GmToolTaskSame(ReqDelUserPicture);
            GmToolTaskSame(ReqSendGlobalEmail);
            GmToolTaskSame(ReqModifyGiftInfo);
            GmToolTaskSame(ReqModifyUserVerify);
            GmToolTaskSame(ReqAddActiveCode);

        }
        //登录
        bool reqGmLogin(GmToolTask* task,const HelloKittyMsgData::ReqGmLogin *message);
        //修改gm密码
        bool reqGmModifypasswd(GmToolTask* task,const HelloKittyMsgData::ReqGmModifypasswd *message);
        //查询所有GM用户
        bool reqGmShowGmAccount(GmToolTask* task,const HelloKittyMsgData::ReqGmShowGmAccount *message);
        //批量修改GM信息
        bool reqModifyGmData(GmToolTask* task,const HelloKittyMsgData::ReqModityGmData *message);
        //批量修改玩家角色属性
        bool reqModifyUserAttr(GmToolTask* task,const HelloKittyMsgData::ReqModifyUserAttr *message);
        //批量修改玩家建筑信息
        bool reqModifyUserBuild(GmToolTask* task,const HelloKittyMsgData::ReqModifyUserBuild *message);
        //封号
        bool reqForbid(GmToolTask* task,const HelloKittyMsgData::ReqForbid *message);
        //发送邮件
        bool reqSendEmail(GmToolTask* task,const HelloKittyMsgData::ReqGmToolSendEmail *message);
        //操作公告
        bool reqOpNotice(GmToolTask* task,const HelloKittyMsgData::ReqOpNotice *message);
        //测试
        bool reqTest(GmToolTask* task,const HelloKittyMsgData::ReqTest *message);
        //操作礼品库存
        bool reqModifyGiftStore(GmToolTask* task,const HelloKittyMsgData::ReqModifyGiftStore *message);
        //活动
        bool ReqAddPlayerActive(GmToolTask* task,const HelloKittyMsgData::ReqAddPlayerActive *message);
        bool ReqModifyPlayerActive(GmToolTask* task,const HelloKittyMsgData::ReqModifyPlayerActive *message);
        bool ReqOpenActive(GmToolTask* task,const HelloKittyMsgData::ReqOpenActive *message);
        //删除照片
        bool ReqDelUserPicture(GmToolTask* task,const HelloKittyMsgData::ReqDelUserPicture *message);
        //全服邮件
        bool ReqSendGlobalEmail(GmToolTask* task,const HelloKittyMsgData::ReqSendGlobalEmail *message);
        //礼品信息
        bool ReqModifyGiftInfo(GmToolTask* task,const HelloKittyMsgData::ReqModifyGiftInfo *message);
        //修改角色认证
        bool ReqModifyUserVerify(GmToolTask* task,const HelloKittyMsgData::ReqModifyUserVerify *message);
        //增加激活码
        bool ReqAddActiveCode(GmToolTask* task,const HelloKittyMsgData::ReqAddActiveCode *message);

};

#endif
