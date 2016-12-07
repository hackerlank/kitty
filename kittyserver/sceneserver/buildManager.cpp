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
#include "buildLandMark.h"
#include "buildItemComposite.h"
#include "GmTool.h"
#include "buffer.h"

BuildManager::BuildManager(SceneUser *owner) : m_owner(owner)
{
    m_maxCellID = 0;
    m_wareHouseBuildMap.clear();
    m_cellSet.clear();
}

BuildManager::~BuildManager()
{
    m_owner = NULL;
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        BuildBase *build = iter->second;
        if(build)
        {
            SAFE_DELETE(build);
        }
    }
    m_buildMap.clear();
}

void BuildManager::reset()
{
    m_workerMap.clear();
    m_buildMap.clear();
    m_buildTypeMap.clear();
    m_wareHouseBuildMap.clear();
}

BuildBase* BuildManager::newBuild(const pb::Conf_t_building *argConf,const HelloKittyMsgData::BuildBase *buidBase,const DWORD dwCreateTime)
{
    BuildBase *build = NULL;
    if(!((argConf && !buidBase) || (!argConf && buidBase)))
    {
        return build;
    }
    const pb::Conf_t_building *buildConf = argConf;
    if(!buildConf)
    {
        QWORD key = hashKey(buidBase->type(),1);
        buildConf = tbx::building().get_base(key);
        if(!buildConf)
        {
            Fir::logger->error("[建筑错误]:新建建筑信息错误 %s,%lu,%u,%u",m_owner->charbase.nickname,m_owner->charid,buidBase->type(),buidBase->level());
            return build;
        }
    }
    switch(buildConf->buildInfo->buildkind())
    {
        case Build_Type_Item_Produce:
            {
                build = buidBase ? new BuildTypeProduceItem(m_owner,*buidBase) : new BuildTypeProduceItem(m_owner,buildConf);
            }
            break;
        case Build_Type_Gold_Produce:
            {
                build = buidBase ? new BuildTypeProduceGold(m_owner,*buidBase) : new BuildTypeProduceGold(m_owner,buildConf);
            }
            break;
        case Build_Type_Land_Mark:
            {
                build = buidBase ? new BuildTypeLandMark(m_owner,*buidBase) : new BuildTypeLandMark(m_owner,buildConf);
            }
            break;
        case Build_Type_Item_Composite:
            {
                build = buidBase ? new BuildTypeCompositeItem(m_owner,*buidBase) : new BuildTypeCompositeItem(m_owner,buildConf);
            }
            break;
        default:
            {
                build = buidBase ? new BuildBase(m_owner,*buidBase) : new BuildBase(m_owner,buildConf);
            }
            break;
    }

    if(build && dwCreateTime > 0 && argConf)
    {
        build->SetCreateTimer(dwCreateTime);
        build->setLastCDSec(argConf->buildInfo->time());
    }
    return build;
}

bool BuildManager::init(const pb::Conf_t_newRoleAttr *confBase)
{
    const std::set<pb::InitBuildInfo> buildLevelSet = confBase->getBuildLevelSet();
    std::map<DWORD,std::vector<Point> > tepmapRoad;
    for(auto iter = buildLevelSet.begin();iter != buildLevelSet.end();++iter)
    {
        const pb::InitBuildInfo &initBuildInfo = *iter;
        QWORD key = hashKey(initBuildInfo.buildID,1);
        const pb::Conf_t_building *buildConf = tbx::building().get_base(key);
        if(!buildConf)
        {
            Fir::logger->error("[建筑错误]:新建角色属性中建筑信息错误 %s,%lu,%u,%u",m_owner->charbase.nickname,m_owner->charid,initBuildInfo.buildID,initBuildInfo.buildLevel);
            return false;
        }
        if(buildConf->buildInfo->buildkind() == Build_Type_Road)
        {
            auto it = tepmapRoad.find(initBuildInfo.buildID);
            if(it == tepmapRoad.end())
            {
                std::vector<Point> tempVec;
                tepmapRoad.insert(std::pair<DWORD,std::vector<Point>>(initBuildInfo.buildID,tempVec));
            }
            it = tepmapRoad.find(initBuildInfo.buildID); 
            if(it != tepmapRoad.end())
            {
                std::vector<Point> &tempVec = const_cast<std::vector<Point>&>(it->second);
                tempVec.push_back(initBuildInfo.point);
            }
            continue;
        }
        BuildBase *build = newBuild(buildConf,NULL,SceneTimeTick::currentTime.sec());
        if(!build)
        {
            return false;
        }
        build->setPoint(initBuildInfo.point);
        if(!addBuild(build))
        {
            SAFE_DELETE(build);
            return false;
        }
        if(!build->build(build->getPoint(),true))
        {
            deleteBuild(build->getID());
            return false;
        }
        build->setLastCDSec(0);
        build->setMark(HelloKittyMsgData::Build_Status_Click_Active);
        m_owner->m_atlasManager.addAtlasByBuild(build->getTypeID(),build->getLevel());
        initTypeBuild(build);
    }
    Fir::logger->debug("[建筑初始化] 创建道路开始(%lu,%s,%lu)",m_owner->charid,m_owner->charbase.nickname,tepmapRoad.size());
    HelloKittyMsgData::ReqBuildRoad roadMsg;
    for(auto iter = tepmapRoad.begin();iter != tepmapRoad.end();++iter)
    {
        roadMsg.set_type(iter->first);
        for(auto it = iter->second.begin(); it != iter->second.end();it++)
        {
            const Point &pt = *it;
            HelloKittyMsgData::Point *tempPt = roadMsg.add_point();
            if(tempPt)
            {
                tempPt->set_x(pt.x);
                tempPt->set_y(pt.y);
            }
        }
        buildRoad(&roadMsg);
    }
    Fir::logger->debug("[建筑初始化] 创建道路结束(%lu,%s,%lu)",m_owner->charid,m_owner->charbase.nickname,tepmapRoad.size());
    checkBuildActive();
    return true;
}

