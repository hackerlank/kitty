#ifndef ATLAS_MANAGER_H
#define ATLAS_MANAGER_H
#include <set>
#include "zType.h"
#include "serialize.pb.h"

class SceneUser;
class AtlasManager
{
    public:
        AtlasManager(SceneUser *owner);
        ~AtlasManager();
        //通过道具来判断是否开启图鉴
        bool addAtlasByItem(const DWORD itemID);
        //开通建筑图鉴
        bool addAtlasByBuild(const DWORD buildTypeID,const DWORD level);
        //刷新所有开启图鉴
        bool flushAtlas();
        //加载
        bool load(const HelloKittyMsgData::Serialize& binary);
        //保存
        bool save(HelloKittyMsgData::Serialize& binary);
        //获得已开启的图鉴数量
        DWORD getAtlasNum();
        //增加图鉴
        bool addAtlas(const DWORD type,const DWORD atlas);
        //删除某个开启的图鉴
        bool delAtlas(const DWORD type,const DWORD atlas);
        //清空所欲获得的图鉴
        bool clearAtlas();
        //重置
        void reset();
    private:
        //更新单个图鉴
        bool updateAtlas(const DWORD type,const DWORD atlas);
        //检查图鉴是否已开启
        bool checkAtlas(const DWORD type,const DWORD atlas);
    private:
        SceneUser *m_owner;
        std::map<DWORD,std::set<DWORD>> m_atlasMap;
};


#endif

