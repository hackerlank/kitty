#include "gardenactivity.h"
#include "tbx.h"
#include "zMisc.h"
#include "TimeTick.h"
#include "buildManager.h"
#include "SceneUser.h"
#include "Misc.h"
#include "zMemDBPool.h"
#include "RedisMgr.h"

ActiveItem::ActiveItem(DWORD Id,QWORD InsId,DWORD NowTimer):m_disappearTimer(0),m_Id(Id),m_InsId(InsId) 
{
    const pb::Conf_t_rubbish *rubbishConf = tbx::rubbish().get_base(m_Id); 
    if(rubbishConf)
    {
        m_disappearTimer = NowTimer + rubbishConf->rubbish->time();
        m_type = static_cast<ACTIVEITEMTYPE>(rubbishConf->rubbish->type());

    }

}


bool  ActiveItem::IsTimerOut(DWORD NowTimer)
{
    return NowTimer >= m_disappearTimer;
}

ActiveCreater::ActiveCreater(ActiveManager &rManager,ACTIVEITEMTYPE type):m_rManager(rManager),m_type(type),m_lastCreateTimer(0)
{
    const std::unordered_map<unsigned int, const pb::Conf_t_rubbish *> &tbxMap = tbx::rubbish().getTbxMap(); 
    for(auto iter = tbxMap.begin();iter != tbxMap.end();++iter)
    {
        ACTIVEITEMTYPE type = static_cast<ACTIVEITEMTYPE>(iter->second->rubbish->type());
        if(type == m_type)
        {
            m_setConfigID.insert(iter->first);
        }
    }

}

ActiveItem * ActiveCreater::Create(DWORD NowTimer)
{
    if(NowTimer < m_lastCreateTimer)
    {
        return NULL;
    }
    DWORD ratelow = 30;
    DWORD ratehigh = 1800;
    std::vector<DWORD> vecRate = PARAM_VEC(eParam_Rubbish_Create);
    if(vecRate.size() < 2)
    {
        Fir::logger->debug("生成垃圾没有配置时间间隔，使用默认参数");
    }
    else
    {
        ratelow = vecRate[0];
        ratehigh = vecRate[1];
    }
    m_lastCreateTimer = NowTimer + zMisc::randBetween(ratelow,ratehigh);
    if(!m_rManager.isHaveFreeRoad())
    {
        return NULL;
    }
    for(auto iter = m_setConfigID.begin();iter != m_setConfigID.end(); iter++)
    {
        const pb::Conf_t_rubbish *rubbishConf = tbx::rubbish().get_base(*iter); 
        if(!rubbishConf)
        {
            continue;

        }
        BYTE rate = zMisc::randBetween(0,100);
        if(rate > rubbishConf->rubbish->rate())
        {
            continue;
        }
        QWORD InsID = m_rManager.getUser().m_buildManager.createItemInRoad(*iter,m_type);
        if(InsID == 0)
        {
            return NULL;//没有空格了，停止创建
        }
        ActiveItem *pItem = new ActiveItem(*iter,InsID,NowTimer);
        if(pItem == NULL)
        {
            m_rManager.getUser().m_buildManager.destroyItemInRoad(InsID,2,m_type);
            return NULL;
        }
        return pItem;

    }
    return NULL;

}

ActiveManager::ActiveManager(SceneUser& rUser):m_rUser(rUser)
{
    for(int it = ACTIVEITEMTYPE_NONE ; it != ACTIVEITEMTYPE_MAX;it++)
    {
        if(it == ACTIVEITEMTYPE_NONE)
        {
            continue;
        }
        ACTIVEITEMTYPE type = static_cast<ACTIVEITEMTYPE>(it);
        ActiveCreater *pCreater = new ActiveCreater(*this,type);
        if(pCreater == NULL)
            continue;
        m_CreateList.push_back(pCreater);


    }
}

void ActiveManager::timerCheck()
{
    DWORD NowTimer = SceneTimeTick::currentTime.sec();
    std::vector<QWORD> delVec;

    for(std::map<QWORD,ActiveItem* >::iterator it = m_ActiveItemmap.begin();it != m_ActiveItemmap.end();++it)
    {
        ActiveItem* pItem =  it->second;
        if(pItem == NULL)
        {
            delVec.push_back(it->first);
        }
        else if(pItem->IsTimerOut(NowTimer))
        {
            getUser().m_buildManager.destroyItemInRoad(it->first,2,it->second->getType());
            delVec.push_back(it->first);
        }
    }

    for(auto iter = delVec.begin();iter != delVec.end();++iter)
    {
        auto findIter = m_ActiveItemmap.find(*iter);
        if(findIter == m_ActiveItemmap.end())
        {
            continue;
        }
        ActiveItem* pItem = const_cast<ActiveItem*>(findIter->second);
        SAFE_DELETE(pItem);
        m_ActiveItemmap.erase(findIter);
    }

    SceneUser* Owner = getUser().m_buildManager.getOwner();
    if(Owner && Owner->havePersonOnline() )//有人才刷物品
    {
        for(auto it = m_CreateList.begin(); it != m_CreateList.end(); it++)
        {
            if(*it == NULL)
            {
                continue;
            }
            if((*it)->getType() == ACTIVEITEMTYPE_SWEET &&  !Owner->haveVisit())
            {
                continue;
            }
            if((*it)->getType() == ACTIVEITEMTYPE_RUBBISH && !Owner->is_online())
            {
                continue;
            }

            ActiveItem *pItem = (*it)->Create(NowTimer);
            if(pItem == NULL)
                continue;
            if(m_ActiveItemmap.insert(std::make_pair(pItem->GetInsID(),pItem)).second == false)
            {
                delete pItem;
            }
        }
    }

}