bool BuildManager::addBuild(BuildBase *build)
{
    if(!build)
    {
        return false;
    }

    auto iter = m_buildMap.find(build->getID());
    if(iter != m_buildMap.end())
    {
        Fir::logger->error("[建筑错误]:建筑id分配器重复 %s,%lu,%lu",m_owner->charbase.nickname,m_owner->charid,BuildBase::generateID);
        return false;
    }
    m_buildMap.insert(std::pair<QWORD,BuildBase*>(build->getID(),build));

    auto iter1 = m_buildTypeMap.find(build->getTypeID());
    if(iter1 == m_buildTypeMap.end())
    {
        std::set<QWORD> tempIdSet;
        m_buildTypeMap.insert(std::pair<DWORD,std::set<QWORD>>(build->getTypeID(),tempIdSet));
    }
    std::set<QWORD> &tempIdSet = m_buildTypeMap[build->getTypeID()];
    if(tempIdSet.find(build->getID()) != tempIdSet.end())
    {
        Fir::logger->error("[建筑错误]:建筑id分配器重复 %s,%lu,%lu",m_owner->charbase.nickname,m_owner->charid,BuildBase::generateID);
        return false;
    }
    tempIdSet.insert(build->getID());
    opKindType(build->getID());
    return true;
}

bool BuildManager::deleteBuild(const QWORD tempid)
{
    BuildBase *build = getBuild(tempid);
    if(!build)
    {
        return true;
    }
    if(build->m_inBuildID)
    {
        m_inBuildIDMap.erase(build->m_inBuildID);
    }
    m_owner->m_kittyGarden.eraseBuildPoint(build);
    opKindType(tempid,false);
    eraseBuild(build);
    SAFE_DELETE(build);
    m_buildMap.erase(tempid);

    HelloKittyMsgData::AckRemoveBuid removeBuild;
    removeBuild.set_updatecharid(m_owner->charid);
    removeBuild.set_tempid(tempid);
    std::string ret;
    encodeMessage(&removeBuild,ret);
    m_owner->broadcastMsgInMap(ret.c_str(),ret.size());
    return true;
}

bool BuildManager::eraseBuild(BuildBase *build)
{
    auto iter = m_buildMap.find(build->getID());
    if(iter == m_buildMap.end())
    {
        return false;
    }

    auto iter1 = m_buildTypeMap.find(build->getTypeID());
    if(iter1 == m_buildTypeMap.end())
    {
        return false;
    }
    std::set<QWORD> &tempIdSet = m_buildTypeMap[build->getTypeID()];
    if(tempIdSet.find(build->getID()) == tempIdSet.end())
    {
        return false;
    }
    tempIdSet.erase(build->getID());
    return true;
}

