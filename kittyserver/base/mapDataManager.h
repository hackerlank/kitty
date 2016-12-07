#ifndef MAP_DATA_MANAGER_H
#define MAP_DATA_MANAGER_H

#include "mapData.h"

class MapDataManager
{
    private:
        std::map<DWORD,MapData> m_mapDataMap;
    public:
        MapDataManager();
        //加载所有地图配置
        bool init(const Fir::XMLParser &xml);
        //获得对应地图的指针
        const MapData* getMapData(const DWORD mapID);
    private:
        //加载地图数据
        bool loadMapData(const DWORD mapID,const char *fileName,const char *path);
};
#endif
