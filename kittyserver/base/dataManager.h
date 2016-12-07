#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

//此文件是进一步包装excel打出来protobuf的类，方便程序中读取,类的命名规则为Conf_+需要包装的类,一定要包含一个key，此处的key可以自由发挥

#include <map>
#include "zType.h"
#include "Fir.h"
#include "ItemInfo.pb.h"
#include "Building.pb.h"
#include "pos.h"
#include <set>
#include <vector>
#include "TaskInfo.pb.h"
#include "Role.pb.h"
#include "AchievementInfo.pb.h"
#include "CarnivalData.pb.h"
#include "GardenActivetyItem.pb.h"
#include "NpcFriends.pb.h"
#include "Event.pb.h"
#include "event.pb.h"
#include "Param.pb.h"
#include "Production.pb.h"
#include "Buffer.pb.h"
#include "Dress.pb.h"
#include "Paper.pb.h"
#include "Divine.pb.h"
#include "CarnivalShop.pb.h"
#include "BurstEventNpc.pb.h"
#include "BurstEventReward.pb.h"
#include "WareHouseGrid.pb.h"
#include "Auction.pb.h"
#include "Order.pb.h"
#include "OrderFacilities.pb.h"
#include "ExchangeGift.pb.h"
#include "SlotMachine.pb.h"
#include "Macro.pb.h"
#include "SystemEmail.pb.h"
#include "FamilyLevel.pb.h"
#include "FamilyOrder.pb.h"
#include "FamilyPersonScore.pb.h"
#include "FamilyScore.pb.h"
#include "ConstellationStar.pb.h"
#include "AdditionalRewards.pb.h"
#include "NewGuide.pb.h"
#include "StarReward.pb.h"
#include "Shop.pb.h"
#include "SushiLevel.pb.h"
#include "ActiveToy.pb.h"
#include "Active.pb.h"
#include "DrawCapsuleType.pb.h"
#include "Composite.pb.h"
#include "Guide.pb.h"
#include "Spread.pb.h"
#include "BagValet.pb.h"
#include "OrderGoods.pb.h"
#include "SignInEveryDay.pb.h"
#include "SignInTotalDay.pb.h"
#include <functional>
#include "ChangeMoney.pb.h"
#include "AuctionCenter.pb.h"
#include "TrainOrderConfig.pb.h"
#include "BuildUpGrade.pb.h"
#include "BuildEffect.pb.h"
#include "Gold.pb.h"
#include "OpenFun.pb.h"
#include "Manservant.pb.h"
#include "ManservantClasses.pb.h"
#include "Market.pb.h"
#include "SushiSpend.pb.h"
#include "SushiRankReward.pb.h"
#include "UniteBuild.pb.h"
#include "UniteGrid.pb.h"
#include "UniteGridInit.pb.h"
#include "UniteBuildLevel.pb.h"
#include "VirtualGiftShop.pb.h"
#include "StarSpend.pb.h"
#include "RoleHeight.pb.h"
#include "RoleIncome.pb.h"
#include "RoleWeight.pb.h"
#include "RoleMaritalStatus.pb.h"
#include "RoleAge.pb.h"
#include "RoleSex.pb.h"
#include "ManservantBox.pb.h"
#include "TopupMall.pb.h"

namespace pb
{
    //以tag来划开字符串
    void parseTagString(const std::string &src,const std::string &tag,std::vector<std::string> &retVec);
    //解析point
    void parsePoint(const std::string &src,Point &destPoint,const std::string &tag = ",");
    //解析map
    void parseDWORDToDWORDMap(const std::string &src,std::map<DWORD,DWORD> &resultMap,const std::string &separatorTag = ",",const std::string &tag = "_");
    //解析vector
    void parseDWORDToVec(const std::string &src,std::vector<DWORD> &resultMap,const std::string &separatorTag = "_");
    //解析set
    void parseDWORDSet(const std::string &src,std::set<DWORD> &resultSet,const std::string &separatorTag = ",",const std::string &tag = "_");
    //解析map
    void parseStringToStringMap(const std::string &src,std::map<std::string,std::string> &resultMap,const std::string &separatorTag,const std::string &tag);
    //解析map<DWORD,std::map<DWORD,DWORD>>数据
    void parseDWORDToMapDWORD(const std::string &src,std::map<DWORD,std::map<DWORD,DWORD>> &resultMap,const std::string &separatorTag = ",",const std::string &tag = "_");
    //buffer结构体
    struct BufferMsg
    {
        //id
        DWORD id;
        //持续时间
        DWORD time;
        BufferMsg()
        {
            id = 0;
            time = 0;
        }
    };
    //解析buffer
    void parseBuffer(const std::string &src,std::map<DWORD,pb::BufferMsg>& bufferMap);
    void parseBufferVec(const std::string &src,std::vector<pb::BufferMsg>& bufferVec);

    struct TwoArgPara
    {
        DWORD para1;
        DWORD para2;
    };

    struct ThreeArgPara
    {
        DWORD para1;
        DWORD para2;
        DWORD para3;
    };

    enum StarRet
    {
        SR_Signle = 0,//单人
        SR_Operator = 1, //协作
        SR_Sport_Win = 2,//竞技赢
        SR_Sport_Fail = 3,//竞技输
        SR_Sport_Peace = 4,//竞技平
    };


    //解析权重
    void parseThreeArgParaVec(const std::string &src,std::vector<pb::ThreeArgPara>& paraVec,DWORD &weight);

    void parseTwoArgParaVec(const std::string &src,std::vector<pb::TwoArgPara>& paraVec,DWORD &weight);

