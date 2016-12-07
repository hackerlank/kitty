#ifndef ADDRESS_MANAGER_H
#define ADDRESS_MANAGER_H
#include "giftpackage.pb.h"
#include "serialize.pb.h"
#include "zType.h"
#include <map>

class SceneUser;
class AddressManager
{
    public:
        AddressManager(SceneUser *owner);
        //加载
        bool load(const HelloKittyMsgData::Serialize& binary);
        //保存
        bool save(HelloKittyMsgData::Serialize& binary);
        //更新
        bool update();
        //更改地址
        bool changeAddress(const HelloKittyMsgData::AddressInfo &reqChange);
        //获得地址
        HelloKittyMsgData::AddressInfo* getAddress(const DWORD id); 
    private:
        SceneUser *m_owner;
        std::map<DWORD,HelloKittyMsgData::AddressInfo> m_addressMap;
};

#endif

