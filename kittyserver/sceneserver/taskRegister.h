#ifndef TASK_REGISTER_H
#define TASK_REGISTER_H
#include "taskAttr.h"
#include "dataManager.h"
#include "task.pb.h"

class SceneUser;

//检查建筑类型任务
TargetRetType checkBuildTarger(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg);
//检查交互型任务
TargetRetType checkInterActiveTarger(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg);
//检查拥有资源类型任务
TargetRetType checkSourceTarger(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg);
//检查角色等级任务
TargetRetType checkRoleLevelTarget(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg);
//检查角色图鉴获取数量
TargetRetType checkAtlasTarget(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg);
//检查时装类型任务
TargetRetType checkAvartarTarget(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg);
//检查占卜类型任务
TargetRetType checkDivineTarget(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg);
//检查扭蛋类型任务
TargetRetType checkToyTarget(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg);
#endif