    //道具表
    class Conf_t_itemInfo
    {
        private:
            QWORD key;
            std::map<DWORD,pb::BufferMsg> bufferMap;
            std::map<DWORD,DWORD>atlasMap;
            std::map<DWORD,DWORD>resolveMap;
            std::map<DWORD,DWORD> recycleMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::map<DWORD,DWORD>& getRecycle() const
            {
                return recycleMap;
            }
            inline const std::map<DWORD,pb::BufferMsg>& getBufferMap() const
            {
                return bufferMap;
            }
            inline const std::map<DWORD,DWORD>& getAtlasMap() const
            {
                return atlasMap;
            }
            inline const std::map<DWORD,DWORD>& getResolveMap() const
            {
                return resolveMap;
            }
            static void initStarMap(const DWORD starLevel,const DWORD itemID);
            //随机出某个星级道具
            static DWORD randStarItem(const DWORD starLevel);
            //根据玩家等级计算车厢数，装载物品，奖励物品
            static bool getTrainOrderInfo(DWORD playerlevel,DWORD TrainSize,std::vector<DWORD> &loadvec,std::vector<DWORD> &awardvec);
            static DWORD getRandItemID(DWORD lowlevel,DWORD highlevel,const std::set<DWORD>& excptSet);
            static DWORD randItemByLevel(const std::set<DWORD> &rewardSet,const std::set<DWORD> &itemLevelSet);
            static DWORD randItemByReward(const std::set<DWORD> &rewardSet,const std::set<DWORD> &itemLevelSet,const DWORD level);
            static DWORD randItemByReward(const DWORD rewardID,const DWORD level);
        public:
            Conf_t_itemInfo(const pb::itemInfo::t_itemInfo *_itemInfo) : itemInfo(_itemInfo) {};
            Conf_t_itemInfo() : itemInfo(NULL) {};
            bool init();
        public:
            const pb::itemInfo::t_itemInfo *itemInfo;
            static std::map<DWORD,std::vector<DWORD>> starMap;
            static std::multimap<DWORD,QWORD> m_maptrainorderload;//火车订单装载 等级-id 对应表
            static std::multimap<DWORD,QWORD> m_maptrainorderaward;//火车订单运达材料奖励 等级-id 对应表 
            static std::map<DWORD,std::vector<QWORD> > m_mapAllItem;
            static std::map<DWORD,std::map<DWORD,std::vector<DWORD> > >m_rewardLevelMap;
            static std::map<DWORD,DWORD> m_levelMap;
    }; 
    //建筑表
    class Conf_t_building
    {
        private:
            QWORD key;
            //升级材料列表
            std::map<DWORD,DWORD> materialMap;
            //解锁列表
            std::map<DWORD,DWORD> unLockMap;
            //合并列表
            std::map<DWORD,std::map<DWORD,DWORD>> compositeMap;
            //长和宽网格数
            Point gridPoint;
            //影响范围
            Point effectPoint;
            //buffer影响表
            std::map<DWORD,pb::BufferMsg> effectMap;
            //是否施加buffer
            bool bufferFlg;
            //图鉴信息(类型和id)
            std::map<DWORD,DWORD> atlasMap;
            //图鉴分解
            std::map<DWORD,DWORD>resolveMap;
            //激活材料
            std::map<DWORD,DWORD> activeMaterialMap;
        private:
            //初始化各种表
            void initAttrMap();
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD>& getActiveMaterialMap() const
            {
                return activeMaterialMap;
            }
            inline const std::map<DWORD,DWORD>& getMaterialMap() const
            {
                return materialMap;
            }
            inline const std::map<DWORD,DWORD>& getUnlockMap() const
            {
                return unLockMap;
            }
            inline const Point& getGridPoint() const
            {
                return gridPoint;
            }
            inline const std::map<DWORD,std::map<DWORD,DWORD>>& getCompositeMap() const
            {
                return compositeMap;
            }
            inline const Point& getEffectPoint() const
            {
                return effectPoint;
            }
            inline const std::map<DWORD,pb::BufferMsg>& getEffectMap() const
            {
                return effectMap;
            }
            inline const bool getBufferFlg() const
            {
                return bufferFlg;
            }
            inline const std::map<DWORD,DWORD>& getAtlasMap() const
            {
                return atlasMap;
            }
            inline const std::map<DWORD,DWORD>& getResolveMap() const
            {
                return resolveMap;
            }
        public:
            Conf_t_building(const pb::building::t_building *_buildInfo) : buildInfo(_buildInfo) {};
            Conf_t_building() : buildInfo(NULL) {};
            bool init();
        public:
            const pb::building::t_building *buildInfo;
    };

    struct InitBuildInfo
    {
        DWORD buildID;     //id 
        DWORD buildLevel;  //等级
        Point point;       //初始坐标
        InitBuildInfo(const DWORD _buildID = 0,const DWORD _buildLevel = 0,const Point &_point = Point()) : buildID(_buildID),buildLevel(_buildLevel),point(_point)
        {
        }

        InitBuildInfo(const InitBuildInfo &initBuildInfo)
        {
            buildID = initBuildInfo.buildID;
            buildLevel = initBuildInfo.buildLevel;
            point = initBuildInfo.point;
        }

        bool operator < (const InitBuildInfo &initBuildInfo) const
        {
            if(point.x == initBuildInfo.point.x)
            {
                return point.y  < initBuildInfo.point.y;
            }
            return point.x < initBuildInfo.point.x;
#if 0
            if(buildID < initBuildInfo.buildID)
            {
                return true;
            }
            return buildLevel < initBuildInfo.buildLevel;
#endif
        }

    };

    //新角色属性表
    class Conf_t_newRoleAttr
    {
        private:
            QWORD key;
            std::set<InitBuildInfo> buildLevelSet;
            std::map<DWORD,DWORD> itemMap;
            //       std::set<Point> roadPtSet;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::set<InitBuildInfo>& getBuildLevelSet() const
            {
                return buildLevelSet;
            }
            inline const std::map<DWORD,DWORD>& getItemMap() const
            {
                return itemMap;
            }
            /*   
                 inline const std::set<Point>& getRoadPtSet() const
                 {
                 return roadPtSet;
                 }
                 */
        public:
            Conf_t_newRoleAttr(const pb::newRoleAttr::t_newRoleAttr *_roleAttr) : roleAttr(_roleAttr) {};
            Conf_t_newRoleAttr() : roleAttr(NULL) {};
            bool init();
        private:
            //解析buildLevelMap
            void initBuildLevelSet();
            //解析itemMap
            void initItemMap();
        public:
            const pb::newRoleAttr::t_newRoleAttr *roleAttr;
    };

    struct TaskTarget
    {
        DWORD para1;
        DWORD para2;
        DWORD para3;
        DWORD para4;
        TaskTarget()
        {
            bzero(this,sizeof(*this));
        }
    };
    //任务表
    class Conf_t_Task
    {
        private:
            QWORD key;
            std::set<QWORD> preTaskSet;
            std::map<DWORD,DWORD> rewardMap;
            std::map<DWORD,TaskTarget> targetMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::set<QWORD>& getPreTaskSet() const
            {
                return preTaskSet;
            }
            inline const std::map<DWORD,DWORD>& getRewardMap() const
            {
                return rewardMap;
            }
            inline const std::map<DWORD,TaskTarget>& getTargetMap() const
            {
                return targetMap;
            }
        public:
            Conf_t_Task(const pb::Task::t_Task *_task) : task(_task) {};
            Conf_t_Task() : task(NULL) {};
            bool init();
        private:
            //初始化前置任务列表
            void initPreTaskSet();
            //初始化奖励列表
            void initRewardMap();
            //初始化目标列表
            void initTargetMap();
        public:
            const pb::Task::t_Task *task;
            //所有前置任务对应的任务列表
            static std::map<QWORD,std::set<QWORD>> s_allPreTaskMap;
    };

