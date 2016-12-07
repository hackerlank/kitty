/**
 * \file	SceneUser.h
 * \version  	$Id: SceneUser.h 67 2013-04-23 09:44:20Z  $
 * \author  	,
 * \date 	2013年04月07日 15时48分31秒 CST
 * \brief 	场景用户定义
 *
 * 
 */

#ifndef _SCENEUSER_H_
#define _SCENEUSER_H_

#include <map>
#include "Fir.h"
#include "SceneTask.h"
#include "CharBase.h"
#include "zTime.h"
#include "RecordCommand.h"
#include "zSerialize.h"
#include "zCmdBuffer.h"
#include "common.h"
#include "UTrade.h"
#include "UStoreHouse.h"
#include "buildManager.h"
#include "common.pb.h"
#include "SceneMapData.h"
#include "taskManager.h"
#include "atlasManager.h"
#include "achievementManager.h"
#include "friend.pb.h" 
#include "EventManger.h"
#include "gardenactivity.h"
#include "FriendManager.h"
#include "emailManager.h"
#include "dressManager.h"
#include "paperManager.h"
#include "dataManager.h"
#include "carnival.pb.h"
#include "burstEventManager.h"
#include "OrderManager.h"
#include "auction.pb.h"
#include "zMemDB.h"
#include "giftPackage.h"
#include "LeaveMessageManager.h"
#include "addressManager.h"
#include "rank.pb.h"
#include "zMemDBPool.h"
#include "toy.pb.h"
#include "room.pb.h"
#include "TrainOrderManager.h"
#include "OrdersystemManager.h"
#include "SignInManager.h"
#include "MarketManager.h"
#include "UnityBuildManager.h"
#include "ResourceCommand.h"
#include "PlayerActiveManager.h"
//愉悦值的3个档次
const DWORD HAPPY_LOW = 60;
const DWORD HAPPY_MID = 80;
const DWORD HAPPY_HIGHT = 100;
//占卜开始和结束
const DWORD DIVINE_BEGIN = 0;
const DWORD DIVINE_END = 3;

class RecordClient;


enum SYSTEMID
{
    System_Build = 1, //建造系统
    System_Composite = 2, //合成系统
    System_Produce = 3, //生产系统
};

enum ITEMRESORETYPE
{
    Item_SysOrder = 0,  //采购系统
    Item_Produce = 1 ,  //生产
    Item_Composite = 2,  //合成
};

enum ResourceType
{
    Picture_Head = 0, //头像
    Picture_Wall = 1, //照片墙
};

struct VistorRecord
{
    QWORD charID;
    DWORD time;
    DWORD status;
    VistorRecord()
    {
        bzero(this,sizeof(*this));
    }

    bool operator < (const VistorRecord &vistorRecord) const
    {
        if(time == vistorRecord.time)
        {
            return charID < vistorRecord.charID;
        }
        return time < vistorRecord.time;
    }
};





