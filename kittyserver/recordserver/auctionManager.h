#ifndef AUCTION_MANAGER_H
#define AUCTION_MANAGER_H
#include "zSingleton.h"
#include "zMemDB.h"
#include "GmToolCommand.h"
#include "giftpackage.pb.h"

const DWORD OPEN_MASK = 1;
class AuctionManager : public Singleton<AuctionManager>
{
    private:
        friend class Singleton<AuctionManager>;
        AuctionManager();
        ~AuctionManager();
    public:
        bool loop();
        //加载数据
        bool loadAuction();
        //GM设置礼品存库数量
        bool loadGiftConfig();
        bool saveGiftConfig();
        bool saveGiftConfigByType(GiftType giftType);
        bool loadbidCenterConfig();
        bool addGiftConfig(const HelloKittyMsgData::GiftConfig &config);
        bool delGiftConfig(const HelloKittyMsgData::GiftConfig &config);
        bool modifyGiftConfig(const HelloKittyMsgData::GiftConfig &config);
    private:
        //提交每一笔竞拍
        bool commitAuction(const DWORD auctionID);
        //自动举牌
        bool autoAuction(const DWORD auctionID);
        bool loadbidCenterConfigEmpty();
    private:
        DWORD m_maxID;
        DWORD m_minID;
};

#endif


