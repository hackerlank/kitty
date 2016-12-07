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
#include "buildItemProduce.h"
#include "buildTypeProduceGold.h"
#include "buildItemComposite.h"
#include "buildLandMark.h"
#include "Misc.h"

//建筑一般性功能
#if 0
bool BuildManager::gmUpGrade(const DWORD typeID,const DWORD level)
{
    auto iter = m_buildTypeMap.find(typeID);
    if(iter == m_buildTypeMap.end())
    {
        return false;
    }
    const std::set<QWORD> &tempSet = iter->second;
    for(auto temp = tempSet.begin();temp != tempSet.end();++temp)
    {
        BuildBase *build = getBuild(*temp);
        if(!build || build->getLevel() >= level)
        {
            continue;
        }
        for(int index = level - build->getLevel();index > 0;--index)
        {
            if(!build->upGrade(true))
            {
                break;
            }
        }
    }
    return true;
}

bool BuildManager::gmUpGrade(const DWORD level)
{
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        BuildBase *build = getBuild(iter->first);
        if(!build || build->getLevel() >= level)
        {
            continue;
        }
        for(int index = level - build->getLevel();index > 0;--index)
        {
            if(!build->upGrade(true))
            {
                break;
            }
        }
    }
    return true;
}
#endif

bool BuildManager::upGrade(const QWORD tempid,const DWORD effectID)
{
    
    BuildBase *build = getBuild(tempid);
    if(!build)
    {
        return false;
    }

    QWORD key = hashKey(build->getTypeID(),build->getLevel()+1);
    const pb::Conf_t_BuildUpGrade *tempConf = tbx::BuildUpGrade().get_base(key);
    if(!tempConf || !build->upGrade(effectID))
    {
        return false;
    }

    const pb::Conf_t_CarnivalData *carnival = tbx::CarnivalData().get_base(1);
    if(carnival)
    {
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Carnival_Val,carnival->carnival->rewardpoint(),"升级增加",true);
    }
    return true;
}

bool BuildManager::move(const HelloKittyMsgData::ReqBuildMovePlace *message)
{
    BuildBase *build = getBuild(message->tempid());
    if(!build)
    {
        return false;
    }
    bool ret = build->movePlace(message);
    if(ret)
    {
        checkBuildActive();
    }
    return ret;
}

bool BuildManager::checkPreBuild(const DWORD typeID,const DWORD level)
{
    QWORD key = hashKey(typeID,level);
    const pb::Conf_t_building *buildConf = tbx::building().get_base(key);
    if(!buildConf || buildConf->buildInfo->buildkind() == Build_Type_Road)
    {
        Fir::logger->error("[建造建筑错误]:建筑类型id非法 %s,%lu,%u",m_owner->charbase.nickname,m_owner->charid,typeID);
        return false;
    }
    DWORD sum = 0;
    if(m_wareHouseTypeMap.find(typeID) != m_wareHouseTypeMap.end())
    {
        sum += m_wareHouseTypeMap[typeID];
    }
    if(m_buildTypeMap.find(typeID) != m_buildTypeMap.end())
    {
        sum += m_buildTypeMap[typeID].size();
    }
    if(sum >= buildConf->buildInfo->maxcap())
    {
        BuildBase *build = NULL;
        if(m_buildTypeMap.find(typeID) != m_buildTypeMap.end())
        {
            std::set<QWORD> &tempSet = m_buildTypeMap[typeID];
            build = getBuild(*(tempSet.begin()));
        }
        else
        {
            build = (m_buildMap.begin())->second;
        }
        if(build)
        {
            build->opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Build_Type_Enough);
            return false;
        }
    }
    return true;
}

bool BuildManager::asistBuild(BuildBase *build,const bool rationFlg,const HelloKittyMsgData::Point &point,const bool addExpFlg)
{
    if(!addBuild(build))
    {
        SAFE_DELETE(build);
        return false;
    }
    build->setPoint(point);
    if(!build->build(point,false,rationFlg,addExpFlg))
    {
        deleteBuild(build->getID());
        return false;
    }
    initTypeBuild(build);
    TaskArgue arg(Target_Build,Attr_Build,build->getTypeID(),build->getLevel());
    m_owner->m_taskManager.target(arg);
    const pb::Conf_t_CarnivalData *carnival = tbx::CarnivalData().get_base(1);
    if(carnival)
    {
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Carnival_Val,carnival->carnival->rewardpoint(),"建造增加",true);
    }
    return true;
}