class SceneUser 
{
    public:
        friend class SceneUserManager;
        //accid(暂时没用)		
        DWORD accid;
        //角色id，全服唯一
        QWORD charid;
        //昵称
        //char nickname[MAX_NAMESIZE+1];
        //charbase,不要在这儿添加字段，很不方便,要改数据库
        CharBase charbase;
        //charbin,后面角色需要存档的零散数据，放这里
        HelloKittyMsgData::CharBin charbin;
        // 贸易
        UTrade m_trade; 
        //仓库
        UStoreHouse m_store_house;
        //建筑管理器
        BuildManager m_buildManager;
        //任务管理器
        TaskManager m_taskManager;
        //图鉴管理
        AtlasManager m_atlasManager;
        //成就管理器
        AchievementManager m_achievementManager;
        //邮件管理器
        EmailManager m_emailManager;
        //时装管理器
        DressManager m_dressManager;
        //图纸管理器
        PaperManager m_paperManager;
        //突发事件管理器
        BurstEventManager m_burstEventManager;
        //礼品包裹
        GiftPackage m_giftPackage;
        //地址管理器
        AddressManager m_addressManager;
        //垃圾产生器
        ActiveManager m_activeManger;
        //事件产生器
        EventManager m_eventmanager;
        //好友
        ManagerFriend m_friend;
        //存档md5	
        char lastSaveMD5[16];
        //kitty乐园
        SceneMapData m_kittyGarden;
        //订单
        OrderManager m_orderManager;
        //留言
        LeaveMessageManager m_leavemessage;
        //个人资料基本信息
        HelloKittyMsgData::PersonalInfo m_personInfo;
        //来访记录 
        std::set<VistorRecord> m_vistorRecordSet; 
        //火车订单
        TrainOrderManager m_managertrain;
        //订货系统
        ordersystemManager m_managerordersystem;
        //签到
        SignInManager m_managersignin;
        //黑市和男仆
        marketManager m_market;
        //合建个人信息
        UnityManager m_unitybuild;
        //组ID
        QWORD m_groupID;
        //活动存档
        PlayerActiveManager m_active;
    public:
        SceneUser(SceneTask *gate,QWORD setCharid);
        virtual ~SceneUser();
        //发送数据到客户端
        bool sendCmdToMe(const void *pstrCmd, const DWORD nCmdLen) const;
        //发送数据到网关
        bool sendCmdToGateway(const void *pstrCmd, const DWORD nCmdLen) const;
        //是否在线标志
        inline bool is_online()
        {
            return _online;
        }
        //获得仓库当前最大容量
        DWORD getStoreLimit();
        //轮询
        bool loop();

        //上线函数
        bool online(std::string phone_uuid,SceneTask* _gate,const bool reconnect = false);
        bool offlineload();
        //从session注册到场景函数
        bool reg(CMD::SCENE::t_regUser_Gatescene *Cmd);
        //注销
        bool unreg();
        // 从charbase和二进制数据克隆一个玩家
        bool clone(CharBase charbase,std::string allbinary);


        //压缩角色数据
        DWORD saveBinaryArchive(unsigned char *out , const int maxsize);
        //保存角色数据
        bool save();

        //检查等级
        bool checkLevel(const DWORD num);
        //操作错误代码返回
        bool opErrorReturn(const HelloKittyMsgData::ErrorCodeType &errorCode,const DWORD itemID = 0);
        //操作成功返回
        bool opSuccessReturn(const HelloKittyMsgData::SuccessCodeType &code);
        //刷新角色基本信息
        bool flushUserInfo();
        //刷新角色单个属性
        bool updateAttrVal(const HelloKittyMsgData::AttrType &attrType,const DWORD value);
        //填充刷新角色乐园所有信息
        //bool flushUserInfo(HelloKittyMsgData::AckFlushUserInfo &send);
        //升级
        bool upgrade();
        void AddVisit(QWORD PlayerID,DWORD GateId);//加入一个访问者
        void DelVisit(QWORD PlayerID);//删除一个访问者
        bool isVisit(QWORD PlayerID);
        QWORD getvisit();
        void setVisit(QWORD PlayerId);
        void BroadCastPersonNum();

        void UserReturn();//回到自己的家园
        const map<QWORD,DWORD>& getVist(){return mapVisit;}
        bool  havePersonOnline();//是否有玩家在线
        bool  haveVisit();
        DWORD getHappyFequence(const DWORD happyVal);
        void  DoBuildAward(const HelloKittyMsgData::vecAward& rvecAward,DWORD EventID,QWORD owerid,bool NeedNotice);//给玩家发奖励