    //角色升级表
    class Conf_t_upgrade
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> rewardMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::map<DWORD,DWORD> getRewardMap() const
            {
                return rewardMap;
            }
        public:
            Conf_t_upgrade(const pb::upgrade::t_upgrade *_upgrade) : upgrade(_upgrade) {};
            Conf_t_upgrade() : upgrade(NULL) {};
            bool init();
        public:
            const pb::upgrade::t_upgrade *upgrade;
    };

    //成就
    class Conf_t_Achievement
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> rewardMap;
            std::map<DWORD,TaskTarget> targetMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD>& getRewardMap() const
            {
                return rewardMap;
            }
            inline const std::map<DWORD,TaskTarget>& getTargetMap() const
            {
                return targetMap;
            }
        public:
            Conf_t_Achievement(const pb::Achievement::t_Achievement *_achievement) : achievement(_achievement) {};
            Conf_t_Achievement() : achievement(NULL) {};
            bool init();
        private:
            //初始化奖励列表
            void initRewardMap();
            //初始化目标列表
            void initTargetMap();
        public:
            const pb::Achievement::t_Achievement *achievement;
    };

    //嘉年华
    class Conf_t_CarnivalData
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_CarnivalData(const pb::CarnivalData::t_CarnivalData *_carnival) : carnival(_carnival) {};
            Conf_t_CarnivalData() : carnival(NULL) {};
            bool init();
        public:
            const pb::CarnivalData::t_CarnivalData *carnival;
    };
    class Conf_t_rubbish
    {
        private: 
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_rubbish(const pb::rubbish::t_rubbish *_rubbish) : rubbish(_rubbish) {};
            Conf_t_rubbish() : rubbish(NULL) {};
            bool init();
        public:
            const pb::rubbish::t_rubbish *rubbish;
            HelloKittyMsgData::vecAward reward;

    };
    class Conf_t_event
    {
        private: 
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }

        public:
            Conf_t_event(const pb::event::t_event *_event) : event(_event) {};
            Conf_t_event() : event(NULL) {};
            bool init();
        public:
            const pb::event::t_event *event;
            DWORD reflushtimemin;
            DWORD reflushtimemax;
            std::vector<DWORD> buildevent;
            std::vector<DWORD> target;
            HelloKittyMsgData::vecAward rewardower;
            HelloKittyMsgData::vecAward rewardguess;

    };
    //参数表
    class Conf_t_param
    {
        private: 
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }

        public:
            Conf_t_param(const pb::param::t_param *_param) : param(_param) {};
            Conf_t_param() : param(NULL) {};
            bool init();
        public:
            const pb::param::t_param *param;
            std::vector<DWORD> vecParam;//数组参数

    };

    //产生
    class Conf_t_produceItem
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> materialMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD>& getMaterialMap() const
            {
                return materialMap;
            }
            static QWORD getIdByItem(DWORD); 
        public:
            Conf_t_produceItem(const pb::produceItem::t_produceItem *_produceItem) : produceItem(_produceItem) {};
            Conf_t_produceItem() : produceItem(NULL) {};
            bool init();
        public:
            const pb::produceItem::t_produceItem *produceItem;
            static std::map<DWORD,QWORD> mapItemID;
    };


    //buffer
    class Conf_t_buffer
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_buffer(const pb::buffer::t_buffer *_buffer) : buffer(_buffer) {};
            Conf_t_buffer() : buffer(NULL) {};
            bool init();
        public:
            const pb::buffer::t_buffer *buffer;
    };

    //时装
    class Conf_t_Dress
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> materialMap;
            std::map<DWORD,pb::BufferMsg> bufferMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD>& getMaterialMap() const
            {
                return materialMap;
            }
            inline const std::map<DWORD,pb::BufferMsg>& getBufferMap() const
            {
                return bufferMap;
            }
        public:
            Conf_t_Dress(const pb::Dress::t_Dress *_dress) : dress(_dress) {};
            Conf_t_Dress() : dress(NULL) {};
            bool init();
        public:
            const pb::Dress::t_Dress *dress;
    };

    //图纸
    class Conf_t_Paper
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> materialMap;
            std::map<DWORD,DWORD> produceMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD>& getMaterialMap() const
            {
                return materialMap;
            }
            inline const std::map<DWORD,DWORD>& getProduceMap() const
            {
                return produceMap;
            }
        public:
            Conf_t_Paper(const pb::Paper::t_Paper *_paper) : paper(_paper) {};
            Conf_t_Paper() : paper(NULL) {};
            bool init();
        public:
            const pb::Paper::t_Paper *paper;
    };

    //占卜
    class Conf_t_Divine
    {
        private:
            QWORD key;
            DWORD weight;
            DWORD bufferWeight;
            std::map<DWORD,DWORD> rewardMap;
            std::vector<pb::ThreeArgPara> argVec;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline DWORD getWeight() const
            {
                return weight;
            }
            inline DWORD getBufferWeight() const
            {
                return bufferWeight;
            }
            inline const std::map<DWORD,DWORD> &getRewardMap() const
            {
                return rewardMap;
            }
            inline const std::vector<pb::ThreeArgPara>& getBufferVec() const
            {
                return argVec;
            }
        public:
            Conf_t_Divine(const pb::Divine::t_Divine *_divine) : divine(_divine) {};
            Conf_t_Divine() : divine(NULL) {};
            bool init();
        public:
            const pb::Divine::t_Divine *divine;
            static std::map<DWORD,DWORD> retWeightMap;
    };

    //嘉年华商店
    class Conf_t_CarnivalShop
    {
        private:
            QWORD key;
            Point  point;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const  Point& getPoint() const
            {
                return point;
            }
        public:
            Conf_t_CarnivalShop(const pb::CarnivalShop::t_CarnivalShop* _carnivalShop) : carnivalShop(_carnivalShop) {};
            Conf_t_CarnivalShop() : carnivalShop(NULL) {};
            bool init();
        public:
            const pb::CarnivalShop::t_CarnivalShop *carnivalShop;
    };

    //突发事件奖励池
    class Conf_t_BurstEventReward
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> randItemMap;
            std::map<DWORD,DWORD> rewardMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD>& getRandItemMap() const
            {
                return randItemMap;
            }
            inline const std::map<DWORD,DWORD>& getRewardMap() const
            {
                return rewardMap;
            }
            bool getRandReward(DWORD &type,DWORD &value) const;
            static DWORD randExceptReward(const DWORD level,const std::set<DWORD>&exceptRewardSet);
        public:
            Conf_t_BurstEventReward(const pb::BurstEventReward::t_BurstEventReward* _burstEventReward) : burstEventReward(_burstEventReward) {};
            Conf_t_BurstEventReward() : burstEventReward(NULL) {};
            bool init();
        public:
            const pb::BurstEventReward::t_BurstEventReward *burstEventReward;
            static std::map<DWORD,std::vector<DWORD>> levelGradeRewardMap;
    };

    //突发事件npc池
    class Conf_t_BurstEventNpc
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            static DWORD randExceptNpc(const DWORD level,const std::set<DWORD>&exceptNpcSet);
        public:
            Conf_t_BurstEventNpc(const pb::BurstEventNpc::t_BurstEventNpc* _burstEventNpc) : burstEventNpc(_burstEventNpc) {};
            Conf_t_BurstEventNpc() : burstEventNpc(NULL) {};
            bool init();
        public:
            const pb::BurstEventNpc::t_BurstEventNpc *burstEventNpc;
            static std::map<DWORD,std::set<DWORD>> levelGradeNpcMap;
    };

    //扩充仓库格子
    class Conf_t_WareHouseGrid
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> materialMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD>& getMaterialMap() const
            {
                return materialMap;
            }
        public:
            Conf_t_WareHouseGrid(const pb::WareHouseGrid::t_WareHouseGrid *_gridInfo) : gridInfo(_gridInfo) {};
            Conf_t_WareHouseGrid() : gridInfo(NULL) {};
            bool init();
        public:
            const pb::WareHouseGrid::t_WareHouseGrid *gridInfo;
    };

    //神秘商店道具池
    class Conf_t_ItemPool
    {
        private:
            QWORD key;
            DWORD weight;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline DWORD getWeight() const
            {
                return weight;
            }
            static DWORD randID(const DWORD poolID);
        private:
            void initWeightMap();
        public:
            Conf_t_ItemPool(const pb::ItemPool::t_ItemPool *_pool) : pool(_pool) {};
            Conf_t_ItemPool() : pool(NULL) {};
            bool init();
        public:
            const pb::ItemPool::t_ItemPool *pool;
            static std::map<DWORD,std::map<DWORD,DWORD>> poolMap; 
            static std::map<DWORD,DWORD> poolSumWeightMap;
    };

    //拍卖行
    class Conf_t_Auction
    {
        private:
            QWORD key;
            DWORD beginTime;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline DWORD getBeginTime() const
            {
                return beginTime;
            }
        public:
            Conf_t_Auction(const pb::Auction::t_Auction *_auction) : auction(_auction) {};
            Conf_t_Auction() : auction(NULL) {};
            bool init();
        public:
            const pb::Auction::t_Auction * auction;
    };
    //订单
    class Conf_t_order
    {
        private: 
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }

        public:
            Conf_t_order(const pb::order::t_order *_order) : order(_order) {};
            Conf_t_order() : order(NULL) {};
            bool init();
            static void getOrderIdbyLv(DWORD level,std::vector<QWORD> &vecOrder);
        public:
            const pb::order::t_order *order;
            HelloKittyMsgData::vecAward needitem;
            HelloKittyMsgData::vecAward awarditem;
            static std::map<DWORD,std::vector<QWORD>> m_maplevelOrder;

    };

    //兑换
    class Conf_t_ExchangeGift
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_ExchangeGift(const pb::ExchangeGift::t_ExchangeGift *_gift) : gift(_gift) {};
            Conf_t_ExchangeGift() : gift(NULL) {};
            bool init();
        public:
            const pb::ExchangeGift::t_ExchangeGift *gift;
    };

    //老虎机
    class Conf_t_SlotMachine
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            //随机某个icon组
            static DWORD randSlotKey();
        public:
            Conf_t_SlotMachine(const pb::SlotMachine::t_SlotMachine *_slot) : slot(_slot) {};
            Conf_t_SlotMachine() : slot(NULL) {};
            bool init();
        public:
            const pb::SlotMachine::t_SlotMachine *slot;
            static DWORD sumWeight;
            static std::map<DWORD,DWORD> idToWeightMap;
    };

    //万家乐
    class Conf_t_Macro
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            //随机某个icon组
            static DWORD randMacroKey();
        public:
            Conf_t_Macro(const pb::Macro::t_Macro *_macro) : macro(_macro) {};
            Conf_t_Macro() : macro(NULL) {};
            bool init();
        public:
            const pb::Macro::t_Macro *macro;
            static DWORD sumWeight;
            static std::map<DWORD,DWORD> idToWeightMap;
    };

    //邮件描述
    class Conf_t_SystemEmail
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_SystemEmail(const pb::SystemEmail::t_SystemEmail *_systemEmail) : systemEmail(_systemEmail) {};
            Conf_t_SystemEmail() : systemEmail(NULL) {};
            bool init();
        public:
            const pb::SystemEmail::t_SystemEmail *systemEmail;
    };
    //家庭
    class Conf_t_familylevel
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_familylevel(const pb::familylevel::t_familylevel *_familylevel) : familylevel(_familylevel) {};
            Conf_t_familylevel() : familylevel(NULL) {};
            bool init();
            static QWORD getKeybyScore(DWORD score);
        public:
            const pb::familylevel::t_familylevel *familylevel;
            static std::map<DWORD,QWORD> m_mapScoreLv;
    };
    //家庭订单
    class Conf_t_familyorder
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            static void getOrderForAllLevel(std::vector<QWORD> &rvec); 

        public:
            Conf_t_familyorder(const pb::familyorder::t_familyorder *_familyorder) : familyorder(_familyorder) {};
            Conf_t_familyorder() : familyorder(NULL) {};
            bool init();
            const HelloKittyMsgData::Award * getRandAward() const;
        public:
            HelloKittyMsgData::vecAward needitem;
            HelloKittyMsgData::vecAward awarditem;
            std::vector<DWORD> vecrate;
            static std::map<DWORD,std::vector<QWORD> > m_maplvorder;

            const pb::familyorder::t_familyorder *familyorder;
    };
    //家庭个人排名
    class Conf_t_familypersonscore
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_familypersonscore(const pb::familypersonscore::t_familypersonscore *_familypersonscore) : familypersonscore(_familypersonscore) {};
            Conf_t_familypersonscore() : familypersonscore(NULL) {};
            bool init();
        public:
            HelloKittyMsgData::vecAward awarditem;
            const pb::familypersonscore::t_familypersonscore *familypersonscore;
    };
    //家庭贡献
    class Conf_t_familyscore
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_familyscore(const pb::familyscore::t_familyscore *_familyscore) : familyscore(_familyscore) {};
            Conf_t_familyscore() : familyscore(NULL) {};
            bool init();
            static QWORD getKeybyScore(DWORD score);
        public:
            HelloKittyMsgData::vecAward awarditem;
            const pb::familyscore::t_familyscore *familyscore;
            static std::map<DWORD,QWORD> m_mapScoreLv;  
    };
    //额外奖励
    class Conf_t_buildOption
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            struct RateAward
            {
                DWORD rate;
                HelloKittyMsgData::vecAward awarditem;
            };
        public:
            Conf_t_buildOption(const pb::buildOption::t_buildOption *_buildOption) : buildOption(_buildOption) {};
            Conf_t_buildOption() : buildOption(NULL) {};
            bool init();
            const HelloKittyMsgData::vecAward * getAward() const ;
        public:
            std::vector<RateAward> m_vecAward;
            const pb::buildOption::t_buildOption *buildOption;
    };
    //新手引导
    class Conf_t_NewGuide
    {
        private:
            QWORD key;
            std::set<DWORD> stopCDSet;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::set<DWORD>& getCDSet() const
            {
                return stopCDSet;
            }
        public:
            Conf_t_NewGuide(const pb::NewGuide::t_NewGuide *_NewGuide) : NewGuide(_NewGuide) {};
            Conf_t_NewGuide() : NewGuide(NULL) {};
            bool init();
            static QWORD getNextGuide(QWORD curkey,bool bgoback);
            static QWORD getlastGuide();
        public:
            static std::set<QWORD> m_setGuide;
            static std::map<QWORD,QWORD> m_mapGuide;
            HelloKittyMsgData::vecAward awarditem;
            const pb::NewGuide::t_NewGuide *NewGuide;
    };
    struct st_Guide
    {
        DWORD m_ID;
        DWORD m_param;
        std::set<DWORD> m_step;
        std::map<DWORD,DWORD> m_goBack;
        DWORD getNextstep(DWORD dwStep,bool bgoback) const ;
    };
    class Conf_t_Guide
    {
        private:
            QWORD key;
            std::set<DWORD> stopCDSet;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::set<DWORD>& getCDSet() const
            {
                return stopCDSet;
            }
            enum TIGGERTYPE
            {
                TIGGERTYPE_MIN = 0 ,
                TIGGERTYPE_LEVEL  =1,
                TIGGERTYPE_BUILD  =2,
                TIGGERTYPE_MAX    =3,
            };
        public:
            Conf_t_Guide(const pb::Guide::t_Guide *_Guide) : Guide(_Guide) {};
            Conf_t_Guide() : Guide(NULL) {};
            bool init();
            static const st_Guide* getGuideByType(DWORD type,DWORD level);
            static const st_Guide* getGuideById(DWORD guidId);
            static std::map<TIGGERTYPE,std::map<DWORD,DWORD> >m_mapTaskguide;//任务触发    
            static std::map<DWORD,st_Guide> m_mapguide;//引导索引
        public:
            HelloKittyMsgData::vecAward awarditem;
            const pb::Guide::t_Guide *Guide;
    };
    //星座关卡奖励
    class Conf_t_StarReward
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> rewardMap;
            std::map<DWORD,DWORD> typeWeightMap;
            std::map<DWORD,std::vector<pb::ThreeArgPara> > typeRewardMap;
            std::vector<pb::ThreeArgPara> argVec;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::map<DWORD,DWORD>& getRewardMap() const
            {
                return rewardMap;
            }
            bool randReward(const StarRet &starRet,std::map<DWORD,DWORD> &randMap) const;
        public:
            Conf_t_StarReward(const pb::StarReward::t_StarReward *_starReward) : starReward(_starReward) {};
            Conf_t_StarReward() : starReward(NULL) {};
            bool init();
            static DWORD getConf(const DWORD step,const DWORD level);
        public:
            const pb::StarReward::t_StarReward *starReward;
            static std::map<DWORD,std::vector<pb::ThreeArgPara> > levelMap;
            static DWORD maxLevel;
    };

    //商店
    class Conf_t_Shop
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> priceMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::map<DWORD,DWORD>& getPriceMap() const
            {
                return priceMap;
            }
        public:
            Conf_t_Shop(const pb::Shop::t_Shop *_shop) : shop(_shop) {};
            Conf_t_Shop() : shop(NULL) {};
            bool init();
        public:
            const pb::Shop::t_Shop *shop;
            static std::map<DWORD,QWORD> buildMap;
    };

    //寿司关卡奖励
    class Conf_t_SushiLevel
    {
        private:
            QWORD key;
            std::vector<pb::ThreeArgPara> specRewardVec;
            DWORD weight;
            std::map<DWORD,DWORD> rewardMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::map<DWORD,DWORD>& getRewardMap() const
            {
                return rewardMap;
            }
            void randSpecialReward(std::map<DWORD,DWORD> &result) const;
        public:
            Conf_t_SushiLevel(const pb::SushiLevel::t_SushiLevel *_sushiReward) : sushiReward(_sushiReward) {};
            Conf_t_SushiLevel() : sushiReward(NULL) {};
            bool init();
            static DWORD getConf(const DWORD step,const DWORD level);
        public:
            const pb::SushiLevel::t_SushiLevel *sushiReward;
            static DWORD maxLevel;
            static std::map<DWORD,std::vector<pb::ThreeArgPara> > levelMap;
    };

    //活动扭蛋
    class Conf_t_ActiveToy
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            //随机某个扭蛋
            static DWORD randToyKey(const DWORD type);
            static DWORD randToKey_bonus(const DWORD type);
        public:
            Conf_t_ActiveToy(const pb::ActiveToy::t_ActiveToy *_activeToy) : activeToy(_activeToy) {};
            Conf_t_ActiveToy() : activeToy(NULL) {};
            bool init();
        public:
            const pb::ActiveToy::t_ActiveToy *activeToy;
            static std::map<DWORD,std::map<DWORD,DWORD> > idToWeightMap;
            static std::map<DWORD,DWORD> sumWeightMap;
            static std::map<DWORD,std::vector<DWORD> > bonusMap;
    };

    //活动
    class Conf_t_Active
    {
        private:
            QWORD key;
            DWORD beginTime;
            DWORD endTime;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline DWORD getBeginTime() const
            {
                return beginTime;
            }
            inline DWORD getEndTime() const
            {
                return endTime;
            }
        public:
            Conf_t_Active(const pb::Active::t_Active *_active) : active(_active) {};
            Conf_t_Active() : active(NULL) {};
            bool init();
        public:
            const pb::Active::t_Active * active;
    };

    //活动消耗
    class Conf_t_DrawCapsuleType
    {
        private:
            QWORD key;
            std::vector<DWORD> levelVec;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::vector<DWORD> &getLevelVec() const
            {
                return levelVec;
            }
        public:
            Conf_t_DrawCapsuleType(const pb::DrawCapsuleType::t_DrawCapsuleType *_consume) : consume(_consume) {};
            Conf_t_DrawCapsuleType() : consume(NULL) {};
            bool init();
        public:
            const pb::DrawCapsuleType::t_DrawCapsuleType* consume;
    };

    //合成建筑解锁单元格
    class Conf_t_Composite
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> materialMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::map<DWORD,DWORD> getMaterialMap() const
            {
                return materialMap;
            }
        public:
            Conf_t_Composite(const pb::Composite::t_Composite *_composite) : composite(_composite) {};
            Conf_t_Composite() : composite(NULL) {};
            bool init();
        public:
            const pb::Composite::t_Composite *composite;
    };
    //扩地表
    class Conf_t_Spread
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> materialMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::map<DWORD,DWORD> getMaterialMap() const
            {
                return materialMap;
            }
        public:
            Conf_t_Spread(const pb::Spread::t_Spread *_spread) : spread(_spread) {};
            Conf_t_Spread() : spread(NULL) {};
            bool init();
        public:
            const pb::Spread::t_Spread *spread;
    };
    //订货系统，格子开启
    class Conf_t_BagValet
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_BagValet(const pb::BagValet::t_BagValet *_BagValet) : BagValet(_BagValet) {};
            Conf_t_BagValet() : BagValet(NULL) {};
            bool init();
            static DWORD getMaxIdbyPopular(DWORD Popular);
        public:
            const pb::BagValet::t_BagValet *BagValet;
            static std::map<DWORD,DWORD,std::greater<DWORD> > m_mapPopular;
    };

    //订货系统，菜单选项
    class Conf_t_OrderGoods
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_OrderGoods(const pb::OrderGoods::t_OrderGoods *_OrderGoods) : OrderGoods(_OrderGoods) {};
            Conf_t_OrderGoods() : OrderGoods(NULL) {};
            bool init();
        public:
            const pb::OrderGoods::t_OrderGoods *OrderGoods;
    };
    //签到系统，月统计奖励
    class Conf_t_SignInEveryDay
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_SignInEveryDay(const pb::SignInEveryDay::t_SignInEveryDay *_SignInEveryDay) : SignInEveryDay(_SignInEveryDay) {};
            Conf_t_SignInEveryDay() : SignInEveryDay(NULL) {};
            bool init();
        public:
            const pb::SignInEveryDay::t_SignInEveryDay *SignInEveryDay;
            HelloKittyMsgData::vecAward awarditem;

    };

    //签到系统，月统计奖励
    class Conf_t_SignInTotalDay
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_SignInTotalDay(const pb::SignInTotalDay::t_SignInTotalDay *_SignInTotalDay) : SignInTotalDay(_SignInTotalDay) {};
            Conf_t_SignInTotalDay() : SignInTotalDay(NULL) {};
            bool init();
        public:
            const pb::SignInTotalDay::t_SignInTotalDay *SignInTotalDay;
            HelloKittyMsgData::vecAward awarditem;
    };
    class Conf_t_moneychange
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_moneychange(const pb::moneychange::t_moneychange *_moneychange) : moneychange(_moneychange) {};
            Conf_t_moneychange() : moneychange(NULL) {};
            bool init();
        public:
            const pb::moneychange::t_moneychange *moneychange;
    };

    //充值
    class Conf_t_TopupMall
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> priceMap;
            std::map<DWORD,DWORD> fristGetMap;
            std::map<DWORD,DWORD> baseMap;
            std::map<DWORD,DWORD> rewardMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD>& getPriceMap() const
            {
                return priceMap;
            }
            inline const std::map<DWORD,DWORD>& getFirstMap() const
            {
                return fristGetMap;
            }
            inline const std::map<DWORD,DWORD>& getMap() const
            {
                return baseMap;
            }
            inline const std::map<DWORD,DWORD>& getRewardMap() const
            {
                return rewardMap;
            }
        public:
            Conf_t_TopupMall(const pb::TopupMall::t_TopupMall *_mall) : mall(_mall) {};
            Conf_t_TopupMall() : mall(NULL) {};
            bool init();
        public:
            const pb::TopupMall::t_TopupMall *mall;
    };


    //拍卖场
    class Conf_t_AuctionCenter
    {
        private:
            QWORD key;
            DWORD beginTime;
            DWORD endTime;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline DWORD getBeginTime() const
            {
                return beginTime;
            }
            inline DWORD getEndTime() const
            {
                return endTime;
            }
        public:
            Conf_t_AuctionCenter(const pb::AuctionCenter::t_AuctionCenter *_auctionCenter) : auctionCenter(_auctionCenter) {};
            Conf_t_AuctionCenter() : auctionCenter(NULL) {};
            bool init();
        public:
            const pb::AuctionCenter::t_AuctionCenter * auctionCenter;
    };
    //火车数量
    class Conf_t_trainnumber
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_trainnumber(const pb::trainnumber::t_trainnumber *_trainnumber) : trainnumber(_trainnumber) {};
            Conf_t_trainnumber() : trainnumber(NULL) {};
            bool init();
            static QWORD getTrainNumID(DWORD playerlevel,DWORD trainno);
            DWORD getRandTrainNum() const;

        public:
            const pb::trainnumber::t_trainnumber * trainnumber;
            static std::map<DWORD,std::map<DWORD,QWORD>,std::greater<DWORD> > map_trainnumber;
            std::map<DWORD,DWORD> m_maprate;
    };

    //火车订单需求数量
    class Conf_t_trainloadnum
    {
        private:
            QWORD key;
            static std::map<DWORD,std::set<DWORD> > s_levelMap;
            static std::set<DWORD> s_levelSet;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            static DWORD getLevel(const DWORD level);
        public:
            Conf_t_trainloadnum(const pb::trainloadnum::t_trainloadnum *_trainloadnum) : trainloadnum(_trainloadnum) {};
            Conf_t_trainloadnum() : trainloadnum(NULL) {};
            bool init();
            DWORD getloadnum() const;
        public:
            const pb::trainloadnum::t_trainloadnum * trainloadnum;
            std::vector<DWORD> m_vecrate;
    };
    //火车装载奖励
    class Conf_t_allorderaward
    {
        private:
            QWORD key;
        public:
            enum awardtype
            {
                e_Orderaward = 1,//订单
                e_Trainaward = 2,//火车
            };
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_allorderaward(const pb::allorderaward::t_allorderaward *_allorderaward) : allorderaward(_allorderaward) {};
            Conf_t_allorderaward() : allorderaward(NULL) {};
            bool init();

        public:
            const pb::allorderaward::t_allorderaward * allorderaward;
    };
    //火车数量
    class Conf_t_trainreturnaward
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_trainreturnaward(const pb::trainreturnaward::t_trainreturnaward *_trainreturnaward) : trainreturnaward(_trainreturnaward) {};
            Conf_t_trainreturnaward() : trainreturnaward(NULL) {};
            bool init();
            static QWORD getTrainawardID(DWORD playerlevel,DWORD trainno);
            DWORD getRandAwardNum() const;

        public:
            const pb::trainreturnaward::t_trainreturnaward * trainreturnaward;
            static std::map<DWORD,std::map<DWORD,std::map<QWORD,DWORD> >,std::greater<DWORD> > map_trainreturnaward;
            std::vector<DWORD> m_vecRate;
    };
    //秒cd
    class Conf_t_Gold
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_Gold(const pb::Gold::t_Gold *_Gold) : Gold(_Gold) {};
            Conf_t_Gold() : Gold(NULL) {};
            bool init();
            static DWORD getClearCDCost(DWORD type,DWORD timer);
            typedef std::map<DWORD,float, std::greater<DWORD> > MapTimerDes;
        public:
            const pb::Gold::t_Gold * Gold;
            static std::map<DWORD,MapTimerDes> map_Gold;
    };

    //建筑升级
    class Conf_t_BuildUpGrade
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> materialMap;
            std::map<DWORD,DWORD> effectMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::map<DWORD,DWORD>& getMaterialMap() const
            {
                return materialMap;
            }
            const std::map<DWORD,DWORD>& getEffectMap() const
            {
                return effectMap;
            }
        public:
            Conf_t_BuildUpGrade(const pb::BuildUpGrade::t_BuildUpGrade *buildUpGrade) : upgrade(buildUpGrade) {};
            Conf_t_BuildUpGrade() : upgrade(NULL) {};
            bool init();
        public:
            const pb::BuildUpGrade::t_BuildUpGrade *upgrade;
    };

    //建筑效果
    class Conf_t_BuildEffect
    {
        private:
            QWORD key;
            std::vector<pb::ThreeArgPara> effectVec;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::vector<pb::ThreeArgPara>& getEffectVec() const
            {
                return effectVec;
            }
        public:
            Conf_t_BuildEffect(const pb::BuildEffect::t_BuildEffect *effect) : effect(effect) {};
            Conf_t_BuildEffect() : effect(NULL) {};
            bool init();
        public:
            const pb::BuildEffect::t_BuildEffect *effect;
    };

    //图标
    class Conf_t_openfun
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            enum eOpenSource
            {
                eOpen_Level =0,
                eOpen_Build =1,
            };
            enum FunID
            {
                FunMarket = 6,

            };
        public:
            Conf_t_openfun(const pb::openfun::t_openfun *openfun) : openfun(openfun) {};
            Conf_t_openfun() : openfun(NULL) {};
            static bool getNewIconByType(eOpenSource type,DWORD param,std::set<DWORD>& OldSet, std::set<DWORD>& newSet);

            bool init();
        public:
            const pb::openfun::t_openfun *openfun;
            static std::map<eOpenSource,std::map<DWORD,std::set<DWORD> > >m_mapOpen;

    };
    //黑市
    class Conf_t_market
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_market(const pb::market::t_market *market) : market(market) {};
            Conf_t_market() : market(NULL) {};
            DWORD getnum() const;
            DWORD getpricerate() const;
            bool init();
        public:
            const pb::market::t_market *market;
            std::map<DWORD,DWORD> m_mapNumRate;
            std::map<DWORD,DWORD> m_mapPriceRate;


    };
    //男仆

    class Conf_t_Manservant
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_Manservant(const pb::Manservant::t_Manservant *manservant) : manservant(manservant) {};
            Conf_t_Manservant() : manservant(NULL) {};
            DWORD getBoxNum() const;
            DWORD getBoxOpenGetNum() const;
            bool init();
        public:
            const pb::Manservant::t_Manservant *manservant;
            std::map<DWORD,DWORD> m_mapBoxRate;
            std::map<DWORD,DWORD> m_mapBoxOpenRate;



    };
    //男仆眼镜
    class Conf_t_Classes
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_Classes(const pb::Classes::t_Classes *classes) : classes(classes) {};
            Conf_t_Classes() : classes(NULL) {};
            bool init();
        public:
            const pb::Classes::t_Classes *classes;



    };

    //寿司花费表
    class Conf_t_SushiSpend
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> priceMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD>& getPriceMap() const
            {
                return priceMap;
            }
        public:
            Conf_t_SushiSpend(const pb::SushiSpend::t_SushiSpend *_spend) : spend(_spend) {};
            Conf_t_SushiSpend() : spend(NULL) {};
            bool init();
        public:
            const pb::SushiSpend::t_SushiSpend *spend;
    };

    //寿司排行榜奖励
    class Conf_t_SushiRankReward
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> rewardMap;
            std::vector<DWORD> rankVec;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD>& getRewardMap() const
            {
                return rewardMap;
            }
            inline const std::vector<DWORD>& getRankVec() const
            {
                return rankVec;
            }
        public:
            Conf_t_SushiRankReward(const pb::SushiRankReward::t_SushiRankReward *_reward) : reward(_reward) {};
            Conf_t_SushiRankReward() : reward(NULL) {};
            bool init();
            static QWORD getKeyByRank(const DWORD rank);
        public:
            const pb::SushiRankReward::t_SushiRankReward *reward;
            static std::map<DWORD,QWORD> rankKeyMap;
    };
    //合建建筑
    class Conf_t_UniteBuild
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_UniteBuild(const pb::UniteBuild::t_UniteBuild *_UniteBuild) : UniteBuild(_UniteBuild) {};
            Conf_t_UniteBuild() : UniteBuild(NULL) {};
            bool init();
        public:
            const pb::UniteBuild::t_UniteBuild *UniteBuild;
    };
    //合建建筑格子
    class Conf_t_UniteGrid
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_UniteGrid(const pb::UniteGrid::t_UniteGrid *_UniteGrid) : UniteGrid(_UniteGrid) {};
            Conf_t_UniteGrid() : UniteGrid(NULL) {};
            bool init();

        public:
            const pb::UniteGrid::t_UniteGrid *UniteGrid; 
            std::set<DWORD> setBuildtype;
            std::set<DWORD> setPricetype;
    };
    //合建建筑初始化
    class Conf_t_UniteGridInit
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_UniteGridInit(const pb::UniteGridInit::t_UniteGridInit *_UniteGridInit) : UniteGridInit(_UniteGridInit) {};
            Conf_t_UniteGridInit() : UniteGridInit(NULL) {};
            bool init();
        public:
            const pb::UniteGridInit::t_UniteGridInit *UniteGridInit; 
            std::set<DWORD> setOtherColId;
            std::set<DWORD> setOpenGridId;
    };
