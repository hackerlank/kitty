#ifndef SCENE_MAP_DATA_H
#define SCENE_MAP_DATA_H 
#include "mapData.h"
#include <set>
#include "common.pb.h"
#include "login.pb.h"
#include "enterkitty.pb.h"

class SceneUser;
class BuildBase;

class SceneMapData : public MapData
{
    private:
        std::map<Point,std::set<QWORD>> m_pointBuildMap;
        std::map<QWORD,std::set<Point>> m_buildPointSet;
        SceneUser *m_owner;
        QWORD m_ownerID;
    public:
        inline const std::map<Point,std::set<QWORD>>& getPointBuildMap() 
        {
            return m_pointBuildMap;
        }
        SceneMapData(SceneUser *owner,const DWORD id = 0,const char *fileName = NULL,const char *path = NULL);
        //刷新乐园消息
        bool flushMap();
        //检查点是否合法(默认不翻转)
        bool checkPoint(BuildBase *buildBase,const Point &point,const bool rationFlg = false);
        //增加网格点中建筑的id(默认不翻转)
        bool addBuildPoint(BuildBase *buildBase,const Point &point,const bool rationFlg = false);
        //减少网格点中建筑id
        bool eraseBuildPoint(BuildBase *buildBase);
        //填充MapData部分数据
        bool fullMapData(const MapData *mapData);
        //开启区域
        bool reqOpenArea(const HelloKittyMsgData::Point &point);
        //成功返回
        bool opKittyGardenSuccessAck(const HelloKittyMsgData::GardenSuccessCodeType &code,const Point &point);
        //失败返回
        bool opKittyGardenFailAck(const HelloKittyMsgData::ErrorCodeType &common = HelloKittyMsgData::Error_Common_Occupy,const HelloKittyMsgData::GardenFailCodeType &code = HelloKittyMsgData::Garden_Occupy);
        //填充userinfo消息
        bool fullMessage(HelloKittyMsgData::UserBaseInfo& useInfo);
        //填充EnterGardenInfo消息
        bool fullMessage(HelloKittyMsgData::EnterGardenInfo& kittyMap);
        //判断建筑是否被激活
        bool checkActive(BuildBase *buildBase,const Point &point,const bool rationFlg = false);
        //走cd
        bool loop();
        bool parseCD(const Point &pt);
    private:
        //从小格得到对应区域格
        Point exchangeArea(const Point &point);
        //检查区域格是否合法
        bool checkArea(BuildBase *buildBase,const Point &point);
        //每个格中建筑的id操作(true为增加)
        bool opCellPoint(BuildBase *buildBase,const Point &point,const bool &opType);
        //刷新区域开放点
        bool flushOpenArea(const Point &areaPt);
        //检测区域4周是否以开放(开放一处就算开放)
        bool checkOpenNearArea(const Point &areaPt);
        //检测区域点是否开放
        bool checkOpenArea(const Point &areaPt);
        //重置数据
        void reset();
        //判断此坐标是否有道路
        bool checkRoad(const Point &point);
};

#endif