        //刷新别人家乐园
        bool flushKittyGardenInfo();
        bool flushKittyGardenInfo(QWORD sendCharID,HelloKittyMsgData::AckEnterGarden &send);
        //gm修改愉悦值天数
        bool changeHappyDataGm(const DWORD happyValType,const DWORD day);
        //判断是否需要刷新日常数据
        bool isDailyData();
        //刷新日常数据
        bool brushDailyData();
        //初始化日常数据
        void initDailyData();
        //增加访问别人次数
        bool addVertiseOther(const DWORD num);
        //增加好友
        bool addFriendNum(const DWORD num);
        //开启嘉年华
        bool openCarnival();
        //重置玩家所有数据
        void resetAllData();
        //更改系统时间刷新有关时间的数据
        bool changeTime();
        //建筑操作
        void opBuild(QWORD PlayerID,const HelloKittyMsgData::Builditype &rBuild);
        bool  checkPush(const HelloKittyMsgData::vecAward& rvecAward);
        bool  pushItem(const HelloKittyMsgData::vecAward& rvecAward,const char *reMark);
        bool  pushItemWithoutCheck(const HelloKittyMsgData::vecAward& rvecAward,const char *reMark);

        //扣除道具价格
        bool deductPrice(const CMD::SCENE::t_UserPurchasePrice *cmd);
        //购买道具，增加道具
        bool purchaseAddItem(const CMD::SCENE::t_UserPurchaseShiftItem *cmd);
        //向在自己乐园家发送消息
        bool sendOtherUserMsg(const void *pstrCmd, const DWORD nCmdLen);
        //发送时间
        void sendhappenevent(const HelloKittyMsgData::PlayerEvent &event);
        void broadcastMsgInMap(const void *pstrCmd, const int nCmdLen,bool bexpVisit = false); // bexpVisit 为true，若玩家访问其他人，那么他不发消息
        void returnMsgInMap(QWORD PlayerID,const void *pstrCmd, const int nCmdLen);
        //给玩家发操作建筑奖励
        void DoAward(QWORD PlayerID, const HelloKittyMsgData::vecAward &vecAward,DWORD EventID = 0,bool NeedNotice = false);
        void SendMsgToOher(QWORD PlayerID,const void *pstrCmd, const DWORD nCmdLen); 
        bool isOtherOnline(QWORD PlayerID);
        ManagerFriend & getFriendManager(){return m_friend;}
        RecordClient  *getSelfRecord();
        //检查材料
        bool checkMaterialMap(const std::map<DWORD,DWORD> &materialMap,const bool notify = false);
        //扣除材料
        bool reduceMaterialMap(const std::map<DWORD,DWORD> &materialMap,const char *reMark);
        //更新全局buffer
        bool updateBufferMsg();
        const std::map<QWORD,std::map<DWORD,Buffer>>& getBufferMap();
        //初始化全局buffer
        void initBuffer();
        //上线辅助函数
        void onlineInit(const bool reconnect,SceneTask* _gate);
        //获得gate
        SceneTask* getGate() const ; 
        //刷新占卜信息
        bool flushDivine(const HelloKittyMsgData::DivineData *divine = NULL);
        //占卜第一个提示
        bool divineNotice();
        //占卜
        bool divine(const bool notify);
        //验证占卜顺序
        bool divineVerify(const std::string &answer);
        //购买嘉年华宝盒
        bool buyCarnivalBox();
        //刷出嘉年华宝盒内容
        bool randCarnivalShop();
        //购买商城道具
        bool purchaseItem(const HelloKittyMsgData::ReqPurchaseItem *cmd);
        //更改愉悦值数据
        void changeHappyData();
        //计算所有道具价格
        static DWORD countPrice(const std::map<DWORD,DWORD> &materialMap);

        
        //星座
        bool beginStar(const DWORD step = 1);
        bool reqCommitStar(const HelloKittyMsgData::ReqStarCommitStep *cmd);
        bool flushStar();
        HelloKittyMsgData::StarData* getStarDataByType(const HelloKittyMsgData::StarType &starType);
        bool startGame(const HelloKittyMsgData::ReqStartGame *reqStartGame);
        bool ackStartGame(HelloKittyMsgData::AckStartGame *message);
        bool ackJoinStartGame(const HelloKittyMsgData::ReqJoinStartGame *message);
        void adjustGroupID();
        bool getGroupInfo(HelloKittyMsgData::StarGroupInfo &groupInfo);
        bool setGroupInfo(HelloKittyMsgData::StarGroupInfo &groupInfo);
        void sendGroupMsg(const HelloKittyMsgData::StarGroupInfo &groupInfo,const std::string &msg,const QWORD charID = 0);
        bool gameOver(const DWORD step,const bool isPass);
        void resetStar(HelloKittyMsgData::StarGroupInfo &groupInfo);
        bool starEnd();
        bool rankHardConfig();
        void randHard(HelloKittyMsgData::AckBeginStar &askMsg,HelloKittyMsgData::StarGroupInfo &groupInfo);
        bool starReward(std::map<DWORD,DWORD> &randReward,const DWORD step,const HelloKittyMsgData::StarGroupInfo &groupInfo);
        bool cancelStarGame(const DWORD reason = HelloKittyMsgData::CR_Client_Quit);