//合建建筑等级
    class Conf_t_UniteBuildlevel
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_UniteBuildlevel(const pb::UniteBuildlevel::t_UniteBuildlevel *_UniteBuildlevel) : UniteBuildlevel(_UniteBuildlevel) {};
            Conf_t_UniteBuildlevel() : UniteBuildlevel(NULL) {};
            bool init();
        public:
            const pb::UniteBuildlevel::t_UniteBuildlevel *UniteBuildlevel;
            std::map<DWORD,DWORD> materialMap;
    };

    //虚拟礼品商城
    class Conf_t_VirtualGiftShop
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> priceMap;
            std::map<DWORD,DWORD> senderProfitMap;
            std::map<DWORD,DWORD> accepterProfitMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::map<DWORD,DWORD> getPriceMap() const
            {
                return priceMap;
            }
            const std::map<DWORD,DWORD> getSenderProfitMap() const
            {
                return senderProfitMap;
            }
            const std::map<DWORD,DWORD> getAccepterProfitMap() const
            {
                return accepterProfitMap;
            }
        public:
            Conf_t_VirtualGiftShop(const pb::VirtualGiftShop::t_VirtualGiftShop *_virtualShop) : virtualShop(_virtualShop) {};
            Conf_t_VirtualGiftShop() : virtualShop(NULL) {};
            bool init();
        public:
            const pb::VirtualGiftShop::t_VirtualGiftShop *virtualShop;
    };

    class Conf_t_StarSpend
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> singleMap;
            std::map<DWORD,DWORD> operatorMap;
            std::map<DWORD,DWORD> sportsMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            const std::map<DWORD,DWORD>& getSingleMap() const
            {
                return singleMap;
            }
            const std::map<DWORD,DWORD>& getOperatorMap() const
            {
                return operatorMap;
            }
            const std::map<DWORD,DWORD>& getSportsMap() const
            {
                return sportsMap;
            }
        public:
            Conf_t_StarSpend(const pb::StarSpend::t_StarSpend *_spend) : spend(_spend) {};
            Conf_t_StarSpend() : spend(NULL) {};
            bool init();
        public:
            const pb::StarSpend::t_StarSpend *spend;
    };

    
    class Conf_t_ConstellationStar
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_ConstellationStar(const pb::ConstellationStar::t_ConstellationStar *_star) : star(_star) {};
            Conf_t_ConstellationStar() : star(NULL) {};
            bool init();
        public:
            const pb::ConstellationStar::t_ConstellationStar *star;
    };

    //玩家身高
    class Conf_t_RoleHeight
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_RoleHeight(const pb::RoleHeight::t_RoleHeight *_height) : height(_height) {};
            Conf_t_RoleHeight() : height(NULL) {};
            bool init();
        public:
            const pb::RoleHeight::t_RoleHeight *height;
    };

    //收入
    class Conf_t_RoleIncome
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_RoleIncome(const pb::RoleIncome::t_RoleIncome *_inCome) : inCome(_inCome) {};
            Conf_t_RoleIncome() : inCome(NULL) {};
            bool init();
        public:
            const pb::RoleIncome::t_RoleIncome *inCome;
    };

    //婚姻
    class Conf_t_RoleMaritalStatus
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_RoleMaritalStatus(const pb::RoleMaritalStatus::t_RoleMaritalStatus *_marital) : marital(_marital) {};
            Conf_t_RoleMaritalStatus() : marital(NULL) {};
            bool init();
        public:
            const pb::RoleMaritalStatus::t_RoleMaritalStatus *marital;
    };

    //体重
    class Conf_t_RoleWeight
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_RoleWeight(const pb::RoleWeight::t_RoleWeight *_weight) : weight(_weight) {};
            Conf_t_RoleWeight() : weight(NULL) {};
            bool init();
        public:
            const pb::RoleWeight::t_RoleWeight *weight;
    };

    //年龄
    class Conf_t_RoleAge
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_RoleAge(const pb::RoleAge::t_RoleAge *_age) : age(_age) {};
            Conf_t_RoleAge() : age(NULL) {};
            bool init();
        public:
            const pb::RoleAge::t_RoleAge *age;
    };

    //性别
    class Conf_t_RoleSex
    {
        private:
            QWORD key;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
        public:
            Conf_t_RoleSex(const pb::RoleSex::t_RoleSex*_sex) : sex(_sex) {};
            Conf_t_RoleSex() : sex(NULL) {};
            bool init();
        public:
            const pb::RoleSex::t_RoleSex *sex;
    };

    //男女仆盒子
    class Conf_t_ManservantBox
    {
        private:
            QWORD key;
            std::map<DWORD,DWORD> m_priceMap;
            DWORD m_boxItemWeight;
            std::vector<pb::TwoArgPara> m_boxItemVec;
            std::set<DWORD> m_rewardSet;
            std::set<DWORD> m_levelSet;
            std::map<DWORD,std::set<DWORD> >m_levelMap;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::map<DWORD,DWORD> &getPrice() const
            {
                return m_priceMap;
            }
        public:
            Conf_t_ManservantBox(const pb::ManservantBox::t_ManservantBox *manservant) : manservantBox(manservant) {};
            Conf_t_ManservantBox() : manservantBox(NULL) {};
            bool init();
            bool randItemMap(std::map<DWORD,DWORD> &itemMap);
            DWORD randItemNum();
        public:
            const pb::ManservantBox::t_ManservantBox *manservantBox;
            static DWORD randBox();
        private:
            static DWORD m_boxTypeWeight;
            static std::vector<pb::TwoArgPara> m_boxTypeVec; 
            static std::map<DWORD,std::set<DWORD> > m_typeMap;

    };

    //npc摊位信心
    class Conf_t_NpcFriends
    {
        private:
            QWORD key;
            std::set<DWORD> m_rewardSet;
            std::set<DWORD> m_levelSet;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::set<DWORD>& getRewardSet() const
            {
                return m_rewardSet;
            }
            inline const std::set<DWORD>& getLevelSet() const
            {
                return m_levelSet;
            }
        public:
            Conf_t_NpcFriends(const pb::NpcFriends::t_NpcFriends *_npc) : npc(_npc) {};
            Conf_t_NpcFriends() : npc(NULL) {};
            bool init();
        public:
            const pb::NpcFriends::t_NpcFriends *npc;
    };

    //订单措施
    class Conf_t_OrderFacilities
    {
        private:
            QWORD key;
            std::set<DWORD> m_excptSet;
            static std::vector<QWORD> s_buildIDVec;
        public:
            inline QWORD getKey() const
            {
                return key;
            }
            inline const std::set<DWORD>& getExcptSet() const
            {
                return m_excptSet;
            }
            static const std::vector<QWORD>& getBuildIDVec()
            {
                return s_buildIDVec;
            }
        public:
            Conf_t_OrderFacilities(const pb::OrderFacilities::t_OrderFacilities *_facility) : facility(_facility) {};
            Conf_t_OrderFacilities() : facility(NULL) {};
            bool init();
        public:
            const pb::OrderFacilities::t_OrderFacilities *facility;
    };

}


#endif



