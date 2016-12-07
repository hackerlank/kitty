#ifndef DRESS_MANAGER_H
#define DRESS_MANAGER_H
#include <set>
#include "zType.h"
#include "serialize.pb.h"

class SceneUser;
class DressManager
{
    public:
        DressManager(SceneUser *owner);
        ~DressManager();
        //通过道具来判断是否开启图鉴
        bool addDressByItem(const DWORD itemID);
        //刷新所有获得的时装
        bool flushDress();
        //加载
        bool load(const HelloKittyMsgData::Serialize& binary);
        //保存
        bool save(HelloKittyMsgData::Serialize& binary);
        //获得已获得的时装数量
        DWORD getDressNum();
        //清空所欲获得的时装
        bool clearDress();
        //升级
        bool upLevel(const DWORD dressID);
        //换时装
        bool changeDress(const DWORD dressID);
        //角色上线加时装buffer
        bool putUpDressInit();
        //查找大於或等於对应等级的时装数量
        DWORD findLevel(const DWORD level);
        //查找最高等级的时装
        DWORD findHigestLevel();
        void reset();
    private:
        //增加时装
        bool addDress(const DWORD dressID);
        //删除某个获得的时装
        bool delDress(const DWORD dressID);
        //更新单个时装
        bool updateDress(const DWORD dressID);
        //检查时装是否已获得
        bool checkDress(const DWORD dressID);
        //穿时装
        bool putUpDress(const DWORD dressID,const DWORD level);
        //脱时装
        bool putDownDress();
        //更新穿在身上的时装
        bool updateBodyDress();
    private:
        SceneUser *m_owner;
        std::map<DWORD,DWORD> m_dressMap;
};


#endif

