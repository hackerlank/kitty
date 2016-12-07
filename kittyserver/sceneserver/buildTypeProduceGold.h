#ifndef BUILD_TYPE_PRODUCE_GOLD_H
#define BUILD_TYPE_PRODUCE_GOLD_H
#include "buildBase.h"
#include "serialize.pb.h"

//产生金币类建筑

class BuildTypeProduceGold : public BuildBase
{
    public:
        BuildTypeProduceGold(SceneUser *owner,const DWORD typeID,const DWORD level,const Point &point = Point(),const bool active = false);
        BuildTypeProduceGold(SceneUser *owner,const HelloKittyMsgData::BuildBase &buildBase);
        BuildTypeProduceGold(SceneUser *owner,const pb::Conf_t_building *buildConf,const Point &point = Point());
    public:
        //加载数据
        bool load(const HelloKittyMsgData::BuildProduce& binary);
        //保存数据
        virtual bool saveProduce(HelloKittyMsgData::Serialize& binary);
        //填充产出信息
        void fullProduceMsg(HelloKittyMsgData::BuildProduce *buildProduce);
        //更新产出
        bool updateProduce();
        //点击结算产出
        bool clickReward();
        //保存产物信息
        void saveProduce(HelloKittyMsgData::WareHouseOtherInfo &produceOther);
        //初始化产物数据
        void initProduce(const HelloKittyMsgData::WareHouseOtherInfo &produceOther);
        //登录结算金币产出
        bool loginCommit();
        virtual void processChangeStatus(const bool loginFlg);
    private:
        //产出
        bool produce(const DWORD sec);
    private:
        //生产列表(按照策划要求，此处应该只会产生金币，但是为了扩展，写成map
        std::map<DWORD,double> m_produceMap;
};

#endif