bool BuildManager::build(const DWORD typeID,const DWORD level,const bool rationFlg,const HelloKittyMsgData::Point &point,const DWORD createtime)
{
    HelloKittyMsgData::ErrorCodeType errorCode = HelloKittyMsgData::Error_Common_Occupy;
    bool ret = false;
    do
    {
        QWORD key = hashKey(typeID,level);
        const pb::Conf_t_building *buildConf = tbx::building().get_base(key);
        if(!buildConf)
        {
            break;
        }
        //判断建筑数量上限
        if(!checkPreBuild(typeID,level))
        {
            break;
        }
#if 0
        //等级不够
        if(m_owner->charbase.level < buildConf->buildInfo->premiselevel())
        {
            errorCode = HelloKittyMsgData::Build_Premise_Limit;
            break;
        }
#endif
        //解锁
        if(level == 1 && !m_owner->isUnLock(typeID))
        {
            errorCode = HelloKittyMsgData::Build_No_UnLock;
            break;
        }
        //材料
        auto iter = pb::Conf_t_Shop::buildMap.find(typeID);
        if(iter != pb::Conf_t_Shop::buildMap.end())
        {
            const pb::Conf_t_Shop *shopConf = tbx::Shop().get_base(iter->second);
            if(!shopConf)
            {
                break;
            }
            if(shopConf->shop->level() > m_owner->charbase.level)
            {
                errorCode = HelloKittyMsgData::Build_Premise_Limit;
                break;
            }
            if(shopConf->shop->popular() > m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Now))
            {
                m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Popular_Now);
                break;
            }

            if(!m_owner->checkMaterialMap(shopConf->getPriceMap(),true))
            {
                break;
            }
        }
        if(buildConf->buildInfo->costpopular())
        {
            //判断人气值
            if(buildConf->buildInfo->costpopular() + m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Now) + m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Now_Temp) > m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Max))
            {
                m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Popular_Max);
                break;
            }
        }

        BuildBase *build = newBuild(buildConf,NULL,createtime);
        if(!build)
        {
            break;
        }
        //装饰物就直接可用了
        if(buildConf->buildInfo->buildkind() == Build_Type_Decorate)
        {
            build->setLastCDSec(0);
            build->setMark(HelloKittyMsgData::Build_Status_Click_Active);
        }
        else
        {
            if(build->getLastCDSec() == 0)
            {
                build->setMark(HelloKittyMsgData::Build_Status_Finish);
            }
        }
        bool ret = asistBuild(build,rationFlg,point);
        if(ret)
        {
            m_owner->m_atlasManager.addAtlasByBuild(build->getTypeID(),build->getLevel());
        }

        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"建造建筑消耗(%u,%u)",typeID,level);

        //扣除材料
        iter = pb::Conf_t_Shop::buildMap.find(typeID);
        if(iter != pb::Conf_t_Shop::buildMap.end())
        {
            const pb::Conf_t_Shop *shopConf = tbx::Shop().get_base(iter->second);
            if(!shopConf)
            {
                break;
            }
            m_owner->m_store_house.addOrConsumeItem(shopConf->getPriceMap(),temp,false);
        }

        //处理人气值
        bzero(temp,sizeof(temp));
        snprintf(temp,sizeof(temp),"建造建筑增加(%u,%u)",typeID,level);
        if(buildConf->buildInfo->costpopular())
        {
            if((build->getMark() & HelloKittyMsgData::Build_Status_Click_Active) || (build->getMark() & HelloKittyMsgData::Build_Status_Normal))
            {
                m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Now,buildConf->buildInfo->costpopular(),temp,true);
            }
            else
            {
                m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Now_Temp,buildConf->buildInfo->costpopular(),temp,true);
            }
        }
        if(buildConf->buildInfo->addpopular())
        {
            if(build->getMark() & HelloKittyMsgData::Build_Status_Click_Active)
            {
                m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Max,buildConf->buildInfo->addpopular(),temp,true);
            }
        }

        //如果是试衣间，那就给时装吧
        if(typeID == 10010040)
        {
            m_owner->m_dressManager.addDressByItem(20010030);
        }
        if(build->getMark() & HelloKittyMsgData::Build_Status_Click_Active)
        {
            m_owner->checkFunOpen(pb::Conf_t_openfun::eOpen_Build,build->getTypeID());
            m_owner->triggerTaskGuid(pb::Conf_t_Guide::TIGGERTYPE_BUILD,build->getTypeID());
        }
        //集市,激活npc摊位,初始化npc摊位
        m_owner->m_trade.openNpcStall();

        ret = true;
        std::string now = SceneTimeTick::currentTime.toString();
        Fir::logger->info("[%s][t_build][f_time=%s][f_char_id=%lu][f_build_name=%s][f_build_level=%u]",now.c_str(),now.c_str(),m_owner->charid,buildConf->buildInfo->name().c_str(),level);
    }while(false);

    if(!ret && errorCode != HelloKittyMsgData::Error_Common_Occupy)
    {
        m_owner->opErrorReturn(errorCode);
    }
    return ret;
}