        DWORD addGiftCD(const QWORD accepter);
        DWORD getContinueGift(const QWORD accepter);

        //加入或者退出游艺中心
        bool opRecreationCenter(const bool opAdd = true);
        //加入或者退出拍卖房间
        bool opAuctionRoom(const DWORD auctionID,const bool opAdd = true);
        //竞标
        bool auction(const DWORD auctionID,const bool autoFlg = false);
        //设置自动竞标
        bool setAutoAuction(const DWORD auctionID,const DWORD cnt);
        //广播竞拍历史数据
        static bool sendHistoryBroadcast();
        //获得竞拍历史数据
        static bool getAuctionHistory(HelloKittyMsgData::AckAuctionHistory &historyData);
        //拍卖结束通知
        static bool sendEndBroadcast(zMemDB* handle,const DWORD auctionID);
        //单发房间简介
        bool sendAuctionBriefSingle();
        //广播房间简介
        static bool sendAuctionBriefBroadCast();
        //log拍卖信息
        bool logAuctionInfo(const DWORD auctionID);
        //角色类型为0
        inline bool isTypeBuild(const DWORD type) const
        {
            return type == 0;
        }
        //点击进入常规礼品店
        bool clickEnterNormalGift();
        //购买常规礼品
        bool parchaseGift(const DWORD gitfID);
        //开始摇杆
        bool beginSlotMachine(const HelloKittyMsgData::ReqBeginSlot *cmd);
        //万家乐游戏
        bool beginMacro(const HelloKittyMsgData::ReqBeginMacro *cmd);
        //开始做寿司
        bool beginSuShi();
        //提交每个关卡的数据
        bool reqCommitSuShi(const HelloKittyMsgData::ReqCommitStep *cmd);
        //刷新寿司数据
        bool flushSuShi();
        //请求等级排名
        bool reqRank(const HelloKittyMsgData::ReqRank *cmd);
        //打印全球排行榜等级日志
        bool logLevelRank();
        //打印所有的区域等级排行榜
        bool logAllAreaLevelRank();
        //打印某个的区域等级排行榜
        bool logAreaLevelRank(const HelloKittyMsgData::AreaType &areaType);
        //判断是否已解锁
        bool isUnLock(const DWORD buildType);
        //解锁建筑
        bool unLock(const DWORD buildType);
        //给道具(包括建筑)
        bool addItempOrEmail(const std::map<DWORD,DWORD> &itemMap,const char *mark);
        bool addItempOrEmail(const DWORD itemID,const DWORD num,const char *mark);
        //参加扭蛋活动
        bool randActiveToy(const HelloKittyMsgData::ReqOpToy *cmd);
        //是否能够参加活动
        bool canJoinActive(const DWORD activeID);
        //请求活动信息
        bool reqActiveInfo(const DWORD activeID);
        //获得玩家显示结构
        static bool getplayershowbase(QWORD playerId,HelloKittyMsgData::playerShowbase &base);
        //上线处理嘉年华宝箱
        void onLineCarnivalBox();
        //通知主人，客人来料
        bool notifyGarden(const QWORD charID);
        //通知客人，主人上线了
        bool notifyVistor();
        //更改个人资料
        bool modifyPresent(const std::string &present);
        bool modifyVoice(const HelloKittyMsgData::ReqModifyVoice *message);
        bool ackPersonalInfo(const QWORD charID,const bool isView = false);
        void opLike(const QWORD charID,const bool addFlg);
        bool isLike(const QWORD charID);
        DWORD getLikeCnt();
        bool viewWechat(const QWORD charID);
        bool adjusetGrid();