bool BuildManager::load(const HelloKittyMsgData::Serialize& binary)
{
    reset();
    for(int index = 0;index < binary.buildbase_size();++index)
    {
        const HelloKittyMsgData::BuildBase &buidBase = binary.buildbase(index);
        BuildBase *build = newBuild(NULL,&buidBase,0);
        if(!addBuild(build))
        {
            SAFE_DELETE(build);
            return false;
        }
        m_oldNewKey.insert(std::pair<QWORD,QWORD>(buidBase.tempid(),build->getID()));
        if(build->isTypeBuild(Build_Type_Road))
        {
            m_emptyRoadSet.insert(build->getID());
        }
        if(build->m_inBuildID)
        {
            m_inBuildIDMap.insert(std::pair<QWORD,QWORD>(build->m_inBuildID,build->getID()));
        }
    }
    for(int index = 0;index < binary.buildproduce_size();++index)
    {
        const HelloKittyMsgData::BuildProduce &temp = binary.buildproduce(index);
        auto iter = m_oldNewKey.find(temp.tempid());
        if(iter == m_oldNewKey.end())
        {
            continue;
        }
        BuildBase *build = getBuild(iter->second);
        if(build && build->isTypeBuild(Build_Type_Gold_Produce))
        {
            BuildTypeProduceGold *buildType = dynamic_cast<BuildTypeProduceGold*>(build);
            if(buildType)
            {
                buildType->load(temp);
            }
        }
    }
    for(int index = 0;index < binary.produceinfo_size();++index)
    {
        const HelloKittyMsgData::ProduceInfo &temp = binary.produceinfo(index);
        auto iter = m_oldNewKey.find(temp.tempid());
        if(iter == m_oldNewKey.end())
        {
            continue;
        }
        BuildBase *build = getBuild(iter->second);
        if(!build)
        {
            continue;
        }
        BuildTypeProduceItem *func = dynamic_cast<BuildTypeProduceItem*>(build);
        if(func)
        {
            func->load(temp);
        }
    }
    for(int index = 0;index < binary.compositeinfo_size();++index)
    {
        const HelloKittyMsgData::CompositeInfo &temp = binary.compositeinfo(index);
        auto iter = m_oldNewKey.find(temp.tempid());
        if(iter == m_oldNewKey.end())
        {
            continue;
        }
        BuildBase *build = getBuild(iter->second);
        if(!build)
        {
            continue;
        }
        BuildTypeCompositeItem *func = dynamic_cast<BuildTypeCompositeItem*>(build);
        if(func)
        {
            func->load(temp);
        }
    }
    for(int index = 0;index < binary.usecard_size();++index)
    {
        const HelloKittyMsgData::UseCardInfo &temp = binary.usecard(index);
        auto iter = m_oldNewKey.find(temp.tempid());
        if(iter == m_oldNewKey.end())
        {
            continue;
        }
        BuildBase *build = getBuild(iter->second);
        if(build)
        {
            build->loadCard(temp);
            opCardSet(build->getID(),temp.cardid());
        }
    }
    std::map<DWORD,DWORD> oldNewCellMap;
    for(int index = 0;index < binary.warebuild_size();++index)
    {
        const HelloKittyMsgData::WareHouseBuildBase &wareBuild = binary.warebuild(index);
        m_maxCellID = m_maxCellID < wareBuild.cellid() ? wareBuild.cellid() : m_maxCellID;
        WareHouseBuildBase wareHouseBuild(wareBuild.cellid(),wareBuild.type(),wareBuild.level(),wareBuild.producetime(),wareBuild.status(),wareBuild.durtime(),wareBuild.num());
        wareHouseBuild.inBuildID = wareBuild.inbuildid();
        wareHouseBuild.inBuildLevel = wareBuild.inbuildlevel();
        wareHouseBuild.friendID = wareBuild.friendid();
        m_wareHouseBuildMap.insert(std::pair<DWORD,WareHouseBuildBase>(wareBuild.cellid(),wareHouseBuild));
        m_cellSet.insert(wareBuild.cellid());
        oldNewCellMap.insert(std::pair<DWORD,DWORD>(wareBuild.cellid(),wareHouseBuild.cellID));
        QWORD key = hashKey(wareBuild.type(),1);

#if 0
        const pb::Conf_t_building *confBase = tbx::building().get_base(key);
        if(confBase && confBase->buildInfo->buildkind() == Build_Type_Decorate)
        {
            if(m_keyIDMap.find(key) == m_keyIDMap.end())
            {
                m_keyIDMap.insert(std::pair<QWORD,DWORD>(key,wareHouseBuild.cellID));
            }
        }
#endif
        auto itr = m_keyIDMap.find(key);
        if(itr == m_keyIDMap.end())
        {
            std::set<DWORD> tempSet;
            tempSet.insert(wareHouseBuild.cellID);
            m_keyIDMap.insert(std::pair<QWORD,std::set<DWORD> >(key,tempSet));
        }
        else
        {
            itr->second.insert(wareHouseBuild.cellID);
        }

        if(wareHouseBuild.inBuildID)
        {
            auto it = m_inBuildWareMap.find(key);
            if(it == m_inBuildWareMap.end())
            {
                std::set<DWORD> tempSet;
                tempSet.insert(wareHouseBuild.cellID);
                m_inBuildWareMap.insert(std::pair<QWORD,std::set<DWORD> >(key,tempSet));
            }
            else
            {
                it->second.insert(wareHouseBuild.cellID);
            }
            m_inBuildIDWareMap.insert(std::pair<QWORD,DWORD>(wareHouseBuild.inBuildID,wareHouseBuild.cellID));
        }
    }
    for(int index = 0;index < binary.wareproduceinfo_size();++index)
    {
        HelloKittyMsgData::WareHouseProduceInfo &temp = const_cast<HelloKittyMsgData::WareHouseProduceInfo&>(binary.wareproduceinfo(index));
        if(oldNewCellMap.find(temp.cellid()) == oldNewCellMap.end())
        {
            continue;
        }
        temp.set_cellid(oldNewCellMap[temp.cellid()]);
        m_wareProduceItemMap.insert(std::pair<DWORD,HelloKittyMsgData::WareHouseProduceInfo>(temp.cellid(),temp));
    }
    for(int index = 0;index < binary.warecompositeinfo_size();++index)
    {
        HelloKittyMsgData::WareHouseCompositeInfo &temp = const_cast<HelloKittyMsgData::WareHouseCompositeInfo&>(binary.warecompositeinfo(index));
        if(oldNewCellMap.find(temp.cellid()) == oldNewCellMap.end())
        {
            continue;
        }
        temp.set_cellid(oldNewCellMap[temp.cellid()]);
        m_wareCompositeItemMap.insert(std::pair<DWORD,HelloKittyMsgData::WareHouseCompositeInfo>(temp.cellid(),temp));
    }
    for(int index = 0;index < binary.wareotherinfo_size();++index)
    {
        HelloKittyMsgData::WareHouseOtherInfo &temp = const_cast<HelloKittyMsgData::WareHouseOtherInfo&>(binary.wareotherinfo(index));
        if(oldNewCellMap.find(temp.cellid()) == oldNewCellMap.end())
        {
            continue;
        }
        temp.set_cellid(oldNewCellMap[temp.cellid()]);
        m_wareProduceOther.insert(std::pair<DWORD,HelloKittyMsgData::WareHouseOtherInfo>(temp.cellid(),temp));
    }
    for(int index = 0;index < binary.buildeffect_size();++index)
    {
        const HelloKittyMsgData::BuildEffect &temp = binary.buildeffect(index);
        auto iter = m_oldNewKey.find(temp.tempid());
        if(iter == m_oldNewKey.end())
        {
            continue;
        }
        BuildBase *build = getBuild(iter->second);
        if(build)
        {
            build->loadEffect(temp);
        }
    }

#if 0
    for(int index = 0;index < binary.worker_size();++index)
    {
        const HelloKittyMsgData::Worker &worker = binary.worker(index);
        m_workerMap.insert(std::pair<DWORD,HelloKittyMsgData::Worker>(worker.id(),worker));
    }
#endif
    loadBuildInMap();
#if 0
    logPoint();
#endif
    return true;
}

