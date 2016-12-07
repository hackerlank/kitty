#include "buildManager.h"
#include "SceneUser.h"
#include "tbx.h"
#include "key.h"
#include "warehouse.pb.h"
#include "taskAttr.h"
#include "TimeTick.h"
#include "SceneTaskManager.h"
#include "SceneUserManager.h"
#include "zMemDBPool.h"
#include "SceneToOtherManager.h"
#include "SceneMail.h"
#include "Misc.h"
//建筑工人相关功能

DWORD BuildManager::findIdleWorkerNum()
{
    DWORD num = 0;
    DWORD now = SceneTimeTick::currentTime.sec();
    for(auto iter = m_workerMap.begin();iter != m_workerMap.end();++iter)
    {
        HelloKittyMsgData::Worker &worker = const_cast<HelloKittyMsgData::Worker&>(iter->second);
        if(!worker.starttime())
        {
            ++num;
        }
        if(worker.starttime())
        {
            DWORD lastSec = now - worker.starttime();
            worker.set_lastsec(worker.cd() > lastSec ? worker.cd() - lastSec : 0);
            if(!worker.lastsec())
            {
                workerReset(iter->first);
                ++num;
            }
            updateWork(iter->first);
        }
    }
    return num;
}

bool BuildManager::workerReset(const DWORD id)
{
    auto iter = m_workerMap.find(id);
    if(iter == m_workerMap.end())
    {
        return false;
    }
    HelloKittyMsgData::Worker &worker = const_cast<HelloKittyMsgData::Worker&>(iter->second);
    worker.set_starttime(0);
    worker.set_cd(0);
    worker.set_build(0);
    worker.set_lastsec(0);
    return true;
}

bool BuildManager::updateWork(const DWORD id)
{
    auto iter = m_workerMap.find(id);
    if(iter == m_workerMap.end())
    {
        return false;
    }
    HelloKittyMsgData::AckUpdateWorker message;
    HelloKittyMsgData::Worker *worker = message.mutable_worker();
    if(!worker)
    {
        return false;
    }
    *worker = iter->second;

    std::string ret;
    encodeMessage(&message,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool BuildManager::work(const QWORD buildID,const DWORD cd,const DWORD num)
{
    DWORD size = 0;
    DWORD now = SceneTimeTick::currentTime.sec();
    for(auto iter = m_workerMap.begin();iter != m_workerMap.end() && size < num;++iter)
    {
        HelloKittyMsgData::Worker &worker = const_cast<HelloKittyMsgData::Worker&>(iter->second);
        if(!worker.starttime())
        {
            worker.set_starttime(now);
            worker.set_cd(cd);
            worker.set_build(buildID);
            worker.set_lastsec(cd);
            updateWork(iter->first);
            ++size;
        }
    }
    return size == num;
}

bool BuildManager::flushWorker()
{
    HelloKittyMsgData::AckWorker message;
    DWORD now = SceneTimeTick::currentTime.sec();
    for(auto iter = m_workerMap.begin();iter != m_workerMap.end();++iter)
    {
        HelloKittyMsgData::Worker &worker = const_cast<HelloKittyMsgData::Worker&>(iter->second);
        if(worker.starttime())
        {
            DWORD lastSec = now - worker.starttime();
            worker.set_lastsec(worker.cd() > lastSec ? worker.cd() - lastSec : 0);
            if(!worker.lastsec())
            {
                workerReset(iter->first);
            }
        }

        HelloKittyMsgData::Worker *temp = message.add_worker();
        if(temp)
        {
            *temp = worker;
        }
    }

    std::string ret;
    encodeMessage(&message,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool BuildManager::resetWorkerCD(const DWORD workerID)
{
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"购买工人cd(%u)",workerID);
    if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,ParamManager::getMe().GetSingleParam(eParam_Buy_WorkerCD_Price),temp,false))
    {
        return m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
    }
    if(!workerReset(workerID))
    {
        return false;
    }
    updateWork(workerID);
    return true;
}

bool BuildManager::addWorker()
{
    DWORD workNum = m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Worker);
    workNum += 1;
    m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Worker,1,"升级功能小屋增加",true);
    if(m_workerMap.size() == workNum)
    {
        return false;
    }
    HelloKittyMsgData::Worker worker;
    worker.set_id(workNum);
    worker.set_starttime(0);
    worker.set_cd(0);
    worker.set_lastsec(0);
    worker.set_build(0);
    m_workerMap.insert(std::pair<DWORD,HelloKittyMsgData::Worker>(worker.id(),worker));
    return updateWork(worker.id());
}