        //排行榜
        bool charismaRank(const HelloKittyMsgData::ReqRank *reqRank);
        bool contributeRank(const HelloKittyMsgData::ReqRank *reqRank);
        bool starRank();
        bool suShiRank();
        bool levelRank(const HelloKittyMsgData::ReqRank *reqRank);
        bool unitybuildRank(const HelloKittyMsgData::ReqRank *reqRank);
        bool getHotSet(std::map<QWORD,DWORD> &rankHotSet);
        bool getVerifySet(std::map<QWORD,DWORD> &rankVerifySet);
        std::string getTimeRankKey(const HelloKittyMsgData::TimeRankType &timeRank);
        std::string getSexRankKey(const HelloKittyMsgData::SexRankType &sexRank);
        static bool ackNpcRoomAndPerson(const QWORD npcID,HelloKittyMsgData::AckRoomAndPersonalInfo &ackMsg);
        //进入空间
        bool ackRoom(const QWORD charID);
        //霓虹广场
        bool ackNoCenter(const DWORD typeID);
        //封号
        bool forBid(const DWORD time,const char *reason);
        //禁言
        bool forBidSys(const DWORD time,const char *reason);
        //是否被禁言
        bool isForbidSys();
        //刷新角色信息
        void ackUserData();
        //调整角色等级
        bool adjustLevel();
        //记录升级日志
        void infoLevel(const DWORD oldExp,const DWORD oldLevel);
        //计算购买cd需要花费的钱
        DWORD countParseCD(const DWORD lastSec);
        //update拍卖广场状态
        bool updateCenterStatus();
        //update兑换礼品数量
        bool updateExchangeNum(const DWORD id = 0);
        bool inGuide();
        //重连刷新函数
        void reconnect();
    public:
        //个人资料块
        bool modifyPersonalInfo(const HelloKittyMsgData::ReqEditPersonInalInfo *message);
        void updatePresent();
        void updateVoice();
        void updatePhoto();
        bool fillRoomMsg(HelloKittyMsgData::RoomInfo *roomInfo);
        void fillVerifyLevel(HelloKittyMsgData::AckNoCenter &ack);
        void fillHot(HelloKittyMsgData::AckNoCenter &ack);
        bool ackRoomAndPerson(const QWORD charID);
        void fillOtherInfo(HelloKittyMsgData::OtherInfo *other,const QWORD charID);
        bool addVistor(const QWORD charID);
        bool reqNeonMark(const HelloKittyMsgData::ReqNeonMark *mark);
        void ackPicture();
        bool addPicture(const HelloKittyMsgData::ReqAddPicture *message);
        bool movePicture(const HelloKittyMsgData::ReqMovePicture *message);
        bool setPictureHead(const HelloKittyMsgData::ReqSetPictureHead *message);
        void ackOtherInfo();
        void sendRoomInfo(const void *pstrCmd, const DWORD nCmdLen);
        bool outRoom(const QWORD charID);
        bool buyLoginLast();
        void loadViewList(const HelloKittyMsgData::Serialize& binary);
        void saveViewList(HelloKittyMsgData::Serialize& binary);

