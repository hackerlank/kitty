#ifndef BUILD_ITEM_PRODUCT_H
#define BUILD_ITEM_PRODUCT_H
#include "buildBase.h"
#include "produce.pb.h"
#include "serialize.pb.h"
#include "login.pb.h"
#include "composite.pb.h"

//产生道具类建筑

class BuildTypeProduceItem : public BuildBase
{
    public:
        BuildTypeProduceItem(SceneUser *owner,const DWORD typeID,const DWORD level,const Point &point = Point(),const bool active = false);
        BuildTypeProduceItem(SceneUser *owner,const HelloKittyMsgData::BuildBase &buildBase);
        BuildTypeProduceItem(SceneUser *owner,const pb::Conf_t_building *buildConf,const Point &point = Point());
    public:
        //加载数据
        bool load(const HelloKittyMsgData::ProduceInfo& binary);
        //保存数据
        virtual bool saveProduce(HelloKittyMsgData::Serialize& binary);
        //更新工作槽
        bool updateProduceCell(const DWORD updateID = 0);
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
        //开始生产
        bool workProduce(const DWORD cellID,const DWORD produceid);
        //激活
        virtual void processChangeStatus(const bool loginFlg);
        //保存产物数据
        bool saveProduce(HelloKittyMsgData::ProduceInfo *produceInfo);
        //获得产出map
        std::map<DWORD,HelloKittyMsgData::ProduceCell>& getProduceMap();
        //重连
        bool fullUserInfo(HelloKittyMsgData::AckReconnectInfo& binary);
    private:
        //获得工作槽的实例
        HelloKittyMsgData::ProduceCell* getProduceCell(const DWORD cellID);
        //开通工作槽
        bool openCell(const DWORD cellID);
        //购买工作槽
        bool purchaseCell();
        //购买cd
        bool purchaseCd(const DWORD cellID);
        //领取道具
        bool getItem(const DWORD cellID);
        //检测cd
        bool checkCd();
        void checkEffect(const DWORD effectID);
        void effectSingle(HelloKittyMsgData::ProduceCell &cell);
    private:
        //产出功能槽
        std::map<DWORD,HelloKittyMsgData::ProduceCell> m_produceMap;
};

#endif