bool BuildManager::buildRoad(const HelloKittyMsgData::ReqBuildRoad *cmd)
{
    DWORD typeID = cmd->type();
    QWORD key = hashKey(typeID,1);
    const pb::Conf_t_building *buildConf = tbx::building().get_base(key);
    if(!buildConf)
    {
        Fir::logger->error("[建造建筑错误]:建筑类型id非法 %s,%lu,%u",m_owner->charbase.nickname,m_owner->charid,typeID);
        return false;
    }

    //购买材料
    std::map<DWORD,DWORD> priceMap;
    auto iter = pb::Conf_t_Shop::buildMap.find(typeID);
    if(iter != pb::Conf_t_Shop::buildMap.end())
    {
        const pb::Conf_t_Shop *shopConf = tbx::Shop().get_base(iter->second);
        if(!shopConf)
        {
            return false;
        }
        for(auto itr = shopConf->getPriceMap().begin();itr != shopConf->getPriceMap().end();++itr)
        {
            priceMap.insert(std::pair<DWORD,DWORD>(itr->first,itr->second * cmd->point_size()));
        }
        if(!m_owner->checkMaterialMap(shopConf->getPriceMap(),true))
        {
            m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gold);
            return false;
        }
    }
    bool ret = true;
    std::vector<QWORD> buildRoadVec;
    for(int index = 0;index < cmd->point_size();++index)
    {
        BuildBase *build = newBuild(buildConf,NULL,SceneTimeTick::currentTime.sec());
        if(!build)
        {
            ret = false ;
            break;
        }
        if(!addBuild(build))
        {
            SAFE_DELETE(build);
            ret = false;
            break;
        }
        buildRoadVec.push_back(build->getID());
        const HelloKittyMsgData::Point &point = cmd->point(index);
        build->setPoint(point);
        if(!build->build(point,false,false))
        {
            ret = false;
            break;
        }
        build->setLastCDSec(0);
        build->setMark(HelloKittyMsgData::Build_Status_Normal);
        m_emptyRoadSet.insert(build->getID());
    }
    if(!ret)
    {
        for(auto iter = buildRoadVec.begin();iter != buildRoadVec.end();++iter)
        {
            deleteBuild(*iter);
            m_emptyRoadSet.erase(*iter);
        }
    }
    else
    {
        if(!priceMap.empty())
        {
            char temp[100] = {0};
            snprintf(temp,sizeof(temp),"建造道路消耗(%u,%u)",typeID,1);
            m_owner->reduceMaterialMap(priceMap,temp);
        }
#if 0
        TaskArgue arg(Target_Build,Attr_Build,typeID,1);
        m_owner->m_taskManager.target(arg);
#endif
    }
    checkBuildActive();
    return true;
}

