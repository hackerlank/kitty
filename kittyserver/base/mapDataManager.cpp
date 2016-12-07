#include "mapDataManager.h"

MapDataManager::MapDataManager()
{
}

bool MapDataManager::init(const Fir::XMLParser &xml)
{
    const Fir::XMLParser::Node *root = xml.root();
    if(!root)
    {
        Fir::logger->debug("[加载配置]:地图配置没有根节点");
        return false;
    }
    
    const Fir::XMLParser::Node *subNode = xml.child(root, "Map");
    while (subNode)
    {
        std::string path = "mapxml/";
        DWORD mapID = (DWORD)xml.node_attribute(subNode, "ID");
        std::string name = (const char*)xml.node_attribute(subNode, "Name");
        path += (const char*)xml.node_attribute(subNode, "Path");
        loadMapData(mapID,name.c_str(),path.c_str());
        subNode = xml.next(subNode, "Map");
    }
    return true;
}

bool MapDataManager::loadMapData(const DWORD mapID,const char *fileName,const char *path)
{
    auto iter = m_mapDataMap.find(mapID);
    if(iter != m_mapDataMap.end())
    {
        Fir::logger->debug("[加载配置]:记载地图数据重复(%s,%u)",fileName,mapID);
        return false;
    }
    
    MapData mapInst(mapID,fileName,path);
    XMLParser mapDataXml;
    mapDataXml.load_from_file(path);
    if(!mapInst.loadMapData(mapDataXml))
    {
        return false;
    }
    m_mapDataMap[mapID] = mapInst;
    
    return true;
}

const MapData* MapDataManager::getMapData(const DWORD mapID)
{
    const MapData* mapData = NULL;
    auto iter = m_mapDataMap.find(mapID);
    if(iter == m_mapDataMap.end())
    {
        Fir::logger->debug("[地图配置]:找不到对应的地图信息(%u)",mapID);
        return mapData;
    }
    mapData = &(iter->second);
    return mapData;
}