bool BuildManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        BuildBase *build = iter->second;
        if(build)
        {
            HelloKittyMsgData::BuildBase *buildBase = binary.add_buildbase();
            if(buildBase)
            {
                build->save(buildBase,true);
                build->saveProduce(binary);
                build->saveCard(binary);
                build->saveEffect(binary);
            }
        }
    }
#if 0
    for(auto iter = m_workerMap.begin();iter != m_workerMap.end();++iter)
    {
        HelloKittyMsgData::Worker *worker = binary.add_worker();
        *worker = iter->second;
    }
#endif
    for(auto iter = m_wareHouseBuildMap.begin();iter != m_wareHouseBuildMap.end();++iter)
    {
        HelloKittyMsgData::WareHouseBuildBase *wareBuild = binary.add_warebuild();
        if(wareBuild)
        {
            const WareHouseBuildBase& wareBuildInfo = iter->second;
            wareBuild->set_type(wareBuildInfo.typeID);
            wareBuild->set_level(wareBuildInfo.level);
            wareBuild->set_num(wareBuildInfo.num);
            wareBuild->set_cellid(wareBuildInfo.cellID);
            wareBuild->set_status(wareBuildInfo.mark);
            wareBuild->set_producetime(wareBuildInfo.produceTime);
            wareBuild->set_durtime(wareBuildInfo.durTime);
            wareBuild->set_inbuildid(wareBuildInfo.inBuildID);
            wareBuild->set_inbuildlevel(wareBuildInfo.inBuildLevel);
            wareBuild->set_friendid(wareBuildInfo.friendID);
        }
    }
    for(auto iter = m_wareProduceItemMap.begin();iter != m_wareProduceItemMap.end();++iter)
    {
        HelloKittyMsgData::WareHouseProduceInfo *temp = binary.add_wareproduceinfo();
        if(temp)
        {
            *temp = iter->second;
        }
    }
    for(auto iter = m_wareCompositeItemMap.begin();iter != m_wareCompositeItemMap.end();++iter)
    {
        HelloKittyMsgData::WareHouseCompositeInfo *temp = binary.add_warecompositeinfo();
        if(temp)
        {
            *temp = iter->second;
        }
    }
    for(auto iter = m_wareProduceOther.begin();iter != m_wareProduceOther.end();++iter)
    {
        HelloKittyMsgData::WareHouseOtherInfo *temp = binary.add_wareotherinfo();
        if(temp)
        {
            *temp = iter->second;
        }
    }
    return true;
}

