#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H
#include <map>
#include "zType.h"
#include "serialize.pb.h"
#include "taskAttr.h"
#include "dataManager.h"
#include <set>

class SceneUser;
class TaskManager
{
    public:
        TaskManager(SceneUser *owner);
        ~TaskManager();
        //加载任务
        bool load(const HelloKittyMsgData::Serialize& binary);
        //保存任务数据
        bool save(HelloKittyMsgData::Serialize& binary);
        //刷新所有任务
        bool flushAllTask(const HelloKittyMsgData::TaskType &taskType);
        //更新任务
        bool updateTask(const QWORD taskID);
        //领取任务奖励
        bool rewardTask(const QWORD taskID);
        //道具使用回调函数
        typedef TargetRetType (*Task_Target_Check)(SceneUser *owner,const pb::Conf_t_Task *taskConf,HelloKittyMsgData::Task *task,const TaskArgue &arg);
        //触发任务检测函数
        bool target(const TaskArgue &argue);
        //初始化任务列表
        bool init();
        //开启任务
        bool openTask(const QWORD taskID,const bool gmFlg = false);
        //完成任务
        bool finishTask(const QWORD taskID);
        //获取某种任务类型的数量
        DWORD getTaskTypeNum(const TaskType &taskType,const HelloKittyMsgData::TaskStatus &status);
        //获得任务总数量
        DWORD getTaskNum();
        //重置日常任务
        bool resetDailyTask();
        //角色等级提升，解锁任务
        bool upLevel();
        //判定某个任务是否完成
        bool checkTaskHasDone(const QWORD taskID);
    private:
        //判断某个任务是否已经领取过奖励
        bool isTaskReward(const QWORD taskID);
        //判断某个任务的前置任务是都都已经完成(领取过奖励)
        bool checkPreTask(const QWORD taskID);
        //接收某个任务
        bool acceptTask(const QWORD taskID);
        //判断某个任务是否已接
        bool isAcceptTask(const QWORD taskID);
        //完成某个任务开启对应的任务
        bool openNextTask(const QWORD taskID);
        //获得某个任务
        HelloKittyMsgData::Task* getTask(const QWORD taskID);
        //对任务类型容器进行操作
        bool opTypeMap(const QWORD taskID,bool opAdd = true);
        //初始化任务注册函数
        void initCheckTragerMap();
        //重置数据
        void reset();
    private:
        SceneUser *m_owner;
        //角色身上所有任务
        std::map<QWORD,HelloKittyMsgData::Task> m_taskMap;
        //按照任务类型对应任务集合
        std::map<DWORD,std::set<QWORD>> m_taskTypeMap;
        static bool s_initTargetCheckFlg;
        //任务检测注册函数容器
        static std::map<DWORD,Task_Target_Check> s_tragetCheckMap;
};


#endif
