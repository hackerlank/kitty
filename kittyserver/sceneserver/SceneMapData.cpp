#include "SceneMapData.h"
#include "SceneUser.h"
#include "buildBase.h"
#include "SceneMapDataManager.h"
#include "dataManager.h"
#include "TimeTick.h"

SceneMapData::SceneMapData(SceneUser *owner,const DWORD id,const char *fileName,const char *path) : MapData(id,fileName,path),m_owner(owner)
{
    m_pointBuildMap.clear();
    m_buildPointSet.clear();
    m_ownerID = m_owner ? m_owner->charid : 0;
}

bool SceneMapData::flushMap()
{
    HelloKittyMsgData::AckKittyGarden ackKittyGarden;
    ackKittyGarden.set_charid(m_ownerID);
    HelloKittyMsgData::KittyGarden *kittyGarden = ackKittyGarden.mutable_kittygarden();
    if(!kittyGarden)
    {
        return false;
    }

    for(auto iter = m_pointAreaMap.begin();iter != m_pointAreaMap.end();++iter) 
    {
        AreaGrid &areaGridInst = iter->second;
        HelloKittyMsgData::AreaGrid *areaGrid = kittyGarden->add_areagrid();
        areaGridInst.save(areaGrid);
    }
    
    for(auto iter = m_pointGateMap.begin();iter != m_pointGateMap.end();++iter) 
    {
        GateGrid &gateGridInst = iter->second;
        HelloKittyMsgData::GateGrid *gateGrid = kittyGarden->add_gategrid();
        gateGridInst.save(gateGrid);
    }
    
    std::string ret;
    encodeMessage(&ackKittyGarden,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool SceneMapData::fullMessage(HelloKittyMsgData::UserBaseInfo& useInfo)
{
    HelloKittyMsgData::KittyGarden *kittyGarden = useInfo.mutable_kittygarden();
    if(!kittyGarden)
    {
        return false;
    }

    for(auto iter = m_pointAreaMap.begin();iter != m_pointAreaMap.end();++iter) 
    {
        AreaGrid &areaGridInst = iter->second;
        HelloKittyMsgData::AreaGrid *areaGrid = kittyGarden->add_areagrid();
        areaGridInst.save(areaGrid);
    }
    
    for(auto iter = m_pointGateMap.begin();iter != m_pointGateMap.end();++iter) 
    {
        GateGrid &gateGridInst = iter->second;
        HelloKittyMsgData::GateGrid *gateGrid = kittyGarden->add_gategrid();
        gateGridInst.save(gateGrid);
    }
    
    return true;
}

bool SceneMapData::fullMessage(HelloKittyMsgData::EnterGardenInfo& kittyMap)
{
    HelloKittyMsgData::KittyGarden *kittyGarden = kittyMap.mutable_kittygarden();
    if(!kittyGarden)
    {
        return false;
    }

    for(auto iter = m_pointAreaMap.begin();iter != m_pointAreaMap.end();++iter) 
    {
        AreaGrid &areaGridInst = iter->second;
        HelloKittyMsgData::AreaGrid *areaGrid = kittyGarden->add_areagrid();
        areaGridInst.save(areaGrid);
    }
    
    for(auto iter = m_pointGateMap.begin();iter != m_pointGateMap.end();++iter) 
    {
        GateGrid &gateGridInst = iter->second;
        HelloKittyMsgData::GateGrid *gateGrid = kittyGarden->add_gategrid();
        gateGridInst.save(gateGrid);
    }
    return true;
}

Point SceneMapData::exchangeArea(const Point &point)
{
    int x = floor(1.0 * point.x / AreaWide);
    int y = floor(1.0 * point.y / AreaHight);
    return Point(x,y);
}

bool SceneMapData::checkArea(BuildBase *buildBase,const Point &point)
{   
    Point areaPoint = exchangeArea(point);
    auto iter = m_pointAreaMap.find(areaPoint);
    if(iter == m_pointAreaMap.end())
    {
        opKittyGardenFailAck(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Garden_Area_Illeag);
        Fir::logger->error("[地图区域]:坐标点非法(%s,%d,%d,%d,%d)",m_mapName,point.x,point.y,areaPoint.x,areaPoint.y);
        return false;
    }
    if(iter->second.m_status != HelloKittyMsgData::AGS_Open)
    {
        opKittyGardenFailAck(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Garden_Area_Close);
        Fir::logger->error("[地图区域]:区域没有开放(%s,%d,%d,%d,%d)",m_mapName,point.x,point.y,areaPoint.x,areaPoint.y);
        return false;
    }
    if(m_pointBuildMap.find(point) != m_pointBuildMap.end())
    {
        if(buildBase)
        {
            //自身障碍不算障碍
            const std::set<QWORD> &tempSet = m_pointBuildMap[point];
            if(tempSet.empty() || (tempSet.find(buildBase->getID()) != tempSet.end() && tempSet.size() == 1))
            {
                return true;
            }
        }
        opKittyGardenFailAck(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Garden_Point_Barrier);
        Fir::logger->error("[地图区域]:区域有障碍点(%s,%d,%d,%d,%d)",m_mapName,point.x,point.y,areaPoint.x,areaPoint.y);
        return false;
    }

    return true;
}

bool SceneMapData::checkPoint(BuildBase *buildBase,const Point &point,const bool rationFlg)
{
    const Point &gridPoint = buildBase->confBase->getGridPoint();
    Point gridPt(gridPoint);
    //如果翻转
    if(rationFlg)
    {
        gridPt.x = gridPoint.y;
        gridPt.y = gridPoint.x;
    }
    for(int width = 0;width < gridPt.x;++width)
    {
        for(int height = 0;height < gridPt.y;++height)
        {
            Point tempPoint(point.x+width,point.y+height);
            if(!checkArea(buildBase,tempPoint))
            {
                return false;
            }
        }
    }

    return true;
}

bool SceneMapData::opCellPoint(BuildBase *buildBase,const Point &point,const bool &opType)
{
    if(!buildBase)
    {
        return false;
    }

    if(m_pointBuildMap.find(point) == m_pointBuildMap.end())
    {
        if(opType)
        {
            std::set<QWORD> buildSet;
            buildSet.insert(buildBase->getID());
            m_pointBuildMap.insert(std::pair<Point,std::set<QWORD>>(point,buildSet));
        }
        return true;
    }
    
    std::set<QWORD> &buildSet = m_pointBuildMap[point];
    if(buildSet.find(buildBase->getID()) != buildSet.end())
    {
        if(!opType)
        {
            buildSet.erase(buildBase->getID());
        }
        if(buildSet.empty())
        {
            m_pointBuildMap.erase(point);
        }
        return true;
    }
    if(opType)
    {
        buildSet.insert(buildBase->getID());
    }
    return true;
}

bool SceneMapData::addBuildPoint(BuildBase *buildBase,const Point &point,const bool rationFlg)
{
    const Point &gridPoint = buildBase->confBase->getGridPoint();
    Point gridPt(gridPoint);
    //翻转
    if(rationFlg)
    {
        gridPt.x = gridPoint.y;
        gridPt.y = gridPoint.x;
    }

    std::set<Point> buildPointSet;
    for(int width = 0;width < gridPt.x;++width)
    {
        for(int height = 0;height < gridPt.y;++height)
        {
            Point tempPoint(point.x+width,point.y+height);
            opCellPoint(buildBase,tempPoint,true);
            buildPointSet.insert(tempPoint);
        }
    }
    m_buildPointSet.insert(std::pair<QWORD,std::set<Point>>(buildBase->getID(),buildPointSet));
    return true;
}

bool SceneMapData::eraseBuildPoint(BuildBase *buildBase)
{
    auto iter = m_buildPointSet.find(buildBase->getID());
    if(iter == m_buildPointSet.end())
    {
        return true;
    }

    std::set<Point> &buildPointSet = iter->second;
    for(auto bIter = buildPointSet.begin();bIter != buildPointSet.end();++bIter)
    {
        const Point &point = *bIter;
        opCellPoint(buildBase,point,false);
    }
    m_buildPointSet.erase(iter);
    return true;
}

void SceneMapData::reset()
{
    m_pointBuildMap.clear();
    m_buildPointSet.clear();
    m_pointGateMap.clear();
    m_pointAreaMap.clear();
    m_areaCdSet.clear();
}

bool SceneMapData::fullMapData(const MapData *mapData)
{
    if(!mapData)
    {
        return false;
    }
    reset();

    const std::map<Point,AreaGrid>& tempAreaGrid = mapData->getAreaMap();
    for(auto iter = tempAreaGrid.begin();iter != tempAreaGrid.end();++iter)
    {
        m_pointAreaMap.insert(std::pair<Point,AreaGrid>(iter->first,iter->second));
    }
    
    const std::map<Point,GateGrid>& tempGateGrid = mapData->getGateMap();
    for(auto iter = tempGateGrid.begin();iter != tempGateGrid.end();++iter)
    {
        m_pointGateMap.insert(std::pair<Point,GateGrid>(iter->first,iter->second));
    }
    return true;
}

bool SceneMapData::checkOpenArea(const Point &areaPt)
{
    return m_pointAreaMap.find(areaPt) == m_pointAreaMap.end() ? false : m_pointAreaMap[areaPt].m_status == HelloKittyMsgData::AGS_Open;
}

bool SceneMapData::checkOpenNearArea(const Point &areaPt)
{
    //检测4周有没有开放
    for(int x = -1;x <= 1;++x)
    {
        Point nearPoint(areaPt.x+x,areaPt.y);
        if(nearPoint == areaPt)
        {
            continue;
        }
        if(checkOpenArea(nearPoint))
        {
            return true;
        }
    }
    
    for(int y = -1;y <= 1;++y)
    {
        Point nearPoint(areaPt.x,areaPt.y+y);
        if(nearPoint == areaPt)
        {
            continue;
        }
        if(checkOpenArea(nearPoint))
        {
            return true;
        }
    }
    return false;
}

bool SceneMapData::reqOpenArea(const HelloKittyMsgData::Point &point)
{
    Point areaPt(point);
    auto iter = m_pointAreaMap.find(areaPt);
    if(iter == m_pointAreaMap.end())
    {
        return false;
    }
    AreaGrid &areaGrid = iter->second;
    if(areaGrid.m_status != HelloKittyMsgData::AGS_Close)
    {
        opKittyGardenFailAck(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Garden_Area_Open_Again);
        return false;
    }
    if(!checkOpenNearArea(areaPt))
    {
        opKittyGardenFailAck(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Garden_Area_Around_Close);
        return false;
    }
    const pb::Conf_t_Spread *conf = tbx::Spread().get_base(m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Open_Grid_Time) + 1);
    if(!conf)
    {
        return false;
    }

    //人气值不够
    if(m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Now) < conf->spread->popular())
    {
        m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Popular_Now);
        //opKittyGardenFailAck(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Garden_Occupy);
        return false;
    }

    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"购买区域(%d,%d)",areaPt.x,areaPt.y);
    const std::map<DWORD,DWORD> &materialMap = conf->getMaterialMap();
    if(!materialMap.empty())
    {
        if(!m_owner->checkMaterialMap(materialMap,true) || !m_owner->reduceMaterialMap(materialMap,temp))
        {
            //opKittyGardenFailAck(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Garden_Occupy);
            return false;
        }
    }
    areaGrid.m_status = HelloKittyMsgData::AGS_CD;
    areaGrid.m_parseTime = SceneTimeTick::currentTime.sec();
    areaGrid.m_lastSec = conf->spread->time() < 100 ? 100 : conf->spread->time();
    areaGrid.m_totalSec = areaGrid.m_lastSec;
    m_areaCdSet.insert(areaPt);

    opKittyGardenSuccessAck(HelloKittyMsgData::Garden_Parse_Area,areaPt);
    m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Open_Grid_Time,1,temp,true);
    TaskArgue arg(Target_Add_Source,Attr_Open_Grid_Val,Attr_Open_Grid_Val,m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Open_Grid_Time));
    m_owner->m_taskManager.target(arg);

    AchieveArg achieveArg(Achieve_Target_Have,Achieve_Sub_Sorce_Num,Attr_Open_Grid_Val,m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Open_Grid_Time));
    m_owner->m_achievementManager.target(achieveArg);
    return true;
}

