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
#include "buildItemComposite.h"
#include "buildItemProduce.h"
#include "buildTypeProduceGold.h"
#include "RedisMgr.h"

//建筑仓库相关的功能

DWORD BuildManager::getEmptyCell()
{
    for(DWORD cellid = 0;cellid < m_maxCellID + 1 ;++cellid)
    {
        if(m_cellSet.find(cellid) == m_cellSet.end())
        {
            return cellid;
        }
    }
    DWORD cellid = ++m_maxCellID;
    return cellid;
}

void BuildManager::opBuildExp(const BuildBase *build,const bool opAdd)
{
#if 0
    DWORD typeID = build->getTypeID();
    for(DWORD level = 1;level < build->getLevel();++level)
    {
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"%s建筑%s(%lu,%u,%u)",opAdd ? "仓库拿出" : "收进仓库",opAdd ? "增加" : "减少",build->getID(),typeID,level);
        QWORD key = hashKey(typeID,level);
        const pb::Conf_t_building *confBase = tbx::building().get_base(key);
        if(confBase)
        {
            m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Exp,confBase->buildInfo->guestproduce(),temp,opAdd);
        }
    }
#endif
}

bool BuildManager::pickBuildInWare(const QWORD tempid)
{
    BuildBase *build = getBuild(tempid);
    //路不可以收起来(有事件的建筑)
    if(!build || build->isTypeBuild(Build_Type_Road) || build->getEvent() || build->underClickStatus())
    {
        return false;
    }

    QWORD key = hashKey(build->getTypeID(),build->getLevel());
    DWORD tempCellID = DWORD(-1);
    if(build->isTypeBuild(Build_Type_Decorate))
    {
        auto itr = m_keyIDMap.find(key);
        if(itr != m_keyIDMap.end())
        {
            std::set<DWORD>& tempSet = itr->second; 
            if(!build->m_inBuildID)
            {
                for(auto iter = tempSet.begin();iter != tempSet.end();++iter)
                {
                    if(findInBuildCell(build->getTypeID(),build->getLevel(),*iter))
                    {
                        continue;
                    }
                    tempCellID = *iter;
                    auto it = m_wareHouseBuildMap.find(tempCellID);
                    if(it == m_wareHouseBuildMap.end())
                    {
                        return false;
                    }
                    WareHouseBuildBase &temp = const_cast<WareHouseBuildBase&>(it->second); 
                    temp.num += 1;
                    break;
                }
            }
        }
    }
    //保存建筑中的产物信息
    if(tempCellID == DWORD(-1))
    {
        WareHouseBuildBase wareBuild(getEmptyCell());
        build->saveAsWareHouseBuildBase(wareBuild);
        wareSaveProduce(build,wareBuild.cellID);
        tempCellID = wareBuild.cellID;
        if(m_wareHouseBuildMap.find(tempCellID) != m_wareHouseBuildMap.end())
        {
            return false;
        }
        m_wareHouseBuildMap.insert(std::pair<DWORD,WareHouseBuildBase>(tempCellID,wareBuild));
        auto itr = m_keyIDMap.find(key);
        if(itr == m_keyIDMap.end())
        {
            std::set<DWORD> tempSet;
            tempSet.insert(tempCellID);
            m_keyIDMap.insert(std::pair<QWORD,std::set<DWORD> >(key,tempSet));
        }
        else
        {
            itr->second.insert(tempCellID);
        }
        if(build->m_inBuildID)
        {
            auto it = m_inBuildWareMap.find(key);
            if(it == m_inBuildWareMap.end())
            {
                std::set<DWORD> tempSet;
                tempSet.insert(tempCellID);
                m_inBuildWareMap.insert(std::pair<QWORD,std::set<DWORD> >(key,tempSet));
            }
            else
            {
                it->second.insert(tempCellID);
            }
            m_inBuildIDWareMap.insert(std::pair<QWORD,DWORD>(build->m_inBuildID,tempCellID));
        }
    }
    if(m_wareHouseTypeMap.find(build->getTypeID()) == m_wareHouseTypeMap.end())
    {
        m_wareHouseTypeMap.insert(std::pair<DWORD,DWORD>(build->getTypeID(),1));
    }
    else
    {
        m_wareHouseTypeMap[build->getTypeID()] += 1;
    }
    opBuildExp(build,false);
    m_cellSet.insert(tempCellID);
    deleteBuild(tempid);
    checkBuildActive();
    updateWareBuild(tempCellID);
    return true;
}