bool BuildManager::clearRoad(const HelloKittyMsgData::ReqClearRoad *cmd)
{
    for(int index = 0;index < cmd->tempid_size();++index)
    {
        if(m_rubbishMap.find(cmd->tempid(index)) != m_rubbishMap.end())
        {
            m_rubbishMap.erase(cmd->tempid(index));
        }
        if(m_burstEventMap.find(cmd->tempid(index)) != m_burstEventMap.end())
        {
            m_owner->m_burstEventManager.delEvent(cmd->tempid(index),HelloKittyMsgData::BES_Del_Road);
        }
        m_owner->m_activeManger.destroyRoad(cmd->tempid(index));
        deleteBuild(cmd->tempid(index));
        m_emptyRoadSet.erase(cmd->tempid(index));
    }
    checkBuildActive();
    return true;
}

bool BuildManager::build(const WareHouseBuildBase &buildBase,const bool rationFlg,const HelloKittyMsgData::Point &point)
{
    if(!checkPreBuild(buildBase.typeID,buildBase.level))
    {
        return false;
    }
    QWORD key = hashKey(buildBase.typeID,buildBase.level);
    const pb::Conf_t_building *buildConf = tbx::building().get_base(key);
    BuildBase *build = newBuild(buildConf,NULL,0);
    if(!buildConf || !build)
    {
        SAFE_DELETE(build);
        return false;
    }
    build->loadWareHouseBuildBase(buildBase);
    build->setFromType(HelloKittyMsgData::BFT_Warehouse);
    build->setMark(buildBase.mark);
    if(asistBuild(build,rationFlg,point,false))
    {
        wareInitProduce(build,buildBase.cellID);
        build->setFromType(HelloKittyMsgData::BFT_Normal);
        opBuildExp(build,true);
        if(buildBase.inBuildID)
        {
            m_inBuildIDMap.insert(std::pair<QWORD,QWORD>(buildBase.inBuildID,build->getID()));
        }
        if(!buildBase.produceTime)
        {
            //处理人气值
            char temp[100];
            bzero(temp,sizeof(temp));
            snprintf(temp,sizeof(temp),"建造建筑增加(%u,%u)",buildBase.typeID,buildBase.level);
            if(buildConf->buildInfo->costpopular())
            {
                m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Now,buildConf->buildInfo->costpopular(),temp,true);
            }
            if(buildConf->buildInfo->addpopular())
            {
                 m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Max,buildConf->buildInfo->addpopular(),temp,true);
            }
        }
    }
    return true;
}

bool BuildManager::wareInitProduce(BuildBase *build,const DWORD cellid)
{
    if(build->isTypeBuild(Build_Type_Gold_Produce))
    {
        auto iter = m_wareProduceOther.find(cellid);
        if(iter == m_wareProduceOther.end())
        {
            return false;
        }
        BuildTypeProduceGold *buildType = dynamic_cast<BuildTypeProduceGold*>(build);
        if(buildType)
        {
            buildType->initProduce(iter->second);
            buildType->updateProduce();
        }
        m_wareProduceOther.erase(iter);
        return true;
    }
    if(build->isTypeBuild(Build_Type_Item_Produce))
    {
        auto iter = m_wareProduceItemMap.find(cellid);
        if(iter == m_wareProduceItemMap.end())
        {
            return false;
        }
        const HelloKittyMsgData::WareHouseProduceInfo &temp = iter->second;
        BuildTypeProduceItem *buildType = dynamic_cast<BuildTypeProduceItem*>(build);
        if(buildType)
        {
            buildType->load(temp.produceinfo());
            buildType->sendInfoMeg();
        }
        m_wareProduceItemMap.erase(iter);
        return true;
    }
    if(build->isTypeBuild(Build_Type_Item_Composite))
    {
        auto iter = m_wareCompositeItemMap.find(cellid);
        if(iter == m_wareCompositeItemMap.end())
        {
            return false;
        }
        const HelloKittyMsgData::WareHouseCompositeInfo &temp = iter->second;
        BuildTypeCompositeItem *buildType = dynamic_cast<BuildTypeCompositeItem*>(build);
        if(buildType)
        {
            buildType->load(temp.compositeinfo());
            buildType->sendInfoMeg();
        }
        m_wareCompositeItemMap.erase(iter);
        return true;
    }
    return false;
}


