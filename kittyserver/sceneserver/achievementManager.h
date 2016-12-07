#ifndef ACHIEVE_MENT_MANAGER_H
#define ACHIEVE_MENT_MANAGER_H
#include "achievement.pb.h"
#include <map>
#include "zType.h"
#include "serialize.pb.h"
#include "dataManager.h"
#include "taskAttr.h"

class SceneUser;
class AchievementManager
{
    public:
        AchievementManager(SceneUser *owner);
        //加载数据
        bool load(const HelloKittyMsgData::Serialize& binary);
        //保存数据
        bool save(HelloKittyMsgData::Serialize& binary);
        //道具使用回调函数
        typedef TargetRetType (*Achieve_Target_Check)(SceneUser *owner,const pb::Conf_t_Achievement *achieveConf,HelloKittyMsgData::AchieveMent *achieve,const AchieveArg &arg);
        //开启一个成就
        bool openAchieve(const QWORD achieveID,const DWORD stars,const bool initFlg = false);
        //初始化成就
        bool init();
        //领取成就奖励
        bool rewardAchieve(const QWORD achieveID);
        //刷新成就列表
        bool flushAllAchieve();
        //触发成就
        bool target(const AchieveArg &arg);
        //完成成就
        bool finishAchieve(const QWORD achieveID);
    private:
        //初始化注册函数容器
        void initCheckTragerMap();
        //判断是否接受该成就
        bool isAcceptAchieve(const QWORD achieveID);
        //接受成就
        bool acceptTask(const QWORD achieveID);
        //获得成就实例
        HelloKittyMsgData::AchieveMent* getAchieve(const QWORD achieveID);
        //处理成就类型容器
        bool opTypeMap(const QWORD achieveID,bool opAdd);
        //更新成就
        bool updateAchieve(const QWORD achieveID);
        //判断是否可以领取奖励
        bool isAchieveReward(const QWORD achieveID);
        //获得物品所占包裹大小
        DWORD getRewardTaskNum(const std::map<DWORD,DWORD>& rewardMap);
        //接受成就
        bool acceptAchieve(const QWORD achieveID,const DWORD stars);
        //重置数据
        void reset();
    private:
        SceneUser *m_owner;
        //成就列表
        std::map<DWORD,HelloKittyMsgData::AchieveMent> m_achievementMap;
        //成就类型对应成就id集合
        std::map<DWORD,std::set<QWORD>> m_achieveTypeMap;
        //成就注册函数初始化标志
        static bool s_initTargetCheckFlg;
        //成就检测注册函数容器
        static std::map<DWORD,Achieve_Target_Check> s_tragetCheckMap;
};

#endif