void ActiveManager::OpRoad(QWORD charid,QWORD InsId,HelloKittyMsgData::AckopBuilding &ack)
{
    do{
        auto it = m_ActiveItemmap.find(InsId);
        if(it ==  m_ActiveItemmap.end())
        {
            ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
            break;

        }
        ActiveItem* pItem =  it->second;
        if(pItem == NULL)
        {
            m_ActiveItemmap.erase(it);
            ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist); 
            break;
        }
        SceneUser* Owner = getUser().m_buildManager.getOwner();
        if(pItem->getType() == ACTIVEITEMTYPE_SWEET  && Owner->charid == charid)
        {
            ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
            break;
        }
        if(pItem->getType() == ACTIVEITEMTYPE_RUBBISH  && Owner->charid != charid)
        {
            ack.set_result(HelloKittyMsgData::PlayerOpEventResult_CaseNoExist);
            break;
        }

        //发奖励
        const pb::Conf_t_rubbish *rubbishConf = tbx::rubbish().get_base(pItem->GetId());
        if(rubbishConf == NULL)
        {
            getUser().m_buildManager.destroyItemInRoad(InsId,2,ACTIVEITEMTYPE_MAX);
            delete pItem;
            m_ActiveItemmap.erase(it);
            ack.set_result(HelloKittyMsgData::PlayerOpEventResult_OtherErr);
            break;
        }
        if(rubbishConf->reward.award_size() > 0)
        {

            for(int i = 0; i != rubbishConf->reward.award_size();i++)
            {
                const HelloKittyMsgData::Award& rcofig = rubbishConf->reward.award(i);
                switch (rcofig.awardtype())
                {
                    case HelloKittyMsgData::Attr_Sweet_Val:
                        {
                            WORD upper = 100;
                            DWORD setUpper =  PARAM_SINGLE(eParam_SweetBoxMax);  
                            if(setUpper == 0)  
                            {
                                Fir::logger->debug("没有设置糖果罐上限，使用默认配置");
                            }
                            else
                            {
                                upper  = setUpper; 
                            }
                            HelloKittyMsgData::Serialize binary;
                            if(!RedisMgr::getMe().get_binary(charid,binary))
                            {
                                Fir::logger->error("无角色%lu",charid);
                                ack.set_result(HelloKittyMsgData::PlayerOpEventResult_OtherErr);
                                break;
                            }

                            UStoreHouse tempUstore(NULL);
                            tempUstore.load(binary);
                            if(tempUstore.getAttr(HelloKittyMsgData::Attr_Sweet_Val) >= upper)
                            {
                                ack.set_result(HelloKittyMsgData::PlayerOpEventResult_BoxFull); 

                            }

                        }
                        break;
                    default:
                        break;
                }

            }
            if(HelloKittyMsgData::PlayerOpEventResult_Suc != ack.result())
            {
                break;
            }

        }
        if(HelloKittyMsgData::PlayerOpEventResult_Suc != ack.result())
        {
            break;
        }

        //加物品:DoTO
        if(rubbishConf->reward.award_size() > 0)
        {

            for(int i = 0; i != rubbishConf->reward.award_size();i++)
            {
                const HelloKittyMsgData::Award& rcofig = rubbishConf->reward.award(i);
                HelloKittyMsgData::Award* pAward = ack.add_award();
                if(pAward )
                {
                    *pAward = rcofig;


                }
            }

        }
        ack.set_process(HelloKittyMsgData::EventProcess_final);
        getUser().m_buildManager.destroyItemInRoad(InsId,3,pItem->getType());
        ack.set_extraid(pItem->GetId());
        delete pItem;
        m_ActiveItemmap.erase(it);
    }while(0);
}



void ActiveManager::destroyRoad(QWORD InsId)
{
    auto it = m_ActiveItemmap.find(InsId);
    if(it ==  m_ActiveItemmap.end())
    {
        return ; 

    }
    SAFE_DELETE(it->second);   
    m_ActiveItemmap.erase(InsId);
}

ActiveManager::~ActiveManager()
{
    for(auto it = m_CreateList.begin(); it != m_CreateList.end();it++)
    {
        SAFE_DELETE(*it);
    }
    for(auto it =m_ActiveItemmap.begin();it != m_ActiveItemmap.end();it++)
    {
        SAFE_DELETE(it->second);
    }
    m_CreateList.clear();
    m_ActiveItemmap.clear();

}

bool  ActiveManager::isHaveFreeRoad()
{
    return getUser().m_buildManager.isHaveFreeRoad();
}