bool BuildManager::giveBuildInWare(const DWORD typeID,const DWORD level)
{
    QWORD key = hashKey(typeID,level);
    const pb::Conf_t_building *tempConf = tbx::building().get_base(key);
    if(!tempConf || tempConf->buildInfo->buildkind() == Build_Type_Road)
    {
        return false;
    }
    DWORD tempCellID = 0;
    if(tempConf->buildInfo->buildkind() == Build_Type_Decorate)
    {
        auto itr = m_keyIDMap.find(key);
        if(itr != m_keyIDMap.end())
        {
            std::set<DWORD>& tempSet = itr->second; 
            for(auto iter = tempSet.begin();iter != tempSet.end();++iter)
            {
                if(findInBuildCell(typeID,level,*iter))
                {
                    continue;
                }
                tempCellID = *iter;
                auto it = m_wareHouseBuildMap.find(tempCellID);
                if(it == m_wareHouseBuildMap.end())
                {
                    return false;
                }
                WareHouseBuildBase &temp = const_cast<WareHouseBuildBase&>(it->second); 
                temp.num += 1;
                break;
            }
        }
    }
    //保存建筑中的产物信息
    if(!tempCellID)
    {
        WareHouseBuildBase wareBuild(getEmptyCell());
        wareBuild.typeID = typeID;
        wareBuild.level = level;
        wareBuild.num = 1;
        tempCellID = wareBuild.cellID;
        if(m_wareHouseBuildMap.find(tempCellID) != m_wareHouseBuildMap.end())
        {
            return false;
        }
        m_wareHouseBuildMap.insert(std::pair<DWORD,WareHouseBuildBase>(tempCellID,wareBuild));
        auto itr = m_keyIDMap.find(key);
        if(itr != m_keyIDMap.end())
        {
            std::set<DWORD>& tempSet = itr->second; 
            tempSet.insert(tempCellID);
            m_keyIDMap.insert(std::pair<QWORD,std::set<DWORD> >(key,tempSet));
        }
        else
        {
            itr->second.insert(tempCellID);
        }
    }
    if(m_wareHouseTypeMap.find(typeID) == m_wareHouseTypeMap.end())
    {
        m_wareHouseTypeMap.insert(std::pair<DWORD,DWORD>(typeID,1));
    }
    else
    {
        m_wareHouseTypeMap[typeID] += 1;
    }
    m_cellSet.insert(tempCellID);
    updateWareBuild(tempCellID);
    return true;
}

bool BuildManager::pushUinityBuild(const QWORD inBuildID,const DWORD inBuildLevel,const QWORD friendID,const DWORD typeID,const DWORD level)
{


    QWORD key = hashKey(typeID,level);
    const pb::Conf_t_building *tempConf = tbx::building().get_base(key);
    if(!tempConf)
    {
        return false;
    }

    WareHouseBuildBase wareBuild(getEmptyCell());
    wareBuild.typeID = typeID;
    wareBuild.level = level;
    wareBuild.num = 1;
    wareBuild.inBuildID = inBuildID;
    wareBuild.inBuildLevel = inBuildLevel;
    wareBuild.friendID = friendID;
    m_wareHouseBuildMap.insert(std::pair<QWORD,WareHouseBuildBase>(wareBuild.cellID,wareBuild));
    auto itr = m_keyIDMap.find(key);
    if(itr == m_keyIDMap.end())
    {
        std::set<DWORD> tempSet;
        tempSet.insert(wareBuild.cellID);
        m_keyIDMap.insert(std::pair<QWORD,std::set<DWORD> >(key,tempSet));
    }
    else
    {
        itr->second.insert(wareBuild.cellID);
    }

    auto it = m_inBuildWareMap.find(key);
    if(it == m_inBuildWareMap.end())
    {
        std::set<DWORD> tempSet;
        tempSet.insert(wareBuild.cellID);
        m_inBuildWareMap.insert(std::pair<QWORD,std::set<DWORD> >(key,tempSet));
    }
    else
    {
        it->second.insert(wareBuild.cellID);
    }
    m_inBuildIDWareMap.insert(std::pair<QWORD,DWORD>(inBuildID,wareBuild.cellID));
    m_cellSet.insert(wareBuild.cellID);
    updateWareBuild(wareBuild.cellID);
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_UnityBuild);

    const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(typeID,inBuildLevel));
    if(pbuildConf && redishandle)
    {
        QWORD uScore = pbuildConf->UniteBuildlevel->costpopular() + pbuildConf->UniteBuildlevel->costpopularmax();
        redishandle->addRankScore("unitbuildrank",m_owner->charid,"totalscore",uScore);
        HelloKittyMsgData::MaxUnityBuild rscorebuild;
        rscorebuild.set_maxscore(0);
        rscorebuild.set_buildid(0);
        rscorebuild.set_buildlv(0);
        rscorebuild.set_otherid(0);
        rscorebuild.set_totalpopular(0);
        rscorebuild.set_totalmaxpopular(0);
        RedisMgr::getMe().get_unitybuildrankdata(m_owner->charid,rscorebuild);
        rscorebuild.set_totalpopular(pbuildConf->UniteBuildlevel->costpopular() + rscorebuild.totalpopular());
        rscorebuild.set_totalmaxpopular(pbuildConf->UniteBuildlevel->costpopularmax() + rscorebuild.totalmaxpopular());
        if(rscorebuild.maxscore() < uScore)
        {
            rscorebuild.set_maxscore(uScore);
            rscorebuild.set_buildid(typeID);
            rscorebuild.set_buildlv(inBuildLevel);
            rscorebuild.set_otherid(friendID);
        }
        RedisMgr::getMe().set_unitybuildrankdata(m_owner->charid,rscorebuild);
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"合建建筑(%u,%u)",typeID,inBuildLevel);
        DWORD val = pbuildConf->UniteBuildlevel->costpopular();
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Now,val,temp,true);
        val = pbuildConf->UniteBuildlevel->costpopularmax();
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Max,val,temp,true);

    }

    return true;
}