        bool loadActiveCode(const HelloKittyMsgData::Serialize& binary);
        bool saveActiveCode(HelloKittyMsgData::Serialize& binary);




        bool resPicture(const CMD::RES::t_RspAddRes *cmd);
        bool resHead(const CMD::RES::t_RspAddRes *cmd);

        //加载贡献值
        bool loadContribute(const HelloKittyMsgData::Serialize &binary);
        bool saveContribute(HelloKittyMsgData::Serialize &binary);
        void loadBuyWeChatList(const HelloKittyMsgData::Serialize& binary);
        void saveBuyWeChatList(HelloKittyMsgData::Serialize& binary);
        void addBuyWechat(const QWORD charID);
        void loadLikeList(const HelloKittyMsgData::Serialize& binary);
        void saveLikeList(HelloKittyMsgData::Serialize& binary);
        bool isBuyWeChat(const QWORD charID);

        bool loadActiveRecharge(const HelloKittyMsgData::Serialize &binary);
        bool saveActiveRecharge(HelloKittyMsgData::Serialize &binary);
        DWORD getActiveRecharge(const DWORD activeID);

        
#if 0
        //参加活动
        bool active(const pb::Conf_t_Active *active);
#endif
        //结束寿司
        bool endSuShi();
        //单发房间竞拍情况
        bool sendAuctionfSingle(const DWORD auctionID);
        //每个拍卖的具体信息
        bool auctionInfo(zMemDB* handle,const DWORD auctionID);
        //下线辅助函数
        void offline();
        //新建角色初始化信息
        bool initNewRole();

        // 获取该角色的二进制信息
        //std::string getAllBinary();
        // 获取该角色二进制数据
        std::string getBinaryArchive();
        //反序列化二进制数据
        void setupBinaryArchive(HelloKittyMsgData::Serialize& binary);
        //第一次访问家园
        void DoFirstVisit(QWORD PlayerID,DWORD GateId);
        //更改愉悦值数据
        void changeHappyData(HelloKittyMsgData::HappyData *happyData,const DWORD value,const DWORD judegeVal,const DWORD sec,const DWORD days = 1,const bool gmFlg = false);
        //检测开启嘉年华
        bool checkOpenCarnival();
        //进入乐园(进入别人家乐园或者返回自己家乐园)
        bool flushGardenMsg(const DWORD gatewayID,const QWORD sendCharID);
        //填充全局buffer
        bool fullUserInfoBuffer(HelloKittyMsgData::UserBaseInfo &userInfo);
        //占卜随机蜡烛顺序
        bool randDivineAnswer(std::string &ret,DWORD &firstKey);
        //占卜随机出运势
        const pb::Conf_t_Divine* randDivine(const DWORD answer);
        //刷出嘉年华宝盒
        bool randCarnivalBox(const bool good = false);
        //重置嘉年华数据
        void initCarnivalShopData();
        //嘉年华商店
        void loopCarnivalBox();
        //小游戏loop
        bool litterGameLoop();
        //单发竞拍历史数据
        bool sendHistorySingle();
        //加载解锁数据
        bool loadUnLockBuild(const HelloKittyMsgData::Serialize& binary);
        //保存解锁数据
        bool saveUnLockBuild(HelloKittyMsgData::Serialize& binary);
        //更新解锁列表
        bool updateUnLock(const DWORD typeID);
        //更改共享值
        void updateContibute(const QWORD charID);
        bool popularNowRank(const HelloKittyMsgData::ReqRank *reqRank);
    public:
        void recordstaticnpc(const DWORD NpcID, const std::string &NpcName,DWORD level);
        bool recordactivenpc(const DWORD NpcID, const std::string &NpcName);
        void sendStaticNpc(const DWORD NpcID);
        static bool getStaticNpc(const DWORD NpcID,HelloKittyMsgData::EnterGardenInfo &info);
        void recordPath(const std::string &filepath);