bool BuildManager::flushAllBuild()
{
    HelloKittyMsgData::AckFlushAllBuild flushAllBuild;
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        HelloKittyMsgData::BuildBase *buildBase = flushAllBuild.add_buildinfo();
        BuildBase *build = iter->second;
        if(buildBase && build)
        {
            build->save(buildBase);
        }
    }

    std::string ret;
    if(encodeMessage(&flushAllBuild,ret))
    {
        m_owner->sendCmdToMe(ret.c_str(),ret.size());
    }
    return true;
}

bool BuildManager::fullMessage(HelloKittyMsgData::UserBaseInfo& userInfo)
{
    for(auto iter = m_buildTypeMap.begin();iter != m_buildTypeMap.end();++iter)
    {
        const set<QWORD> &idSet = iter->second;
        for(auto it = idSet.begin();it != idSet.end();++it)
        {
            BuildBase *build = getBuild(*it);
            if(!build)
            {
                continue;
            }
            HelloKittyMsgData::BuildBase *buildBase = userInfo.add_buildinfo();
            if(buildBase)
            {
                build->save(buildBase);
            }
            build->flushEffect(userInfo);
        }
    }
#if 0
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        HelloKittyMsgData::BuildBase *buildBase = userInfo.add_buildinfo();
        BuildBase *build = iter->second;
        if(buildBase && build)
        {
            build->save(buildBase);
        }
    }
#endif
    for(auto iter = m_rubbishMap.begin();iter != m_rubbishMap.end();++iter)
    {
        const pb::Conf_t_rubbish *rubbishConf = tbx::rubbish().get_base(iter->second);
        if(rubbishConf == NULL)
            continue;
        ACTIVEITEMTYPE type = static_cast<ACTIVEITEMTYPE>(rubbishConf->rubbish->type());
        if(type == ACTIVEITEMTYPE_SWEET)
            continue;
        HelloKittyMsgData::RubbishData *temp = userInfo.add_rubbish();
        if(temp)
        {
            temp->set_roadid(iter->first);
            temp->set_rubbish(iter->second);
        }
    }
    return true;
}

bool BuildManager::fullMessage(HelloKittyMsgData::AckReconnectInfo& reconnect)
{
    for(auto iter = m_buildTypeMap.begin();iter != m_buildTypeMap.end();++iter)
    {
        const set<QWORD> &idSet = iter->second;
        for(auto it = idSet.begin();it != idSet.end();++it)
        {
            BuildBase *build = getBuild(*it);
            if(!build)
            {
                continue;
            }
            HelloKittyMsgData::BuildBase *buildBase = reconnect.add_buildinfo();
            if(buildBase)
            {
                build->save(buildBase);
            }
        }
    }
#if 0
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        HelloKittyMsgData::BuildBase *buildBase = userInfo.add_buildinfo();
        BuildBase *build = iter->second;
        if(buildBase && build)
        {
            build->save(buildBase);
        }
    }
#endif
    for(auto iter = m_rubbishMap.begin();iter != m_rubbishMap.end();++iter)
    {
        const pb::Conf_t_rubbish *rubbishConf = tbx::rubbish().get_base(iter->second);
        if(rubbishConf == NULL)
            continue;
        ACTIVEITEMTYPE type = static_cast<ACTIVEITEMTYPE>(rubbishConf->rubbish->type());
        if(type == ACTIVEITEMTYPE_SWEET)
            continue;
        HelloKittyMsgData::RubbishData *temp = reconnect.add_rubbish();
        if(temp)
        {
            temp->set_roadid(iter->first);
            temp->set_rubbish(iter->second);
        }
    }
    return true;
}


bool BuildManager::fullMessage(HelloKittyMsgData::EnterGardenInfo& kittyInfo)
{
    for(auto iter = m_buildTypeMap.begin();iter != m_buildTypeMap.end();++iter)
    {
        const set<QWORD> &idSet = iter->second;
        for(auto it = idSet.begin();it != idSet.end();++it)
        {
            BuildBase *build = getBuild(*it);
            if(!build)
            {
                continue;
            }
            HelloKittyMsgData::BuildBase *buildBase = kittyInfo.add_buildinfo();
            if(buildBase)
            {
                build->save(buildBase);
            }
        }
    }
#if 0
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        HelloKittyMsgData::BuildBase *buildBase = kittyInfo.add_buildinfo();
        BuildBase *build = iter->second;
        if(buildBase && build)
        {
            build->save(buildBase);
        }
    }