bool BuildManager::pickoutBuild(const HelloKittyMsgData::ReqPickOutBuid *message)
{
    auto iter = m_wareHouseBuildMap.find(message->cellid());
    if(iter == m_wareHouseBuildMap.end())
    {
        return false;
    }

    //先干掉仓库中的
    WareHouseBuildBase &temp = const_cast<WareHouseBuildBase&>(iter->second); 
    temp.num -= 1;
    QWORD key = hashKey(temp.typeID,temp.level);
    updateWareBuild(temp.cellID);
    if(!build(temp,message->moveflg(),message->point()))
    {
        temp.num += 1;
        updateWareBuild(temp.cellID);
        return false;
    }
    if(m_wareHouseTypeMap.find(temp.typeID) != m_wareHouseTypeMap.end())
    {
        if(m_wareHouseTypeMap[temp.typeID] >= 1)
        {
            m_wareHouseTypeMap[temp.typeID] -= 1;
        }
        else
        {
            m_wareHouseTypeMap.erase(temp.typeID);
        }
    }
    auto itr = m_keyIDMap.find(key);
    if(itr != m_keyIDMap.end())
    {
        itr->second.erase(temp.cellID);
    }
    if(temp.inBuildID)
    {
        auto it = m_inBuildWareMap.find(key);
        if(it != m_inBuildWareMap.end())
        {
            it->second.erase(temp.cellID);
        }
        m_inBuildIDWareMap.erase(temp.inBuildID);
    }
    checkBuildActive();
    if(!temp.num)
    {
        m_wareHouseBuildMap.erase(iter);
        m_cellSet.erase(message->cellid());
    }
    return true;
}

