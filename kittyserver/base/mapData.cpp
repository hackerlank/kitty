#include "mapData.h"
#include "dataManager.h"

MapData::MapData(const DWORD id,const char *fileName,const char *path) : m_mapID(id)
{
    bzero(m_mapName,sizeof(m_mapName));
    bzero(m_path,sizeof(m_path));
    if(fileName)
    {
        strncpy(m_mapName,fileName,sizeof(m_mapName));
    }
    if(path)
    {
        strncpy(m_path,path,sizeof(m_path));
    }
}

bool MapData::loadMapData(const Fir::XMLParser &xml)
{
    const Fir::XMLParser::Node *root = xml.root();
    if(!root)
    {
        Fir::logger->error("[加载配置]:记载地图数据失败,没有根节点 (%s)",m_mapName);
        return false;
    }
    
    const Fir::XMLParser::Node *mapInfoNode = xml.child(root, "sMapInfo");
    if(!mapInfoNode)
    {
        Fir::logger->error("[加载配置]:记载地图数据失败,没有sMapInfo(%s)",m_mapName);
        return false;
    }

    const Fir::XMLParser::Node *mapNode = xml.child(mapInfoNode, "sMapInfoItem");
    if (!mapNode || !xml.has_attribute(mapNode, "ID") || !xml.has_attribute(mapNode, "Name"))
    {
        Fir::logger->error("[加载配置]:地图配置没有ID属性(%s)",m_mapName);
        return false;
    }
    
    DWORD mapID = (DWORD)xml.node_attribute(mapNode,"ID");
    std::string mapName = (const char*)xml.node_attribute(mapNode,"Name");
    if(m_mapID != mapID || strncmp(m_mapName,mapName.c_str(),sizeof(m_mapName)))
    {
        Fir::logger->error("[加载配置]:地图配置ID和Name属性错误(%s,%u)",m_mapName,m_mapID);
        return false;
    }

    const Fir::XMLParser::Node *subNode = xml.child(mapNode,"AreaList");
    if(!subNode)
    {
        Fir::logger->error("[加载配置]:地图配置没有AreaList节点(%s)",m_mapName);
        return false;
    }
    loadAreaListNode(xml,subNode);
    
    subNode = xml.child(mapNode,"GateList");
    if(!subNode)
    {
        Fir::logger->error("[加载配置]:地图配置没有GateList节点(%s)",m_mapName);
        return false;
    }
    loadGateListNode(xml,subNode);
   // print();
    return true;

}

bool MapData::loadAreaListNode(const Fir::XMLParser &xml, const Fir::XMLParser::Node *node)
{
    const Fir::XMLParser::Node *subNode = xml.child(node, "MapAreaItem");
    if(!subNode)
    {
        Fir::logger->error("[加载配置]:地图配置没有MapAreaItem节点(%s)",m_mapName);
        return false;
    }

    while(subNode)
    {
        loadMapAreaItemNode(xml,subNode);
        subNode = xml.next(subNode, "MapAreaItem");
    }
   // print();
    return true;

}

bool MapData::loadMapAreaItemNode(const Fir::XMLParser &xml, const Fir::XMLParser::Node *node)
{
    if(!xml.has_attribute(node, "GridX") || !xml.has_attribute(node, "GridY") || !xml.has_attribute(node, "IsOpen"))
    {
        Fir::logger->error("[加载配置]:地图配置没有GridX或者GridY或者IsOpen属性(%s)",m_mapName);
        return false;
    }
    Point point;
    point.x = (int)xml.node_attribute(node,"GridX");
    point.y = (int)xml.node_attribute(node,"GridY");
    std::string openFlg = (const char*)xml.node_attribute(node,"IsOpen");
    HelloKittyMsgData::AreaGridStatus status = openFlg == "false" ? HelloKittyMsgData::AGS_Close : HelloKittyMsgData::AGS_Open;  
    
    AreaGrid areaGrid(point,status);
    auto iter = m_pointAreaMap.find(point);
    if(iter != m_pointAreaMap.end())
    {
        Fir::logger->error("[加载配置]:地图配置坐标点重复(%s,%d,%d)",m_mapName,point.x,point.y);
        return false;
    }
    if(!status)
    {
        std::string material = (const char*)xml.node_attribute(node,"OpenItems");
        std::map<DWORD,DWORD> materialMap;
        pb::parseDWORDToDWORDMap(material,materialMap);
        auto it = m_materialMap.find(point);
        if(it == m_materialMap.end())
        {
            m_materialMap.insert(std::pair<Point,std::map<DWORD,DWORD>>(point,materialMap));
        }
    }
#if 0
    size_t beforeSize = m_pointAreaMap.size();
#endif
    auto ret = m_pointAreaMap.insert(std::pair<Point,AreaGrid>(point,areaGrid));
#if 0
    m_pointAreaMap[point] = areaGrid;
    Fir::logger->info("[加载配置]:地图配置坐标点(%d,%d,%d,%lu,%s,%s,%s,%lu)",point.x,point.y,isOpen,beforeSize,(const char*)xml.node_attribute(node,"GridX"),(const char*)xml.node_attribute(node,"GridY"),(const char*)xml.node_attribute(node,"IsOpen"),m_pointAreaMap.size());
    print();
#endif
    return true;
}

