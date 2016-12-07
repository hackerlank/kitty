#ifndef GIFT_PACKAGE_H
#define GIFT_PACKAGE_H
#include "giftpackage.pb.h"
#include "serialize.pb.h"
#include "zType.h"
#include <set>

class SceneUser;

//实物描述
struct PhyCondInfo
{
    DWORD length;
    DWORD width;
    DWORD height;
    DWORD num;
    char des[100];
    PhyCondInfo()
    {
        length = 20;
        width = 10;
        height = 5;
        num = 10;
        bzero(des,sizeof(des));
    }
};

class GiftPackage
{
    public:
        GiftPackage(SceneUser *owner);
        //加载
        bool load(const HelloKittyMsgData::Serialize& binary);
        //存档
        bool save(HelloKittyMsgData::Serialize& binary);
        //插入数据库
        static bool insertDB(const QWORD sender,const QWORD accepter,const QWORD time,const HelloKittyMsgData::GiftContain &giftInfo);
        //过期检测
        bool delTimeOut();
        //接收礼品
        bool acceptGift(const QWORD sender,const QWORD time,const HelloKittyMsgData::GiftContain &giftInfo);
        //请求日志或者礼品
        bool reqGiftOp(const HelloKittyMsgData::ReqOpGift *cmd);
        //请求更新
        bool reqUpdate(const HelloKittyMsgData::ReqUpdate *cmd);
        //兑换实物
        bool reqCashGift(const HelloKittyMsgData::ReqCashGift *cmd);
        //确认领取实物
        bool reqCashGift(const HelloKittyMsgData::ReqCommitGift *cmd);
        //获得礼品(为真表示竞拍)
        bool addGift(HelloKittyMsgData::GiftInfo &giftInfo);
        //获得竞拍物品
        bool addGiftCash(HelloKittyMsgData::GiftCashInfo &giftInfo);
        //更改状态
        bool changeGiftStautus(const QWORD cashID,const DWORD status,const char *deliveryCompany = "上海通耀",const char *deliveryNum = "1223");
        //刷新库存
        bool updatePhyCondInfo(const DWORD id);
        //赠送鲜花
        bool sendFlower(const HelloKittyMsgData::ReqSendFlower *cmd);
        //初始化鲜花列表
        static void initFlowerSet();
    private:
        //生成唯一ID
        static QWORD generalID();
        //获得礼品实例
        HelloKittyMsgData::GiftInfo* getGift(const QWORD id);
        HelloKittyMsgData::GiftCashInfo* getGiftCash(const QWORD id);
        //更新礼品
        bool updateGift(const QWORD id);
        bool updateGiftCash(const QWORD id);

        //更新礼品
        bool updateGift(const std::set<QWORD> &idSet,const bool delFlg = false,const char *delReason = NULL);
        bool updateGiftCash(const std::set<QWORD> &idSet,const bool delFlg = false,const char *delReason = NULL);
        //从数据库中加载礼品
        bool loadDB();
        //发送礼品
        bool sendGift(const QWORD accepter,const std::set<QWORD> &idSet);
    private:
        SceneUser *m_owner;
        //id生成器
        static QWORD giftID;
        //兑换礼品容器
        std::map<QWORD,HelloKittyMsgData::GiftInfo> m_exchangeMap;
        //拍卖礼品容器
        std::map<QWORD,HelloKittyMsgData::GiftCashInfo> m_giftMap;
        //兑换id对应礼品id
        std::map<QWORD,std::set<QWORD>> m_cashGiftMap;
        //兑换的道具id对应的唯一IDmap(兑换的可以叠加)
        std::map<DWORD,QWORD> m_typeExchangeIDMap;
        //兑换背包改变标识
        bool m_exchangeFlg;
        //实物背包改变标识
        bool m_cashFlg;
        //库存表
        static std::map<DWORD,PhyCondInfo> s_PhyCondInfoMap;
        //鲜花类型集合
        static std::set<DWORD> s_flowerIDSet;
};

#endif