bool BuildManager::reqSellWareHouseBuild(const HelloKittyMsgData::ReqSellWareHouseBuild *message)
{
    auto iter = m_wareHouseBuildMap.find(message->cellid());
    if(iter == m_wareHouseBuildMap.end())
    {
        return false;
    } 
    WareHouseBuildBase &temp = const_cast<WareHouseBuildBase&>(iter->second);
    //正在合建建筑，不可以删除
    if(temp.inBuildID)
    {
        HelloKittyMsgData::UnitRunInfo binary;
        if(RedisMgr::getMe().get_unitybuildata(temp.inBuildID,binary))
        {
            m_owner->opErrorReturn(HelloKittyMsgData::SellUnityBuilding);
            return false;

        }
    }
    //先干掉仓库中的
    temp.num -= 1;
    QWORD key = hashKey(temp.typeID,temp.level);
    updateWareBuild(temp.cellID);
    if(m_wareHouseTypeMap.find(temp.typeID) != m_wareHouseTypeMap.end())
    {
        if(m_wareHouseTypeMap[temp.typeID] >= 1)
        {
            m_wareHouseTypeMap[temp.typeID] -= 1;
        }
        else
        {
            m_wareHouseTypeMap.erase(temp.typeID);
        }
    }
    auto itr = m_keyIDMap.find(key);
    if(itr != m_keyIDMap.end())
    {
        itr->second.erase(temp.cellID);
    }
    if(temp.inBuildID)
    {
        auto it = m_inBuildWareMap.find(key);
        if(it != m_inBuildWareMap.end())
        {
            it->second.erase(temp.cellID);
        }
        m_inBuildIDWareMap.erase(temp.inBuildID);
    }
    //给钱

    HelloKittyMsgData::vecAward awarditem;
    HelloKittyMsgData::Award* pitem = awarditem.add_award();
    if(pitem)
    {
        pitem->set_awardtype(HelloKittyMsgData::Attr_Gold);
        pitem->set_awardval(1);
        if(temp.inBuildID > 0)
        {
            const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(temp.typeID,temp.inBuildLevel));
            if(pbuildConf)
                pitem->set_awardval(pbuildConf->UniteBuildlevel->sellprice());
        }
        else
        {
            auto itshop = pb::Conf_t_Shop::buildMap.find(temp.typeID);
            if(itshop != pb::Conf_t_Shop::buildMap.end())
            {
                const pb::Conf_t_Shop *shopConf = tbx::Shop().get_base(itshop->second);
                if(shopConf != NULL)
                {
                    auto itershop = shopConf->getPriceMap().find(HelloKittyMsgData::Attr_Gold);
                    if(itershop != shopConf->getPriceMap().end())
                    {
                        pitem->set_awardval(ceil(0.5*itershop->second));
                    }
                }
            }

        }
        m_owner->pushItem(awarditem,"sell build");
    }



    if(!temp.num)
    {
        m_wareHouseBuildMap.erase(iter);
        m_cellSet.erase(message->cellid());
    }
    if(temp.inBuildID)
    {
        //合建排行榜维护
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_UnityBuild);
        const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(temp.typeID,temp.inBuildLevel));
        if(pbuildConf && redishandle)
        {
            SQWORD uScore = pbuildConf->UniteBuildlevel->costpopular() + pbuildConf->UniteBuildlevel->costpopularmax();
            if(redishandle->getRankScore("unitbuildrank","totalscore",m_owner->charid) <= QWORD(uScore))
            {
                redishandle->delRank("unitbuildrank","totalscore",m_owner->charid);
                RedisMgr::getMe().del_unitybuildrankdata(m_owner->charid);

            }
            else
            {
                redishandle->addRankScore("unitbuildrank",m_owner->charid,"totalscore",uScore*(-1));
                HelloKittyMsgData::MaxUnityBuild rscorebuild;
                rscorebuild.set_maxscore(0);
                rscorebuild.set_buildid(0);
                rscorebuild.set_buildlv(0);
                rscorebuild.set_otherid(0);
                rscorebuild.set_totalpopular(0);
                rscorebuild.set_totalmaxpopular(0);
                RedisMgr::getMe().get_unitybuildrankdata(m_owner->charid,rscorebuild);
                rscorebuild.set_totalpopular(rscorebuild.totalpopular() - pbuildConf->UniteBuildlevel->costpopular());
                rscorebuild.set_totalmaxpopular(rscorebuild.totalmaxpopular() - pbuildConf->UniteBuildlevel->costpopularmax());
                if(rscorebuild.buildid() == temp.inBuildID)
                {
                    DWORD InbuildLv = 0;
                    QWORD InbuilID  = 0;
                    QWORD friendID  = 0;
                    DWORD maxscore = getMaxBuildScore(InbuilID,InbuildLv,friendID);
                    if(maxscore > 0)
                    {
                        rscorebuild.set_maxscore(maxscore);
                        rscorebuild.set_buildid(InbuilID);
                        rscorebuild.set_buildlv(InbuildLv);
                        rscorebuild.set_otherid(friendID);
                    }

                }
                RedisMgr::getMe().set_unitybuildrankdata(m_owner->charid,rscorebuild);
            }

            char reMark[100] = {0};
            snprintf(reMark,sizeof(reMark),"出售合建建筑(%u,%u)",temp.typeID,temp.inBuildLevel);
            DWORD val = pbuildConf->UniteBuildlevel->costpopular();
            m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Now,val,reMark,false);
            val = pbuildConf->UniteBuildlevel->costpopularmax();
            m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Max,val,reMark,false);
        }
    }
    return true;
}

bool BuildManager::flushAllWareHouseBuild()
{
    HelloKittyMsgData::AckFlushWareBuild message;
    for(auto iter = m_wareHouseBuildMap.begin();iter != m_wareHouseBuildMap.end();++iter)
    {
        const WareHouseBuildBase &wareBuildInfo = iter->second;
        HelloKittyMsgData::WareHouseBuildBaseForCli *wareBuildForCli = message.add_warebuild();
        HelloKittyMsgData::WareHouseBuildBase *wareBuild = wareBuildForCli->mutable_baseinfo();
        HelloKittyMsgData::playerShowbase* pother = wareBuildForCli->mutable_othershow();
        if(wareBuild == NULL || pother == NULL)
        {
            continue;
        }
        wareBuild->set_type(wareBuildInfo.typeID);
        wareBuild->set_level(wareBuildInfo.level);
        wareBuild->set_num(wareBuildInfo.num);
        wareBuild->set_cellid(wareBuildInfo.cellID);
        wareBuild->set_inbuildid(wareBuildInfo.inBuildID);
        wareBuild->set_inbuildlevel(wareBuildInfo.inBuildLevel);
        wareBuild->set_friendid(wareBuildInfo.friendID);
        SceneUser::getplayershowbase(wareBuildInfo.friendID,*pother);
    }

    std::string ret;
    if(encodeMessage(&message,ret))
    {
        m_owner->sendCmdToMe(ret.c_str(),ret.size());
    }
    return true;
}

