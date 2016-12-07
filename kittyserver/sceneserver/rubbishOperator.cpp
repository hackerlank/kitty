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
//垃圾相关的功能

//随机找个位置放下垃圾，参数垃圾id,返回道路id
QWORD BuildManager::createItemInRoad(DWORD Id,ACTIVEITEMTYPE type)
{
    QWORD roadID = 0;
    BuildBase* road = getEmptyRoad();
    if(road)
    {
        if(updateRoadRubbish(road->getID(),Id,1,type))
        {
            roadID = road->getID();
        }
    }
    return roadID;
}

bool BuildManager::updateRoadRubbish(const QWORD roadID,const DWORD rubbishType,const BYTE opType,ACTIVEITEMTYPE type)
{
    HelloKittyMsgData::AckUpdateRubbish ackRubbish;
    ackRubbish.set_updatecharid(m_owner->charid);
    ackRubbish.mutable_rubbish()->set_roadid(roadID);
    ackRubbish.mutable_rubbish()->set_rubbish(rubbishType);
    ackRubbish.set_optype(opType);

    bool result = true;
    if(opType == 1)
    {
        std::pair<std::map<QWORD,DWORD>::iterator,bool> ret = m_rubbishMap.insert(std::pair<QWORD,DWORD>(roadID,rubbishType));
        result = ret.second;
        ackRubbish.mutable_rubbish()->set_rubbish(rubbishType);
        m_emptyRoadSet.erase(roadID);
    }
    else
    {
        auto iter = m_rubbishMap.find(roadID);
        if(iter == m_rubbishMap.end())
        {
            result = false;
        }
        else
        {
            ackRubbish.mutable_rubbish()->set_rubbish(iter->second);
            m_rubbishMap.erase(iter);
        }
        m_emptyRoadSet.insert(roadID);
    }
    if(result)
    {
        std::string ret;
        encodeMessage(&ackRubbish,ret);
        if(type == ACTIVEITEMTYPE_SWEET)
        {
            m_owner->sendOtherUserMsg(ret.c_str(),ret.size());
        }
        else if(type == ACTIVEITEMTYPE_RUBBISH)
        {
            m_owner->sendCmdToMe(ret.c_str(),ret.size());
        }
        else
        {
            m_owner->broadcastMsgInMap(ret.c_str(),ret.size());
        }
    }
    return result;
}


//删除某条道路的垃圾
void BuildManager::destroyItemInRoad(QWORD Id,const BYTE delType,ACTIVEITEMTYPE type)
{
    BuildBase *road= getBuild(Id);
    if(road)
    {
        updateRoadRubbish(road->getID(),0,delType, type);
    }
    return;    
}

//是否有空道路可放垃圾
bool BuildManager::isHaveFreeRoad()
{
    BuildBase* road = getEmptyRoad();
    return road ? true : false;
}


bool BuildManager::isRoad(const QWORD tempid)
{
    BuildBase *buildBase = getBuild(tempid);
    if(!buildBase)
    {
        return false;
    }
    return buildBase->isTypeBuild(Build_Type_Road);
}

BuildBase* BuildManager::getEmptyRoad()
{
    if(m_emptyRoadSet.empty())
    {
        return NULL;
    }
    std::vector<QWORD> tempVec(m_emptyRoadSet.begin(),m_emptyRoadSet.end());
    DWORD rand = zMisc::randBetween(0,tempVec.size() -1);
    return getBuild(tempVec[rand]);
}

bool BuildManager::rubbishInRoad(const QWORD roadID)
{
    return m_rubbishMap.find(roadID) != m_rubbishMap.end();
}

bool BuildManager::opBurstEventMap(const QWORD roadID,const DWORD npcID,const bool opAdd)
{
    auto iter = m_burstEventMap.find(roadID);
    if(iter == m_burstEventMap.end())
    {
        if(opAdd)
        {
            m_burstEventMap.insert(std::pair<QWORD,DWORD>(roadID,npcID));
            m_emptyRoadSet.erase(roadID);
        }
    }
    else
    {
        if(!opAdd)
        {
            m_burstEventMap.erase(roadID);
            m_emptyRoadSet.insert(roadID);
        }
    }
    return true;
}



