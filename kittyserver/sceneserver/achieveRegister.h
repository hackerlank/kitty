#ifndef ACHIEVE_REGISTER_H
#define ACHIEVE_REGISTER_H
#include "taskAttr.h"
#include "dataManager.h"
#include "achievement.pb.h"

class SceneUser;

//检查资源拥有类型
TargetRetType checkTargerHave(SceneUser *owner,const pb::Conf_t_Achievement *achieveConf,HelloKittyMsgData::AchieveMent *achieve,const AchieveArg &arg);
//检查兑换类型成就
TargetRetType checkTargerExchange(SceneUser *owner,const pb::Conf_t_Achievement *achieveConf,HelloKittyMsgData::AchieveMent *achieve,const AchieveArg &arg);
#endif