bool BuildManager::updateWareBuild(const DWORD cellID)
{
    auto iter = m_wareHouseBuildMap.find(cellID);
    if(iter == m_wareHouseBuildMap.end())
    {
        return false;
    }

    const WareHouseBuildBase &wareBuildInfo = iter->second; 
    HelloKittyMsgData::AckUpdateWareBuid update;
    HelloKittyMsgData::WareHouseBuildBaseForCli *wareBuildForCli = update.mutable_warebuild();
    HelloKittyMsgData::WareHouseBuildBase *temp = wareBuildForCli->mutable_baseinfo();
    HelloKittyMsgData::playerShowbase* pother = wareBuildForCli->mutable_othershow();
    if(temp == NULL || pother == NULL)
    {
        return false;
    }
    temp->set_type(wareBuildInfo.typeID);
    temp->set_level(wareBuildInfo.level);
    temp->set_num(wareBuildInfo.num);
    temp->set_cellid(wareBuildInfo.cellID);
    temp->set_inbuildid(wareBuildInfo.inBuildID);
    temp->set_inbuildlevel(wareBuildInfo.inBuildLevel);
    temp->set_friendid(wareBuildInfo.friendID);
    SceneUser::getplayershowbase(wareBuildInfo.friendID,*pother);

    std::string ret;
    if(encodeMessage(&update,ret))
    {
        m_owner->sendCmdToMe(ret.c_str(),ret.size());
    }
    return true;
}

bool BuildManager::wareSaveProduce(BuildBase *build,const DWORD cellID)
{
    //保存建筑中的产物信息
    if(build->isTypeBuild(Build_Type_Item_Produce))
    {
        HelloKittyMsgData::WareHouseProduceInfo produceItem;
        produceItem.set_cellid(cellID);
        HelloKittyMsgData::ProduceInfo *tempCell = produceItem.mutable_produceinfo();
        if(tempCell)
        {
            BuildTypeProduceItem *func = dynamic_cast<BuildTypeProduceItem*>(build);
            if(func)
            {
                func->saveProduce(tempCell);
            }
            if(m_wareProduceItemMap.find(produceItem.cellid()) == m_wareProduceItemMap.end())
            {
                m_wareProduceItemMap.insert(std::pair<DWORD,HelloKittyMsgData::WareHouseProduceInfo>(produceItem.cellid(),produceItem));
                return true;
            }
        }
    }
    if(build->isTypeBuild(Build_Type_Item_Composite))
    {
        HelloKittyMsgData::WareHouseCompositeInfo produceItem;
        produceItem.set_cellid(cellID);
        HelloKittyMsgData::CompositeInfo *tempCell = produceItem.mutable_compositeinfo();
        if(tempCell)
        {
            BuildTypeCompositeItem *func = dynamic_cast<BuildTypeCompositeItem*>(build);
            if(func)
            {
                func->saveProduce(tempCell);
            }
            if(m_wareCompositeItemMap.find(produceItem.cellid()) == m_wareCompositeItemMap.end())
            {
                m_wareCompositeItemMap.insert(std::pair<DWORD,HelloKittyMsgData::WareHouseCompositeInfo>(produceItem.cellid(),produceItem));
                return true;
            }
        }
    }
    else if(build->isTypeBuild(Build_Type_Gold_Produce))
    {
        BuildTypeProduceGold *buildType = dynamic_cast<BuildTypeProduceGold*>(build);
        HelloKittyMsgData::WareHouseOtherInfo produceOther;
        produceOther.set_cellid(cellID);
        if(buildType)
        {
            buildType->saveProduce(produceOther);
            if(m_wareProduceOther.find(produceOther.cellid()) == m_wareProduceOther.end())
            {
                m_wareProduceOther.insert(std::pair<DWORD,HelloKittyMsgData::WareHouseOtherInfo>(produceOther.cellid(),produceOther));
                return true;
            }
        }
    }
    return false;
}

bool BuildManager::subBuild(const DWORD typeID,const DWORD level,const DWORD num)
{
    DWORD delNum = num;
    bool changeFlg = false;
    std::vector<DWORD> tempVec;
    //删仓库中的建筑
    for(auto iter = m_wareHouseBuildMap.begin();iter != m_wareHouseBuildMap.end() && delNum;++iter)
    {
        WareHouseBuildBase &temp = const_cast<WareHouseBuildBase&>(iter->second);
        if(temp.typeID == typeID && temp.level == level)
        {
            if(m_wareHouseTypeMap.find(temp.typeID) != m_wareHouseTypeMap.end())
            {
                if(m_wareHouseTypeMap[temp.typeID] >= delNum)
                {
                    m_wareHouseTypeMap[temp.typeID] -= delNum;
                    delNum = 0;
                }
                else
                {
                    delNum -= m_wareHouseTypeMap[temp.typeID];
                    tempVec.push_back(temp.cellID);
                }
                temp.num = m_wareHouseTypeMap[temp.typeID];
                updateWareBuild(temp.cellID);
            }
        }
    }
    for(auto it = tempVec.begin();it != tempVec.end();++it)
    {
        auto itr = m_wareHouseBuildMap.find(*it);
        if(itr != m_wareHouseBuildMap.end())
        {
            WareHouseBuildBase &temp = const_cast<WareHouseBuildBase&>(itr->second);
            m_wareHouseTypeMap.erase(temp.typeID);
            m_cellSet.erase(*it);
            QWORD key = hashKey(temp.typeID,temp.level);
            m_keyIDMap.erase(key);
            m_wareHouseBuildMap.erase(itr);
        }
    }
    //在已建好的建筑
    if(delNum)
    {

        std::vector<QWORD> delVec;
        auto iter = m_buildTypeMap.find(typeID);
        std::set<QWORD> &tempSet = const_cast<std::set<QWORD>&>(iter->second);
        for(auto it = tempSet.begin();it != tempSet.end() && delNum;++it)
        {
            BuildBase *build = getBuild(*it);
            if(build && build->getLevel() == level)
            {
                delVec.push_back(*it);
                --delNum;
            }
        }
        for(auto it = delVec.begin();it != delVec.end();++it)
        {
            deleteBuild(*it);
            changeFlg = true;
        }
    }
    if(changeFlg)
    {
        checkBuildActive();
    }
    return !delNum; 
}

