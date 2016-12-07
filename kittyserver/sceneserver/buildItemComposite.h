#ifndef BUILD_ITEM_COMPOSITE_H
#define BUILD_ITEM_COMPOSITE_H
#include "buildBase.h"
#include "composite.pb.h"
#include "serialize.pb.h"
#include "login.pb.h"
#include <list>
#include "buffer.h"

//合成道具类建筑

class BuildTypeCompositeItem : public BuildBase
{
    public:
        BuildTypeCompositeItem(SceneUser *owner,const DWORD typeID,const DWORD level,const Point &point = Point(),const bool active = false);
        BuildTypeCompositeItem(SceneUser *owner,const HelloKittyMsgData::BuildBase &buildBase);
        BuildTypeCompositeItem(SceneUser *owner,const pb::Conf_t_building *buildConf,const Point &point = Point());
    public:
        //加载数据
        bool load(const HelloKittyMsgData::CompositeInfo& binary);
        //保存数据
        virtual bool saveProduce(HelloKittyMsgData::Serialize& binary);
        //开始合并
        bool workComposite(const DWORD produceid);
        //初始化工作槽
        bool init();
        //工作槽的操作
        bool OpBTPType(const HelloKittyMsgData::BTPOpType &opType,const DWORD placeID);
        //loop
        virtual bool loop();
        //填充userinfo
        bool fullUserInfo(HelloKittyMsgData::UserBaseInfo& binary);
        //开通此类建筑刷新
        bool sendInfoMeg();
        //更新指定工作槽
        void update(const DWORD cellID = 0);
        //激活建筑
        virtual void processChangeStatus(const bool loginFlg);
        //保存产物数据
        bool saveProduce(HelloKittyMsgData::CompositeInfo *compositeInfo);
        //解锁工作槽
        bool unlock();
        //update 滞留产品
        void updateItem();
        //查找处于指定状态下的工作槽
        HelloKittyMsgData::CompositeCell* findStatusCell(const HelloKittyMsgData::PlaceStatusType &status);
        //更新指定工作槽
        void update(const HelloKittyMsgData::CompositeCell *cell);
        //重连
        bool fullUserInfo(HelloKittyMsgData::AckReconnectInfo& binary);
        void checkEffect(const DWORD effectID);
    private:
        //查找id对应的工作槽
        HelloKittyMsgData::CompositeCell* findIDCell(const DWORD placeID);
        //删掉某个id的工作槽
        void eraseIDCell(const DWORD placeID);
        //合成工作
        bool work();
        //购买工作槽
        bool purchaseCell();
        //购买cd
        bool purchaseCd();
        //领取道具
        bool getItem(const DWORD cellID);
        //检测cd
        bool checkCd();
        //初始化placeID集合
        void initPlaceID(const DWORD cnt);
        //工作下一个
        void workNext(const DWORD placeID,const DWORD sec = 0);
        //放入道具
        bool putItem(const DWORD itemID);
        //滞留物品是否已满
        bool fullItem();
        bool checkCdInit();
    private:
        //合并功能槽
        std::list<HelloKittyMsgData::CompositeCell> m_compositeList;
        //滞留产品
        std::vector<DWORD> m_itemList;
};

#endif