#endif
    for(auto iter = m_rubbishMap.begin();iter != m_rubbishMap.end();++iter)
    {
        const pb::Conf_t_rubbish *rubbishConf = tbx::rubbish().get_base(iter->second);
        if(rubbishConf == NULL)
            continue;
        ACTIVEITEMTYPE type = static_cast<ACTIVEITEMTYPE>(rubbishConf->rubbish->type());
        if(type == ACTIVEITEMTYPE_RUBBISH)
            continue;

        HelloKittyMsgData::RubbishData *temp = kittyInfo.add_rubbish();
        if(temp)
        {
            temp->set_roadid(iter->first);
            temp->set_rubbish(iter->second);
        }
    }
    return true;
}

bool BuildManager::flushOneBuild(const DWORD tempid)
{
    auto iter = m_buildMap.find(tempid);
    if(iter == m_buildMap.end())
    {
        Fir::logger->debug("[刷新建筑错误]:建筑找不到tempid %u",tempid);
        return false;
    }
    return iter->second->flush();
}

BuildBase* BuildManager::getBuild(const QWORD tempid)
{
    auto iter = m_buildMap.find(tempid);
    if(iter == m_buildMap.end())
    {
        Fir::logger->debug("[建筑查找实例错误]:建筑找不到tempid %lu",tempid);
        return NULL;
    }
    return iter->second;
}

bool BuildManager::loop()
{
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        BuildBase *build = iter->second;
        if(!build || build->isTypeBuild(Build_Type_Road))
        {
            continue;
        }
        build->loop();
    }
    return true;
}

void BuildManager::getAllPoint(std::vector<InitBuildPoint> &rAllPoint)
{
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        BuildBase *build = iter->second;
        if(!build)
        {
            continue;
        }
        InitBuildPoint buildPoint;
        buildPoint.buildType = build->getTypeID();
        buildPoint.point = build->getPoint();
        buildPoint.buildLevel = build->getLevel();
        rAllPoint.push_back(buildPoint);
    }

}

bool BuildManager::loadBuildInMap()
{
    if(m_owner == NULL)
        return true;
    //种点
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        BuildBase *build = iter->second;
        if(!build)
        {
            continue;
        }
        m_owner->m_kittyGarden.addBuildPoint(build,build->getPoint(),build->getRationMask());
    }

    //判断是否激活
    checkBuildActive();

    //登录结算金币产出
    auto iter = m_kindTypeMap.find(Build_Type_Gold_Produce);
    const std::set<QWORD> &goldSet = iter->second;
    for(auto iter = goldSet.begin();iter != goldSet.end();++iter)
    {
        BuildTypeProduceGold *goldBuild = dynamic_cast<BuildTypeProduceGold*>(getBuild(*iter));
        if(goldBuild)
        {
            goldBuild->loginCommit();
        }
    }

    return true;
}

bool BuildManager::checkBuildLevel(const DWORD typeID,const DWORD level,const DWORD num)
{
    return getBuildLevelNum(typeID,level) >= num;
}

DWORD BuildManager::getBuildLevelNum(const DWORD typeID,const DWORD level)
{
    DWORD size = 0;
    auto iter = m_buildTypeMap.find(typeID);
    if(iter == m_buildTypeMap.end())
    {
        return size;
    }

    const std::set<QWORD> &tempSet = iter->second;
    for(auto temp = tempSet.begin();temp != tempSet.end();++temp)
    {
        BuildBase *build = getBuild(*temp);
        if(build && !build->isTypeBuild(Build_Type_Road))
        {
            if(build->getLevel() >= level)
            {
                size += 1;
            }
        }
    }
    return size;
}   

DWORD BuildManager::getBuildLevel(const DWORD typeID)
{
    DWORD level = 0;
    auto iter = m_buildTypeMap.find(typeID);
    if(iter == m_buildTypeMap.end())
    {
        return level;
    }

    const std::set<QWORD> &tempSet = iter->second;
    for(auto temp = tempSet.begin();temp != tempSet.end();++temp)
    {
        BuildBase *build = getBuild(*temp);
        if(build && !build->isTypeBuild(Build_Type_Road))
        {
            if(build->getLevel() >= level)
            {
                level = build->getLevel();
            }
        }
    }
    return level;
}   

DWORD BuildManager::getBuildLevelNum(const DWORD level)
{
    DWORD num = 0;
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        const BuildBase *build = iter->second;
        if(build && build->getLevel() >= level && (build->isTypeBuild(Build_Type_Function) || build->isTypeBuild(Build_Type_Gold_Produce) || build->isTypeBuild(Build_Type_Item_Produce) || build->isTypeBuild(Build_Type_Item_Composite)))
        {
            num += 1;
        }
    }
    return num;

}