bool BuildManager::findInBuildCell(const DWORD typeID,const DWORD level,const DWORD cellID)
{
    QWORD key = hashKey(typeID,level);
    auto iter = m_inBuildWareMap.find(key);
    if(iter == m_inBuildWareMap.end())
    {
        return 0;
    }
    std::set<DWORD> &tempSet = iter->second;
    return tempSet.find(cellID) != tempSet.end(); 
}

DWORD BuildManager::getUinityBuildLevel(const QWORD UnityOnlyID)
{
    if(m_inBuildIDWareMap.find(UnityOnlyID) != m_inBuildIDWareMap.end())
    {
        DWORD cellID = m_inBuildIDWareMap[UnityOnlyID];
        auto iter = m_wareHouseBuildMap.find(cellID);
        if(iter == m_wareHouseBuildMap.end())
        {
            return 0;
        }
        return iter->second.inBuildLevel;
    }
    if(m_inBuildIDMap.find(UnityOnlyID) != m_inBuildIDMap.end())
    {
        QWORD buildID = m_inBuildIDMap[UnityOnlyID];
        BuildBase *build = getBuild(buildID);
        if(build)
        {
            return build->m_inBuildLevel;
        }
    }
    return 0;
}

DWORD BuildManager::getDecorationsBuildingNum()
{
    DWORD num = 0;
    for(auto it = m_wareHouseBuildMap.begin() ;it != m_wareHouseBuildMap.end();it++)
    {
        WareHouseBuildBase &rHouse = it->second;

        QWORD key = hashKey(rHouse.typeID,rHouse.level);
        const pb::Conf_t_building *tempConf = tbx::building().get_base(key);
        if(tempConf == NULL)
        {
            continue;
        }
        if(tempConf->buildInfo->buildkind() == 6)
        {
            num += rHouse.num;

        }

    }
    for(auto it= m_buildMap.begin();it != m_buildMap.end();it++)
    {
        BuildBase *build = it->second;
        if(build == NULL)
        {
            continue;
        }
        QWORD key = hashKey(build->getTypeID(),build->getLevel());
        const pb::Conf_t_building *tempConf = tbx::building().get_base(key);
        if(tempConf == NULL)
        {
            continue;
        }
        if(tempConf->buildInfo->buildkind() == 6)
        {
            num++; 

        }
    }
    return num ;


}

DWORD BuildManager::getPopularBuildingNum()
{
    DWORD num = 0;
    for(auto it = m_wareHouseBuildMap.begin() ;it != m_wareHouseBuildMap.end();it++)
    {
        WareHouseBuildBase &rHouse = it->second;

        QWORD key = hashKey(rHouse.typeID,rHouse.level);
        const pb::Conf_t_building *tempConf = tbx::building().get_base(key);
        if(tempConf == NULL)
        {
            continue;
        }
        if(tempConf->buildInfo->costpopular() > 0)
        {
            num += rHouse.num;
            continue;

        }
        if(rHouse.inBuildID == 0)
        {
            continue;
        }
        const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(rHouse.typeID,rHouse.inBuildLevel));
        if(pbuildConf ==NULL)
            continue;
        if(pbuildConf->UniteBuildlevel->costpopular() > 0)
        {
            num += rHouse.num;
        }

    }
    for(auto it= m_buildMap.begin();it != m_buildMap.end();it++)
    {
        BuildBase *build = it->second;
        if(build == NULL)
        {
            continue;
        }
        QWORD key = hashKey(build->getTypeID(),build->getLevel());
        const pb::Conf_t_building *tempConf = tbx::building().get_base(key);
        if(tempConf == NULL)
        {
            continue;
        }
        if(tempConf->buildInfo->costpopular() > 0)
        {
            num++;
            continue;

        }
        if(build->m_inBuildID == 0)
        {
            continue;
        }
        const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(build->getTypeID(),build->m_inBuildLevel));
        if(pbuildConf ==NULL)
            continue;
        if(pbuildConf->UniteBuildlevel->costpopular() > 0)
            num++;
    }
    return num ;

}

