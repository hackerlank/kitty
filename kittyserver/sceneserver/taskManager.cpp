#include "taskManager.h"
#include "SceneUser.h"
#include "taskRegister.h"
#include "tbx.h"
#include "TimeTick.h"

bool TaskManager::s_initTargetCheckFlg = false;
std::map<DWORD,TaskManager::Task_Target_Check> TaskManager::s_tragetCheckMap;

TaskManager::TaskManager(SceneUser *owner) : m_owner(owner)
{
    initCheckTragerMap();
}

TaskManager::~TaskManager()
{
}

bool TaskManager::load(const HelloKittyMsgData::Serialize& binary)
{
    reset();
    for(int index = 0;index < binary.task_size();++index)
    {
        const HelloKittyMsgData::Task &task = binary.task(index);
        const pb::Conf_t_Task *taskConf = tbx::Task().get_base(task.id());
        if(!taskConf)
        {
            continue;
        }
        if(m_taskMap.find(task.id()) != m_taskMap.end())
        {
            Fir::logger->debug("[加载任务出错]:任务id重复(%lu)",task.id());
            continue;
        }
        m_taskMap.insert(std::pair<QWORD,HelloKittyMsgData::Task>(task.id(),task));
        opTypeMap(task.id());
    }
    return true;
}

bool TaskManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_taskMap.begin();iter != m_taskMap.end();++iter)
    {
        const HelloKittyMsgData::Task &task = iter->second;
        HelloKittyMsgData::Task *temp = binary.add_task();
        if(temp)
        {
            *temp = task;
        }
    }
    return true;
}

bool TaskManager::openTask(const QWORD taskID,const bool gmFlg)
{
    const pb::Conf_t_Task *taskConf = tbx::Task().get_base(taskID);
    if(!taskConf || isAcceptTask(taskID))
    {
        return false;
    }
    if(!gmFlg && !checkPreTask(taskID))
    {
        return false;
    }

    TaskArgue arg((TaskTargetType)taskConf->task->target_type());
    arg.initFlg = true;
    auto fun = s_tragetCheckMap.find(arg.targerType);
    if(fun == s_tragetCheckMap.end())
    {
        Fir::logger->debug("[任务]:开启任务失败,任务类型没有对应的注册函数(%lu,%s,%lu,%u)",m_owner->charid,m_owner->charbase.nickname,taskID,arg.targerType);
        return false;
    }

    acceptTask(taskID);
    HelloKittyMsgData::Task* task = getTask(taskID);
    if(!task)
    {
        return false;
    }

    TargetRetType targetVal = fun->second(m_owner,taskConf,task,arg);
    //矫正一些错误的数值
    if(!task->total())
    {
        task->set_total(1);
        targetVal = Target_Ret_Update;
    }
    if(task->current() > task->total())
    {
        task->set_current(task->total());
        targetVal = Target_Ret_Finish;
    }

    if(targetVal == Target_Ret_Update)
    {
        updateTask(taskID);
    }
    else if(targetVal == Target_Ret_Finish)
    {
        finishTask(taskID);
    }
    else
    {
        updateTask(taskID);
    }
    return true;
}

void TaskManager::reset()
{
    m_taskMap.clear();
    m_taskTypeMap.clear();
}

bool TaskManager::init()
{
    reset();
    const std::unordered_map<unsigned int, const pb::Conf_t_Task*> &tbxMap = tbx::Task().getTbxMap();
    for(auto iter = tbxMap.begin();iter != tbxMap.end();++iter)
    {
        openTask(iter->first);
    }
    return true;
}