bool MapData::loadGateListNode(const Fir::XMLParser &xml, const Fir::XMLParser::Node *node)
{
    const Fir::XMLParser::Node *subNode = xml.child(node, "MapGateItem");
    if(!subNode)
    {
        Fir::logger->error("[加载配置]:地图配置没有MapGateItem节点(%s)",m_mapName);
        return false;
    }

    while(subNode)
    {
        loadMapGateItemNode(xml,subNode);
        subNode = xml.next(subNode, "MapGateItem");
    }
    return true;

}

bool MapData::loadMapGateItemNode(const Fir::XMLParser &xml, const Fir::XMLParser::Node *node)
{
    if(!xml.has_attribute(node, "GridX") || !xml.has_attribute(node, "GridY"))
    {
        Fir::logger->error("[加载配置]:地图配置没有GridX或者GridY属性(%s)",m_mapName);
        return false;
    }
    Point point;
    point.x = (int)xml.node_attribute(node,"GridX");
    point.y = (int)xml.node_attribute(node,"GridY");

    GateGrid gateGrid(point);
    auto iter = m_pointGateMap.find(point);
    if(iter != m_pointGateMap.end())
    {
        Fir::logger->error("[加载配置]:地图配置坐标点重复(%s,%d,%d)",m_mapName,point.x,point.y);
        return false;
    }
    m_pointGateMap[point] = gateGrid;
    return true;
}

bool MapData::loadMapData(const HelloKittyMsgData::Serialize &binary,const DWORD now)
{
    const HelloKittyMsgData::KittyGarden& kittyGarden = binary.kittygarden();
    m_mapID = kittyGarden.mapid();
    strncpy(m_mapName,kittyGarden.mapname().c_str(),sizeof(m_mapName));
    for(int index = 0;index < kittyGarden.areagrid_size();++index)
    {
        const HelloKittyMsgData::AreaGrid &areaGrid = kittyGarden.areagrid(index);
        AreaGrid areaGridInst(areaGrid);
        if(m_pointAreaMap.find(areaGridInst.m_point) != m_pointAreaMap.end())
        {
            Fir::logger->error("[数据库加载地图]:地图配置坐标点重复(%s,%d,%d)",m_mapName,areaGridInst.m_point.x,areaGridInst.m_point.y);
            continue;
        }
        if(areaGridInst.m_lastSec)
        {
            if(now - areaGridInst.m_parseTime >= areaGridInst.m_lastSec)
            {
                areaGridInst.m_lastSec = 0;
                areaGridInst.m_status = HelloKittyMsgData::AGS_Open;
            }
            else
            {
                m_areaCdSet.insert(areaGridInst.m_point);
            }
        }
        m_pointAreaMap.insert(std::pair<Point,AreaGrid>(areaGridInst.m_point,areaGridInst));
    }
  
    for(int index = 0;index < kittyGarden.gategrid_size();++index)
    {
        const HelloKittyMsgData::GateGrid &gateGrid= kittyGarden.gategrid(index);
        GateGrid gateGridInst(gateGrid);
        if(m_pointGateMap.find(gateGridInst.m_point) != m_pointGateMap.end())
        {
            Fir::logger->error("[数据库加载地图]:地图配置(gate)坐标点重复(%s,%d,%d)",m_mapName,gateGridInst.m_point.x,gateGridInst.m_point.y);
            continue;
        }
        m_pointGateMap.insert(std::pair<Point,GateGrid>(gateGridInst.m_point,gateGridInst));
    }
    return true;
}

bool MapData::saveMapData(HelloKittyMsgData::Serialize &binary)
{
    HelloKittyMsgData::KittyGarden* kittyGarden = binary.mutable_kittygarden();
    if(!kittyGarden)
    {
        return false;
    }
    
    kittyGarden->set_mapid(m_mapID);
    kittyGarden->set_mapname(m_mapName);

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

const std::map<Point,AreaGrid>& MapData::getAreaMap() const
{
    return m_pointAreaMap;
}

const std::map<Point,GateGrid>& MapData::getGateMap() const
{
    return m_pointGateMap;
}

void MapData::print() const
{
    Fir::logger->debug("Test the KittyGarden Map begin");
    Fir::logger->debug("Test the KittyGarden Map area point begin (%lu)",m_pointAreaMap.size());
    size_t cnt = 0;
    for(auto iter = m_pointAreaMap.begin();iter != m_pointAreaMap.end();++iter)
    {
        const AreaGrid &gateGridInst = iter->second;
        const Point &key = iter->first;
        ++cnt;
        Fir::logger->debug("%d,%d,%d,%d,%d",key.x,key.y,gateGridInst.m_point.x,gateGridInst.m_point.y,gateGridInst.m_status);
    }
    Fir::logger->debug("Test the KittyGarden Map area point end (%lu)",cnt);
    Fir::logger->debug("Test the KittyGarden Map gate point begin");
    for(auto iter = m_pointGateMap.begin();iter != m_pointGateMap.end();++iter)
    {
        const GateGrid &gateGridInst = iter->second;
        const Point &key = iter->first;
        Fir::logger->debug("%d,%d,%d,%d",key.x,key.y,gateGridInst.m_point.x,gateGridInst.m_point.y);
    }
    Fir::logger->debug("Test the KittyGarden Map gate point end");
}

const std::map<DWORD,DWORD>& MapData::getMaterialMap(const Point &point) const
{
    static std::map<DWORD,DWORD> tempMap;
    auto iter = m_materialMap.find(point);
    if(iter == m_materialMap.end())
    {
        return tempMap;
    }
    return iter->second;
}