//获取所有者
SceneUser* BuildManager::getOwner()
{
    return  m_owner; 
}

bool BuildManager::checkBuildActive(const bool loginFlg)
{
    //判断是否激活
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        BuildBase *build = iter->second;
        if(!build || build->isTypeBuild(Build_Type_Road) || build->underClickStatus())
        {
            continue;
        }
        bool isActiveFlg = build->isActive();
        bool flg = m_owner->m_kittyGarden.checkActive(build,build->getPoint(),build->getRationMask());
        if(flg != isActiveFlg)
        {
            flg ? build->setMark(HelloKittyMsgData::Build_Status_Normal) : build->setMark(HelloKittyMsgData::Build_Status_Click_Active);
            build->processChangeStatus(loginFlg);
            build->flush();
        }
    }
    return true;
}

QWORD BuildManager::getAnyBuildBytype(const std::vector<DWORD> &vecId,bool bExp)
{
    std::set<DWORD> setId(vecId.begin(),vecId.end());
    std::vector<QWORD> rSel;
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();iter++)
    {
        BuildBase* pBase = iter->second;
        if(!pBase->isActive())
        {
            continue;
        }
        bool bFind = setId.find(pBase->getTypeBuild()) != setId.end();
        if(bFind && !bExp)
        {
            rSel.push_back(iter->first);

        }
        if(!bFind && bExp)
        {
            rSel.push_back(iter->first);
        }

    }

    if(rSel.empty())
        return 0;
    DWORD rate = zMisc::randBetween(0,rSel.size()-1);
    return rSel[rate];
}

QWORD BuildManager::getAnyBuildById(DWORD buildid)
{
    auto iter = m_buildTypeMap.find(buildid);
    if(iter == m_buildTypeMap.end())
        return 0;
    if(iter->second.empty())
        return 0;
    std::vector<QWORD> rSel;
    for(auto it = iter->second.begin();it != iter->second.end();it++)
    {
        BuildBase* pBase = getBuild(*it);
        if(!pBase || !pBase->isActive())
        {
            continue;
        }
        rSel.push_back(*it);

    }
    if(rSel.empty())
        return 0;
    DWORD rate = zMisc::randBetween(0,rSel.size()-1);
    return rSel[rate]; 
}

QWORD BuildManager::getAnyBuildById(const std::vector<DWORD> &vecId,bool bExp)
{
    std::set<DWORD> setId(vecId.begin(),vecId.end());
    std::vector<QWORD> rSel;       
    for(auto iter = m_buildTypeMap.begin(); iter != m_buildTypeMap.end(); iter++)
    {
        bool bFind = setId.find(iter->first) != setId.end(); 

        if(bFind && !bExp)
        {
            for(auto it = iter->second.begin(); it != iter->second.end(); it++)
            {
                BuildBase* pBase = getBuild(*it);
                if(!pBase || !pBase->isActive())
                {
                    continue;
                }
                rSel.push_back(*it);
            }

        }
        if(!bFind && bExp)
        { 
            for(auto it = iter->second.begin(); it != iter->second.end(); it++)
            {
                BuildBase* pBase = getBuild(*it);
                if(!pBase || !pBase->isActive())
                {
                    continue;
                }
                rSel.push_back(*it);
            }
        }


    }
    if(rSel.empty())
        return 0;
    DWORD rate = zMisc::randBetween(0,rSel.size()-1);
    return rSel[rate];

}

void   BuildManager::destroyBuild(QWORD BuildId)
{
    deleteBuild(BuildId);
}

DWORD  BuildManager::getBuildNum()
{
    DWORD wCount = 0;
    for(auto iter = m_buildMap.begin();iter != m_buildMap.end();++iter)
    {
        BuildBase *build = iter->second;
        if(!build || !build->isActive())
        {
            continue;
        }
        if(build->isTypeBuild(Build_Type_Function) || build->isTypeBuild(Build_Type_Gold_Produce) || build->isTypeBuild(Build_Type_Item_Produce) || build->isTypeBuild(Build_Type_Item_Composite))
        {
            wCount++;
        }

    }

    return wCount;
}

bool BuildManager::opKindType(const QWORD tempid,const bool opAdd)
{
    BuildBase *temp = getBuild(tempid);
    if(!temp)
    {
        return false;
    }
    DWORD kindType = temp->confBase->buildInfo->buildkind();
    auto iter = m_kindTypeMap.find(kindType);
    if(iter == m_kindTypeMap.end())
    {
        if(opAdd)
        {
            std::set<QWORD> tempSet;
            tempSet.insert(tempid);
            m_kindTypeMap.insert(std::pair<DWORD,std::set<QWORD>>(kindType,tempSet));
            return true;
        }
        return true;
    }
    std::set<QWORD> &tempSet = const_cast<std::set<QWORD>&>(iter->second);
    if(opAdd)
    {
        tempSet.insert(tempid);
        return true;
    }
    tempSet.erase(tempid);
    if(tempSet.empty())
    {
        m_kindTypeMap.erase(iter);
    }
    return true;
}

