#ifndef _UTRADE_H
#define _UTRADE_H

#include "zType.h"
#include <map>
#include <set>
#include <bitset>
#include "TradeCommand.h"
#include "serialize.pb.h"
#include "enterkitty.pb.h"
#include "zRWLock.h"
#include "SceneCommand.h"

enum TradeStatusType
{
    Trade_status_Normal      =  0,   //正常状态
    Trade_status_Wait_Paper  =  1,   //等待生成paper状态
    Trade_status_Sync_Cell   =  2,   //等待sync状态
};

enum ItemTradeType
{
    ITT_None = 0, //可售可交易
    ITT_Forbid_Trade = 1, //禁止交易
    ITT_Forbid_Recycle = 2, //禁止回购
    ITT_Forbid_Both = 3, //都禁止
};

class SceneUser;
class UTrade
{
	public:
		UTrade(SceneUser* u);
		~UTrade();
        //序列化保存数据
		void save(HelloKittyMsgData::Serialize& binary);
        //反序列化还原数据
		void load(const HelloKittyMsgData::Serialize& binary);
        //刷新摊位
		bool flushSaleBooth();
        //在某个格子上放上东西
		bool addSallItem(const HelloKittyMsgData::ReqSallPutItem *cmd);
		//初始化摊位		
		bool initTradeInfo();
        //物品下架
        bool offSallItem(const DWORD cellID);
        //开通某个摊位
        bool openCeil();
        //请求paper
        bool requireCellPaper(const HelloKittyMsgData::ReqSellPaper *cmd);
        //打广告
        bool advertise(const DWORD cellID);
        //对被购买物品加锁
        void purchaseItemLock(const CMD::SCENE::t_UserPurchaseLockItem *cmd );
        //解锁和转移道具
        void purchaseUnLockItem(const CMD::SCENE::t_UserPurchaseUnlockeItem *cmd);
        //拿钱
        bool reqGetCellMoney(const DWORD cellID);
        //刷新摊位数据
        bool flushSaleBooth(HelloKittyMsgData::AckPbSaleCeilFlush &flush);
        //刷新广告cd时间
        bool ackAdvertiseTime();
        //购买广告cd
        bool parchaseCD();
        //及时刷新报纸
        void buyPaperItemSuccess(const QWORD ackCharID,const DWORD cellID);
        //npc回收
        void recycleItem();
        void openNpcStall();
        bool flushNpcSaleBooth();
        bool buyNpcStall(const DWORD cellID);
        void randNpcStallDay();
    private:
        //扩充一个格子
		bool addSaleCell(const DWORD cellid);
        //判断格子是否存在
		bool hasCell(const DWORD cellid);
        //获得摊位的格子
        HelloKittyMsgData::PbSaleCell* getCell(const DWORD cellid);
        //刷新单个单元格(客户端)
        bool flushStall(const DWORD cellID);
        //摊位操作失败原因
        bool opFailReturn(const HelloKittyMsgData::ErrorCodeType &commonError = HelloKittyMsgData::Error_Common_Occupy,const HelloKittyMsgData::TradeFailCodeType &code = HelloKittyMsgData::Trade_Occupy);
        //重置数据
        void reset();
        bool sendPaperMsg();

        void emptyCell(HelloKittyMsgData::PbSaleCell &cell);
        bool npcRandItem(HelloKittyMsgData::PbSaleCell &cell);
        bool flushNpcSaleBooth(HelloKittyMsgData::AckPbSaleCeilFlush &flush);
        void serializeAdPapre(HelloKittyMsgData::Serialize &binary);
        void parseAdPapre(const HelloKittyMsgData::Serialize &binary);
        bool whetherRandPaper();
        void updateRandPaperAd(const DWORD cellID,const bool opAdd);
        void randPaperAd();
        void getRandCellInfoVec(const QWORD charID,const std::set<QWORD> &cellIDSet,std::vector<HelloKittyMsgData::PbSaleCell> &cellInfoVec);
    private:
        SceneUser* m_owner;
        HelloKittyMsgData::PbSaleBooth m_saleBooth;
        //被购买道具枷锁
        zRWLock itemLock;
        //npc摊位
        std::map<DWORD,HelloKittyMsgData::PbSaleCell> m_npcStallMap;
        std::map<QWORD,HelloKittyMsgData::PbSaleCell> m_paperMap;
        DWORD m_randTime;
};

#endif