DWORD BuildManager::getUinityBuildnum()
{
    DWORD num = 0;
    for(auto it = m_wareHouseBuildMap.begin() ;it != m_wareHouseBuildMap.end();it++)
    {
        WareHouseBuildBase &rHouse = it->second;

        if(rHouse.inBuildID > 0)
        {
            num += rHouse.num;
        }

    }
    for(auto it= m_buildMap.begin();it != m_buildMap.end();it++)
    {
        BuildBase *build = it->second;
        if(build == NULL)
        {
            continue;
        }
        if(build->m_inBuildID > 0)
        {
            num++;
        }
    }
    return num ;

}

DWORD BuildManager::getUinityBuildLevelTotal()
{
    DWORD num = 0;
    for(auto it = m_wareHouseBuildMap.begin() ;it != m_wareHouseBuildMap.end();it++)
    {
        WareHouseBuildBase &rHouse = it->second;

        if(rHouse.inBuildID > 0)
        {
            num += rHouse.num*rHouse.inBuildLevel;
        }

    }
    for(auto it= m_buildMap.begin();it != m_buildMap.end();it++)
    {
        BuildBase *build = it->second;
        if(build == NULL)
        {
            continue;
        }
        if(build->m_inBuildID > 0)
        {
            num += build->m_inBuildLevel;
        }
    }
    return num ;

}

DWORD BuildManager::getUinityBuildnumByLevel(DWORD InBuildLv)
{
    DWORD num = 0;
    for(auto it = m_wareHouseBuildMap.begin() ;it != m_wareHouseBuildMap.end();it++)
    {
        WareHouseBuildBase &rHouse = it->second;

        if(rHouse.inBuildID > 0 && rHouse.inBuildLevel >= InBuildLv)
        {
            num += rHouse.num;
        }

    }
    for(auto it= m_buildMap.begin();it != m_buildMap.end();it++)
    {
        BuildBase *build = it->second;
        if(build == NULL)
        {
            continue;
        }
        if(build->m_inBuildID > 0 && build->m_inBuildLevel >= InBuildLv)
        {
            num ++;
        }
    }
    return num ;

}

DWORD BuildManager::getMaxBuildScore(QWORD &InBuildID,DWORD &InBuildLv,QWORD &friendID)
{
    DWORD score = 0;
    for(auto it = m_wareHouseBuildMap.begin() ;it != m_wareHouseBuildMap.end();it++)
    {
        WareHouseBuildBase &rHouse = it->second;
        if(rHouse.inBuildID == 0)
        {
            continue;
        }
        const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(rHouse.typeID,rHouse.inBuildLevel));
        if(pbuildConf ==NULL)
            continue;
        QWORD uScore = pbuildConf->UniteBuildlevel->costpopular() + pbuildConf->UniteBuildlevel->costpopularmax();
        if(uScore > score)
        {
            score = uScore;
            InBuildID = rHouse.inBuildID;
            InBuildLv = rHouse.inBuildLevel;
            friendID = rHouse.friendID;
        }

    }
    for(auto it= m_buildMap.begin();it != m_buildMap.end();it++)
    {
        BuildBase *build = it->second;
        if(build == NULL)
        {
            continue;
        }
        if(build->m_inBuildID == 0)
        {
            continue;
        }


        const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(build->getTypeID(),build->m_inBuildLevel));
        if(pbuildConf ==NULL)
            continue;
        QWORD uScore = pbuildConf->UniteBuildlevel->costpopular() + pbuildConf->UniteBuildlevel->costpopularmax();
        if(uScore > score)
        {
            score = uScore;
            InBuildID = build->m_inBuildID;
            InBuildLv = build->m_inBuildLevel;
            friendID = build->m_friendID;
        }
    }
    return score ;

}