bool SceneMapData::flushOpenArea(const Point &areaPt)
{
    auto iter = m_pointAreaMap.find(areaPt);
    if(iter == m_pointAreaMap.end())
    {
        return false;
    }
    
    HelloKittyMsgData::AckFlushArea ackFlushArea;
    HelloKittyMsgData::AreaGrid *areaGrid = ackFlushArea.mutable_areagrid();
    iter->second.save(areaGrid);
   
    std::string ret;
    encodeMessage(&ackFlushArea,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool SceneMapData::opKittyGardenSuccessAck(const HelloKittyMsgData::GardenSuccessCodeType &code,const Point &areaPt)
{
    auto iter = m_pointAreaMap.find(areaPt);
    if(iter == m_pointAreaMap.end())
    {
        return false;
    }
   
    HelloKittyMsgData::AckGardenOpSuccess success;
    success.set_code(code);
    HelloKittyMsgData::AreaGrid *areaGrid = success.mutable_areagrid();
    if(!areaGrid)
    {
        return false;
    }
    iter->second.save(areaGrid);

    std::string ret;
    encodeMessage(&success,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool SceneMapData::opKittyGardenFailAck(const HelloKittyMsgData::ErrorCodeType &common,const HelloKittyMsgData::GardenFailCodeType &code)
{
    HelloKittyMsgData::AckGardenOpFail fail;
    fail.set_code(code);
    fail.set_commoncode(common);

    std::string ret;
    encodeMessage(&fail,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool SceneMapData::checkRoad(const Point &point)
{
    auto iter = m_pointBuildMap.find(point);
    if(iter == m_pointBuildMap.end())
    {
        return false;
    }
    const std::set<QWORD> &tempSet = iter->second;
    for(auto buildIter = tempSet.begin();buildIter != tempSet.end();++buildIter)
    {
        if(m_owner->m_buildManager.isRoad(*buildIter))
        {
            return true;
        }
    }
    return false;
}

bool SceneMapData::checkActive(BuildBase *buildBase,const Point &point,const bool rationFlg)
{
    //有些建筑不需要道路就可以激活
    if(!buildBase->confBase->buildInfo->roadactivetype())
    {
        return true;
    }
    const Point &gridPoint = buildBase->confBase->getGridPoint();
    Point gridPt(gridPoint);
    //如果翻转
    if(rationFlg)
    {
        gridPt.x = gridPoint.y;
        gridPt.y = gridPoint.x;
    }
    
    for(int height = 0;height < gridPt.y;++height)
    {
        Point tempLeft(point.x-1,point.y+height);
        Point tempRight(point.x+gridPt.x,point.y+height);
        if(checkRoad(tempLeft)||checkRoad(tempRight))
        {
            return true;
        }
    }
    
    for(int width = 0;width < gridPt.x;++width)
    {
        Point tempLow(point.x+width,point.y-1);
        Point tempUper(point.x+width,point.y+gridPt.y);
        if(checkRoad(tempLow)||checkRoad(tempUper))
        {
            return true;
        }
    }
    return false;
}

bool SceneMapData::loop()
{
    DWORD now = SceneTimeTick::currentTime.sec();
    std::set<Point> delSet;
    for(auto iter = m_areaCdSet.begin();iter != m_areaCdSet.end();++iter)
    {
            auto it = m_pointAreaMap.find(*iter);
            if(it == m_pointAreaMap.end())
            {
                continue;
            }
            AreaGrid &areaGrid = it->second;
            if(areaGrid.m_status != HelloKittyMsgData::AGS_CD)
            {
                continue;
            }
            DWORD sec = now - areaGrid.m_parseTime;
            if(sec >= areaGrid.m_lastSec)
            {
                areaGrid.m_lastSec = 0;
                areaGrid.m_status = HelloKittyMsgData::AGS_Open;
                opKittyGardenSuccessAck(HelloKittyMsgData::Garden_Open_Area,*iter);
                delSet.insert(*iter);
            }
    }

    for(auto iter = delSet.begin();iter != delSet.end();++iter)
    {
        m_areaCdSet.erase(*iter);
    }
    return true;
}

bool SceneMapData::parseCD(const Point &pt)
{
    auto it = m_pointAreaMap.find(pt);
    if(it == m_pointAreaMap.end())
    {
        return false;
    }
    AreaGrid &areaGrid = it->second;
    DWORD now = SceneTimeTick::currentTime.sec();
    DWORD lastSec = areaGrid.m_lastSec - (now - areaGrid.m_parseTime); 
    DWORD money = MiscManager::getMe().getMoneyForReduceTimer(eTimerReduce_Third,lastSec);
    char reMark[100] = {0};
    snprintf(reMark,sizeof(reMark),"扩地购买cd(%d,%d,%u)",pt.x,pt.y,lastSec);
    if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,money,reMark,false))
    {
        m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
        return false;
    }
    areaGrid.m_lastSec = 0;
    areaGrid.m_status = HelloKittyMsgData::AGS_Open;
    opKittyGardenSuccessAck(HelloKittyMsgData::Garden_Open_Area,areaGrid.m_point);
    m_areaCdSet.erase(areaGrid.m_point);
    return true;
}