    public:
        //角色所带来的全局buffer
        std::map<QWORD,std::map<DWORD,Buffer>> m_bufferMap;
        //检测新功能:
        void checkFunOpen(pb::Conf_t_openfun::eOpenSource type,DWORD param);
        bool stopCd(const SYSTEMID &systemID);


        static bool suShiRankReward() ;
        static bool StarRankReward();

        bool clearWeekData();
        bool clearMonthData();
        //请周榜
        static bool clearWeekRank();
        static bool clearMonthRank();
        bool syncGame(const HelloKittyMsgData::ReqSyncStar *reqSyncGame);
        bool ackReadyGame();

        //操作贡献值
        void opContrubute(const QWORD charID,const DWORD val,const char *reMark,const bool addOp = true);
        DWORD getContribute(const QWORD charID);
        //送虚拟商店礼品
        bool sendVirtualGift(const HelloKittyMsgData::ReqSendVirtualGift *cmd);
        bool acceptVirtualGift(const QWORD sender,const HelloKittyMsgData::ReqSendVirtualGift *cmd);
        void opCharisma(const QWORD charID,const DWORD val,const char *reMark,const bool addOp);
        void triggerTaskGuid(DWORD type,DWORD param);//触发任务引导
        //充值人民币
        DWORD rechargeRMB(const DWORD rmb,const DWORD activeID);
        bool rewardActiveCode(const std::string &activeCode);
        void opItemResourMap(const ITEMRESORETYPE type,const DWORD itemID,const DWORD num,const bool opType);
        DWORD getItemResourNum(const ITEMRESORETYPE type,const DWORD itemID);
    private:
        bool clearWeekSuShiRank();
        bool clearWeekCharisRank();
        bool clearMonthCharisRank();
        bool clearWeekContributeRank();
        bool clearMonthContributeRank();
        bool clearWeekPopularNowRank();
        bool clearMonthPopularNowRank();
        bool clearWeekStarRank();

        void ackToyTime();
        void ackCoinToyDailyTime();
        void saveRecord(HelloKittyMsgData::Serialize &binary);
        void loadRecord(const HelloKittyMsgData::Serialize &binary);


    private:
        DWORD m_gateid;
        //玩家在线状态
        bool _online; 
        bool needSave;
        char lastSaveCharBaseMD5[16];
        map<QWORD,DWORD> mapVisit;//id,gate
        map<DWORD,QWORD> mapVisitTimer;
        QWORD VisitID;
        DWORD m_LastCheckTimer;
        //嘉年华商店数据(不用存档)
        HelloKittyMsgData::CarnivalShopData m_carnivalShopData;
        //是否正在参与自动竞拍
        bool autoBidFlg;
        //解锁建筑类型id
        std::set<DWORD> m_unLockBuildSet;
        //贡献值
        std::map<QWORD,DWORD> m_contrubuteMap;
        //点赞
        std::set<QWORD> m_likeSet;
        //花钱买联系方式
        std::set<QWORD> m_buyConnectSet;
        static std::map<DWORD,std::vector<pb::ThreeArgPara> >s_randMap;
        std::set<QWORD> m_seeRoomSet;
        std::set<QWORD> m_viewWechatSet;
        std::set<QWORD> m_buyedSet;
        std::map<QWORD,DWORD> m_acceptCharismaMap;
        std::map<QWORD,std::pair<DWORD,DWORD> > m_giftContinue;
        std::map<DWORD,DWORD> m_activeRechargeMap;
        //生产,合成,采购总量
        std::map<DWORD,DWORD >m_sysOrderMap;
        std::map<DWORD,DWORD> m_prodeceMap;
        std::map<DWORD,DWORD> m_compositeMap;
        //激活码类型以及激活码
        std::map<DWORD,std::string> m_codeTypeMap;

};
#endif