bool BuildManager::setUinityBuildLevel(const QWORD UnityOnlyID,const DWORD UnityLevel)
{
    DWORD typeID = 0;
    QWORD friendID = 0;
    DWORD OldLevel = 0;

    bool ret = true;
    do{
        if(m_inBuildIDWareMap.find(UnityOnlyID) != m_inBuildIDWareMap.end())
        {
            DWORD cellID = m_inBuildIDWareMap[UnityOnlyID];
            auto iter = m_wareHouseBuildMap.find(cellID);
            if(iter == m_wareHouseBuildMap.end())
            {
                ret = false;
                break;
            }
            OldLevel = iter->second.inBuildLevel;
            iter->second.inBuildLevel = UnityLevel;
            updateWareBuild(cellID);
            typeID = iter->second.typeID;
            friendID = iter->second.friendID;
            break;
        }
        if(m_inBuildIDMap.find(UnityOnlyID) != m_inBuildIDMap.end())
        {
            QWORD buildID = m_inBuildIDMap[UnityOnlyID];
            BuildBase *build = getBuild(buildID);
            if(build == NULL)
            {
                ret = false;  
                break;
            }
            OldLevel = build->m_inBuildLevel;
            build->m_inBuildLevel = UnityLevel;
            build->flush();
            typeID = build->getTypeID();
            friendID = build->m_friendID;
            break;
        }
    }while(0);
    if(!ret)
    {
        return ret;
    }
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_UnityBuild);
    const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(typeID,UnityLevel));
    const pb::Conf_t_UniteBuildlevel *plowbuildConf = tbx::UniteBuildlevel().get_base(hashKey(typeID,OldLevel));
    if(pbuildConf 
            && redishandle 
            && plowbuildConf 
            && pbuildConf->UniteBuildlevel->costpopular() >= plowbuildConf->UniteBuildlevel->costpopular()
            && pbuildConf->UniteBuildlevel->costpopularmax() >= plowbuildConf->UniteBuildlevel->costpopularmax())
    {
        QWORD udiffpopular = pbuildConf->UniteBuildlevel->costpopular() - plowbuildConf->UniteBuildlevel->costpopular();
        QWORD udiffpopularmax =  pbuildConf->UniteBuildlevel->costpopularmax() - plowbuildConf->UniteBuildlevel->costpopularmax();
        QWORD uScore = pbuildConf->UniteBuildlevel->costpopular() + pbuildConf->UniteBuildlevel->costpopularmax();
        redishandle->addRankScore("unitbuildrank",m_owner->charid,"totalscore",udiffpopular + udiffpopularmax);
        HelloKittyMsgData::MaxUnityBuild rscorebuild;
        rscorebuild.set_maxscore(0);
        rscorebuild.set_buildid(0);
        rscorebuild.set_buildlv(0);
        rscorebuild.set_otherid(0);
        rscorebuild.set_totalpopular(0);
        rscorebuild.set_totalmaxpopular(0);
        RedisMgr::getMe().get_unitybuildrankdata(m_owner->charid,rscorebuild);
        rscorebuild.set_totalpopular(udiffpopular + rscorebuild.totalpopular());
        rscorebuild.set_totalmaxpopular(udiffpopularmax + rscorebuild.totalmaxpopular());
        if(rscorebuild.maxscore() < uScore)
        {
            rscorebuild.set_maxscore(uScore);
            rscorebuild.set_buildid(typeID);
            rscorebuild.set_buildlv(UnityLevel);
            rscorebuild.set_otherid(friendID);
        }
        RedisMgr::getMe().set_unitybuildrankdata(m_owner->charid,rscorebuild);

        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"合建建筑(%u,%u)",typeID,UnityLevel);
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Now,udiffpopular,temp,true);
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Max,udiffpopularmax,temp,true);
    }

    return ret;
}

void BuildManager::getUinityBuildOnlySet(const DWORD BuildID,std::set<QWORD> &setOnlyId)
{
    for(auto iter = m_wareHouseBuildMap.begin();iter != m_wareHouseBuildMap.end();++iter)
    {
        const WareHouseBuildBase &wareBuild = iter->second;
        if(wareBuild.typeID == BuildID && wareBuild.inBuildID)
        {
            setOnlyId.insert(wareBuild.inBuildID);
        }
    }
    auto iter = m_buildTypeMap.find(BuildID);
    if(iter != m_buildTypeMap.end())
    {
        const std::set<QWORD> &tempSet = iter->second;
        for(auto iter = tempSet.begin();iter != tempSet.end();++iter)
        {
            BuildBase *build = getBuild(*iter);
            if(build && build->getTypeID() == BuildID && build->m_inBuildID)
            {
                setOnlyId.insert(build->m_inBuildID);
            }
        }
    }
}

void BuildManager::getUinityBuildOnlySetByPlayerID(const QWORD PlayerID,const DWORD BuildID,std::set<QWORD> &setOnlyId)
{
    do
    {
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(PlayerID);
        if(!handle)
        {
            break;
        }
        DWORD SenceId = handle->getInt("playerscene",PlayerID,"sceneid");
        if(SenceId)
        {
            SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerID);
            if(user)
            {
                user->m_buildManager.getUinityBuildOnlySet(BuildID,setOnlyId);
                break;
            }
            HelloKittyMsgData::Serialize binary;
            if(!RedisMgr::getMe().get_binary(PlayerID,binary))
            {
                break;
            }
            BuildManager tempManager(NULL);
            tempManager.load(binary);
            tempManager.getUinityBuildOnlySet(BuildID,setOnlyId);
            break;
        }
        else
        {
            SceneUser* user  =  SceneUserManager::getMe().CreateTempUser(PlayerID);
            if(user)
            {
                user->m_buildManager.getUinityBuildOnlySet(BuildID,setOnlyId);
                break;
            }
        }
        break;
    }while(false);

}