bool BuildManager::initTypeBuild(BuildBase *build)
{
    if(!build)
    {
        return false;
    }
    //生产道具建筑
    if(build->isTypeBuild(Build_Type_Item_Produce))
    {
        BuildTypeProduceItem* temp = dynamic_cast<BuildTypeProduceItem*>(build);
        if(temp)
        {
            temp->init();
            temp->sendInfoMeg();
        }
    }
    //合成道具建筑
    else if(build->isTypeBuild(Build_Type_Item_Composite))
    {
        BuildTypeCompositeItem* temp = dynamic_cast<BuildTypeCompositeItem*>(build);
        if(temp)
        {
            temp->init();
            temp->sendInfoMeg();
        }
    }
    //生产金币建筑
    else if(build->isTypeBuild(Build_Type_Gold_Produce))
    {
        BuildTypeProduceGold* temp = dynamic_cast<BuildTypeProduceGold*>(build);
        if(temp)
        {
            temp->updateProduce();
        }
    }
    return true;
}

void BuildManager::logPoint()
{
    Fir::logger->debug("BuildManager::logPoint begin(%lu,%s)",m_owner->charid,m_owner->charbase.nickname);
    for(auto iter = m_buildTypeMap.begin();iter != m_buildTypeMap.end();++iter)
    {
        const std::set<QWORD>& tempSet = iter->second;
        char temp[10240] = {0};
        for(auto it = tempSet.begin();it != tempSet.end();++it)
        {
            BuildBase* build = getBuild(*it);
            if(build)
            {
                snprintf(temp+strlen(temp),sizeof(temp),"%d_%d,",build->getPoint().x,build->getPoint().y);
            }
        }
        Fir::logger->debug("建筑类型:%u,建筑坐标:%s",iter->first,temp);
    }
    Fir::logger->debug("BuildManager::logPoint end(%lu,%s)",m_owner->charid,m_owner->charbase.nickname);
}

bool BuildManager::judgeHasFitRoom()
{
    auto iter = m_buildTypeMap.find(10010040);
    if(iter == m_buildTypeMap.end() || (iter->second).empty())
    {
        return false;
    }
    return true;
}

bool BuildManager::adjustBuildLevel(const CMD::GMTool::ModifyAttr &modify)
{
    auto it = m_buildTypeMap.find(modify.attrID);
    if(it == m_buildTypeMap.end())
    {
        return false;
    }
    QWORD buildID = 0;
    const std::set<QWORD>& tempSet = it->second;
    for(auto iter = tempSet.begin();iter != tempSet.end();++iter)
    {
        buildID = *iter;
        BuildBase* build = getBuild(*iter);
        if(!build)
        {
            continue;
        }
        bool delFlg = false;
        DWORD level = build->getLevel();
        if(modify.opType & ADD_OP)
        {
            for(DWORD num = 0;num < modify.val;++num)
            {
                //build->upGrade(true);
            }
        }
        if(modify.opType & SUB_OP)
        {
            if(level > modify.val)
            {
                build->subLevel(modify.val);
            }
            else
            {
                delFlg = true;
            }
        }
        if(modify.opType & SET_OP)
        {
            if(level < modify.val)
            {
                for(DWORD num = 0;num < modify.val - level;++num)
                {
                    //build->upGrade(true);
                }
            }
            else if(level > modify.val)
            {
                if(!modify.val)
                {
                    delFlg = true;
                }
                else
                {
                    build->subLevel(level - modify.val);
                }
            }
        }
        if(delFlg)
        {
            deleteBuild(build->getID());
        }
        Fir::logger->debug("[GM操作建筑] %lu,%u,%u,%u",buildID,modify.val,modify.attrID,modify.opType);
    }
    return true;
}

DWORD BuildManager::getRoadNum()
{
    DWORD ret = 0;
    if(m_buildTypeMap.find(10010041) != m_buildTypeMap.end())
    {
        ret += m_buildTypeMap[10010041].size();
    }
    if(m_buildTypeMap.find(10010065) != m_buildTypeMap.end())
    {
        ret += m_buildTypeMap[10010065].size();
    }
    if(m_buildTypeMap.find(10010066) != m_buildTypeMap.end())
    {
        ret += m_buildTypeMap[10010066].size();
    }
    if(m_buildTypeMap.find(10010067) != m_buildTypeMap.end())
    {
        ret += m_buildTypeMap[10010067].size();
    }
    return ret;
}