bool TaskManager::flushAllTask(const HelloKittyMsgData::TaskType &taskType)
{
    HelloKittyMsgData::AckAllTask message;
    message.set_tasktype(taskType);
    for(auto iter = m_taskMap.begin();iter != m_taskMap.end();++iter)
    {
        const HelloKittyMsgData::Task &task = iter->second;
        HelloKittyMsgData::Task *temp = message.add_task();
        const pb::Conf_t_Task *taskConf = tbx::Task().get_base(task.id());
        if(temp && taskConf)
        {
            if(taskType == HelloKittyMsgData::Task_Type_Default || taskConf->task->sub() == (DWORD)taskType)
            {
                *temp = task;
            }
        }
    }

    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool TaskManager::finishTask(const QWORD taskID)
{
    HelloKittyMsgData::Task *task = getTask(taskID);
    if(!task)
    {
        return false;
    }
    if(task->status() != HelloKittyMsgData::Task_Accept)
    {
        return false;
    }
    task->set_status(HelloKittyMsgData::Task_Finish);
    task->set_current(task->total());
    updateTask(taskID);
    return true;

#if 0
    const pb::Conf_t_Task *taskConf = tbx::Task().get_base(taskID);
    if(!taskConf)
    {
        return false;
    }

    DWORD key = 0;
    if(taskConf->task->sub() == Task_Type_Main)
    {
        key = Attr_Main_Task;
    }
    else if(taskConf->task->sub() == Task_Type_Day)
    {
        key = Attr_Day_Task;
    }
    else
    {
        return true;
    }
    std::string now = SceneTimeTick::currentTime.toString();
    DWORD rewardID = 0,rewardNum = 0;
    const std::map<DWORD,DWORD>& tempMap = taskConf->getRewardMap();
    if(!tempMap.empty())
    {
        rewardID = tempMap.begin()->first;
        rewardNum = tempMap.begin()->second;
    }   
    Fir::logger->info("[%s][t_task][f_time=%s][f_char_id=%lu][f_task_name=%s][f_task_award=%u][f_award_count=%u]",now.c_str(),now.c_str(),m_owner->charid,taskConf->task->title().c_str(),rewardID,rewardNum);

    AchieveArg arg(Achieve_Target_Have,Achieve_Sub_Sorce_Num,key,0);
    m_owner->m_achievementManager.target(arg);

    HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
    dailyData->set_finishtask(dailyData->finishtask() + 1);
    TaskArgue arg1(Target_Add_Source,Attr_Finish_Task,Attr_Finish_Task,dailyData->finishtask());
    target(arg1);

    if(key == Attr_Day_Task)
    {
        dailyData->set_finishdailytask(dailyData->finishdailytask() + 1);
        TaskArgue dailyTask(Target_Add_Source,Attr_Finish_Daily_Task,Attr_Finish_Daily_Task,dailyData->finishdailytask());
        target(dailyTask);
    }

    return true;
#endif
}

bool TaskManager::updateTask(const QWORD taskID)
{
    auto iter = m_taskMap.find(taskID);
    if(iter == m_taskMap.end())
    {
        return false;
    }

    HelloKittyMsgData::AckUpdateTask message;
    HelloKittyMsgData::Task *temp = message.mutable_task();
    if(temp)
    {
        *temp = iter->second;
    }

    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool TaskManager::isTaskReward(const QWORD taskID)
{
    auto iter = m_taskMap.find(taskID);
    if(iter == m_taskMap.end())
    {
        return false;
    }
    if(iter->second.status() != HelloKittyMsgData::Task_Award)
    {
        return false;
    }
    return true;
}

bool TaskManager::checkPreTask(const QWORD taskID)
{
    const pb::Conf_t_Task *taskConf = tbx::Task().get_base(taskID);
    if(!taskConf || taskConf->task->pre_level() > m_owner->charbase.level)
    {
        return false;
    }
    const std::set<QWORD> &preTaskSet = taskConf->getPreTaskSet();
    for(auto iter = preTaskSet.begin();iter != preTaskSet.end();++iter)
    {
        if(!isTaskReward(*iter))
        {
            return false;
        }
    }
    return true;
}

bool TaskManager::isAcceptTask(const QWORD taskID)
{
    auto iter = m_taskMap.find(taskID);
    if(iter == m_taskMap.end())
    {
        return false;
    }
    return iter->second.status() == HelloKittyMsgData::Task_Accept;
}

bool TaskManager::acceptTask(const QWORD taskID)
{
    if(m_taskMap.find(taskID) != m_taskMap.end())
    {
        HelloKittyMsgData::Task &task = m_taskMap[taskID];
        task.set_status(HelloKittyMsgData::Task_Accept);
        task.set_current(0);
    }
    else
    {
        HelloKittyMsgData::Task task;
        task.set_id(taskID);
        task.set_status(HelloKittyMsgData::Task_Accept);
        task.set_current(0);
        task.set_total(0);
        m_taskMap.insert(std::pair<QWORD,HelloKittyMsgData::Task>(taskID,task));
    }
    opTypeMap(taskID);

    AchieveArg arg(Achieve_Target_Have,Achieve_Sub_Sorce_Day,Attr_Total_Task,0);
    m_owner->m_achievementManager.target(arg);
    return true;
}

bool TaskManager::openNextTask(const QWORD taskID)
{
    auto iter = pb::Conf_t_Task::s_allPreTaskMap.find(taskID);
    if(iter == pb::Conf_t_Task::s_allPreTaskMap.end())
    {
        return false;
    }

    const std::set<QWORD> &taskSet = iter->second;
    for(auto task_iter = taskSet.begin();task_iter != taskSet.end();++task_iter)
    {
        openTask(*task_iter);
    }
    return true;
}

bool TaskManager::rewardTask(const QWORD taskID)
{
    bool ret = false;
    do
    {
        auto iter = m_taskMap.find(taskID);
        if(iter == m_taskMap.end() || iter->second.status() != HelloKittyMsgData::Task_Finish)
        {
            break;
        }
        const pb::Conf_t_Task *taskConf = tbx::Task().get_base(taskID);
        if(!taskConf)
        {
            break;
        }
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"任务获得(%lu)",taskID);
        const std::map<DWORD,DWORD>& tempMap = taskConf->getRewardMap();
        if(!m_owner->m_store_house.hasEnoughSpace(tempMap))
        {
            m_owner->opErrorReturn(HelloKittyMsgData::WareHouse_Is_Full);
            break;
        }
        m_owner->m_store_house.addOrConsumeItem(taskConf->getRewardMap(),temp,true);
        HelloKittyMsgData::Task &task = const_cast<HelloKittyMsgData::Task&>(iter->second);
        task.set_status(HelloKittyMsgData::Task_Award);

        updateTask(taskID);
        openNextTask(taskID);

        DWORD key = taskConf->task->sub() == Task_Type_Main ? Attr_Main_Task : Attr_Day_Task;
        std::string now = SceneTimeTick::currentTime.toString();
        DWORD rewardID = 0,rewardNum = 0;
        if(!tempMap.empty())
        {
            rewardID = tempMap.begin()->first;
            rewardNum = tempMap.begin()->second;
        }   
        Fir::logger->info("[%s][t_task][f_time=%s][f_char_id=%lu][f_task_name=%s][f_task_award=%u][f_award_count=%u]",now.c_str(),now.c_str(),m_owner->charid,taskConf->task->title().c_str(),rewardID,rewardNum);

        AchieveArg arg(Achieve_Target_Have,Achieve_Sub_Sorce_Num,key,0);
        m_owner->m_achievementManager.target(arg);

        HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
        dailyData->set_finishtask(dailyData->finishtask() + 1);
        TaskArgue arg1(Target_Add_Source,Attr_Finish_Task,Attr_Finish_Task,dailyData->finishtask());
        target(arg1);

        if(key == Attr_Day_Task)
        {
            dailyData->set_finishdailytask(dailyData->finishdailytask() + 1);
            TaskArgue dailyTask(Target_Add_Source,Attr_Finish_Daily_Task,Attr_Finish_Daily_Task,dailyData->finishdailytask());
            target(dailyTask);
        }
        ret = true;
    }while(false);
    return ret;
}

bool TaskManager::opTypeMap(const QWORD taskID,bool opAdd)
{
    const pb::Conf_t_Task *taskConf = tbx::Task().get_base(taskID);
    if(!taskConf)
    {
        return false;
    }

    auto iter = m_taskTypeMap.find(taskConf->task->target_type());
    if(iter == m_taskTypeMap.end())
    {
        if(!opAdd)
        {
            return false;
        }
        std::set<QWORD> temp;
        temp.insert(taskID);
        m_taskTypeMap.insert(std::pair<DWORD,std::set<QWORD>>(taskConf->task->target_type(),temp));
    }
    else
    {
        std::set<QWORD> &temp = const_cast<std::set<QWORD>&>(iter->second);
        if(opAdd)
        {
            temp.insert(taskID);
        }
        else
        {
            temp.erase(taskID);
        }
    }
    return true;
}

HelloKittyMsgData::Task* TaskManager::getTask(const QWORD taskID)
{
    HelloKittyMsgData::Task* task = NULL;
    auto iter = m_taskMap.find(taskID);
    if(iter == m_taskMap.end())
    {
        return task;
    }
    task = const_cast<HelloKittyMsgData::Task*>(&iter->second);
    return task;
}



bool TaskManager::target(const TaskArgue &arg)
{
    auto iter = m_taskTypeMap.find(arg.targerType);
    if(iter == m_taskTypeMap.end())
    {
        return false;
    }
    auto fun = s_tragetCheckMap.find(arg.targerType);
    if(fun == s_tragetCheckMap.end())
    {
        return false;
    }

    const std::set<QWORD> &tempSet = iter->second;
    for(auto temp = tempSet.begin();temp != tempSet.end();++temp)
    {
        HelloKittyMsgData::Task *task = getTask(*temp);
        if(!task || task->status() != HelloKittyMsgData::Task_Accept)
        {
            continue;
        }
        const pb::Conf_t_Task *taskConf = tbx::Task().get_base(*temp);
        if(!taskConf)
        {
            continue;
        }
        TargetRetType targetVal = fun->second(m_owner,taskConf,task,arg);
        if(targetVal == Target_Ret_Update)
        {
            updateTask(*temp);
        }
        else if(targetVal == Target_Ret_Finish)
        {
            finishTask(*temp);
        }
    }
    return true;
}

DWORD TaskManager::getTaskTypeNum(const TaskType &taskType,const HelloKittyMsgData::TaskStatus &status)
{
    DWORD num = 0;
    if(taskType < Task_Type_Main || taskType > Task_Type_Burst)
    {
        return num;
    }
    for(auto iter = m_taskMap.begin();iter != m_taskMap.end();++iter)
    {
        const HelloKittyMsgData::Task &task = iter->second;
        const pb::Conf_t_Task *taskConf = tbx::Task().get_base(task.id());
        if(!taskConf)
        {
            continue;
        }
        if((TaskType)(taskConf->task->sub()) == taskType && task.status() == status)
        {
            ++num;
        }
    }
    return num;
}

DWORD TaskManager::getTaskNum()
{
    return m_taskMap.size();
}

bool TaskManager::resetDailyTask()
{
    for(auto iter = m_taskMap.begin();iter != m_taskMap.end();++iter)
    {
        const pb::Conf_t_Task *taskConf = tbx::Task().get_base(iter->first);
        if(taskConf && taskConf->task->sub() == Task_Type_Day)
        {
            openTask(iter->first);
        }
    }
    return true;
}

void TaskManager::initCheckTragerMap()
{
    if(s_initTargetCheckFlg)
    {
        return;
    }
    s_initTargetCheckFlg = true;
    s_tragetCheckMap.insert(std::pair<DWORD,Task_Target_Check>(Target_Build,checkBuildTarger));
    s_tragetCheckMap.insert(std::pair<DWORD,Task_Target_Check>(Target_InterActive,checkInterActiveTarger));
    s_tragetCheckMap.insert(std::pair<DWORD,Task_Target_Check>(Target_Add_Source,checkSourceTarger));
    s_tragetCheckMap.insert(std::pair<DWORD,Task_Target_Check>(Target_Role_Level,checkRoleLevelTarget));
    s_tragetCheckMap.insert(std::pair<DWORD,Task_Target_Check>(Target_Atlas,checkAtlasTarget));
    s_tragetCheckMap.insert(std::pair<DWORD,Task_Target_Check>(Target_Avartar,checkAvartarTarget));
    s_tragetCheckMap.insert(std::pair<DWORD,Task_Target_Check>(Target_Divine,checkDivineTarget));
    s_tragetCheckMap.insert(std::pair<DWORD,Task_Target_Check>(Target_Toy,checkToyTarget));
    return;
}

bool TaskManager::upLevel()
{
    const std::unordered_map<unsigned int, const pb::Conf_t_Task*> &tbxMap = tbx::Task().getTbxMap();
    for(auto iter = tbxMap.begin();iter != tbxMap.end();++iter)
    {
        HelloKittyMsgData::Task* task = getTask(iter->first);
        if(!task)
        {
            openTask(iter->first);
        }
    }
    return true;
}

bool TaskManager::checkTaskHasDone(const QWORD taskID)
{
    auto iter = m_taskMap.find(taskID);
    if(iter == m_taskMap.end())
    {
        return false;
    }
    if(iter->second.status() != HelloKittyMsgData::Task_Finish)
    {
        return false;
    }
    return true;

}
