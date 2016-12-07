#ifndef MAP_DATA_H
#define MAP_DATA_H 

#include "xmlparser.h"
#include "pos.h"
#include <map>
#include <set>
#include "kittygarden.pb.h"
#include "serialize.pb.h"

//kitty乐园地图id为1
const DWORD KittyGardenMapID = 1;
//一个区域5格高宽5小格
const DWORD AreaWide = 5;
const DWORD AreaHight = 5;

class MapData
{
    protected:
        std::map<Point,AreaGrid> m_pointAreaMap;
        std::map<Point,GateGrid> m_pointGateMap;
        std::map<Point,std::map<DWORD,DWORD>> m_materialMap;
        std::set<Point> m_areaCdSet;
        DWORD m_mapID;
        char m_mapName[MAX_NAMESIZE];
        char m_path[MAX_NAMESIZE];
    public:
        MapData(const DWORD id = 0,const char *fileName = NULL,const char *path = NULL);
        bool loadMapData(const Fir::XMLParser &xml);
        bool loadMapData(const HelloKittyMsgData::Serialize &binary,const DWORD now);
        bool saveMapData(HelloKittyMsgData::Serialize &binary);
        //获得m_pointAreaMap
        const std::map<Point,AreaGrid>& getAreaMap() const;
        //获得m_pointGateMap
        const std::map<Point,GateGrid>& getGateMap() const;
        //打印日志测试
        void print() const; 
        //获得此点开放材料列表
        const std::map<DWORD,DWORD>& getMaterialMap(const Point &point) const;
    protected:
        //加载AreaList节点
        bool loadAreaListNode(const Fir::XMLParser &xml, const Fir::XMLParser::Node *node);
        //加载MapAreaItem节点
        bool loadMapAreaItemNode(const Fir::XMLParser &xml, const Fir::XMLParser::Node *node);
        //加载GateList节点
        bool loadGateListNode(const Fir::XMLParser &xml, const Fir::XMLParser::Node *node);
        //加载MapGateItem节点
        bool loadMapGateItemNode(const Fir::XMLParser &xml, const Fir::XMLParser::Node *node);

};
#endif

