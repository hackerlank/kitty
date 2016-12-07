#ifndef MISC_H__
#define MISC_H__
#include <set>
#include <vector> 
#include <map>
#include "zType.h"
#include "zSingleton.h"
#include "common.pb.h"
enum eParam
{
    eParam_None = 0,
    eParam_Rubbish_Create = 1,//垃圾刷新间隔时间，用vector
    eParam_SweetBoxMax   = 2,//糖果罐容积 ，用single
    eParam_SweetBoxClear_Award = 3,//糖果罐奖励，用vector
    eParam_Family_recommend = 4,//家族推荐列表长度
    eParam_Family_ApplyMax = 5,//申请家族长度
    eParam_Family_MemberMax =6,//家族最大人数
    eParam_Family_NameMax = 7,//家族名最大长度
    eParam_Family_NameMin = 8,//家族名最小长度
    eParam_Family_NoticeMax = 9,//家族公告最大长度
    eParam_Family_TopLen = 10,//排行榜置顶显示名次
    eParam_Family_NearRank = 11,//排行榜自己前后显示名次
    eParam_Family_CreateRes = 12,//创建需求资源
    eParam_Family_CreateNeedBulid = 13,//创建家族需求建筑
    eParam_Order_NeedBulid = 14 ,//订单需求建筑
    eParam_Order_InitNum   =15,//初始订单数量
    eParam_Order_ByPrice   =16,//订单栏位购买价格
    eParam_Guess_Star_Price = 17, //猜星座花费钻石数
    eParam_Make_Sushi_Price = 18, //猜星座花费钻石数
    eParam_Guess_Star_Reward = 19,//猜中星座给予点券数
    eParam_Make_Sushi_Reward = 20,//寿司游戏给予点券数
    eParam_Divine_Price = 21,//占卜花费钻石数
    eParam_Build_Need_Worker = 23, //建筑某个建筑花费工人数
    eParam_Item_System_Price = 24, //道具出售给系统价格
    eParam_Buy_WorkerCD_Price = 25, //购买工人cd钻石数
    eParam_Buy_SallCell_Price = 26, //开通一个摊位格子所消耗钻石数
    eParam_Down_Item_Price = 27, //下架所需钻石数
    eParam_Brush_Paper_Rate = 28, //报纸刷新频率
    eParam_Init_OpenSallCell_Num = 29, //初始化开通摊位数量
    eParam_LeaveMessage = 30, //留言参数，GetSingleParam 得到免费次数 GetVecParam 得到货币类型和需求数量
    eParam_Advertise_CD = 31, //广告cd秒数
    eParam_Clear_Advertise_CD = 32, //清除广告cd
    eParam_Init_Composite_Num = 33, //合成建筑的工作槽
    eParam_Buy_Composite_Cell = 34, //购买工作槽所需钻石数
    eParam_Init_Produce_Num = 35,  //默认生产工作槽数目
    eParam_Max_Produce_Num = 36,  //最大生产工作槽数目
    eParam_Room_Total_Process = 37, //空间进度总量
    eParam_Room_Process_Reward = 38, //空间进度奖励
    eParam_Init_Ware_House_Grid = 39, //默认初始化格子数
    eParam_Expend_Ware_Grid = 40, //每次扩仓库格子数
    eParam_setName_NeedRes = 41, //修改名字需要的资源
    eParam_OpenFirstTrain  = 42,//第一列火车开启
    eParam_OpenSecondTrain  = 43,//第二列火车开启
    eParam_OpenThirdTrain  = 44,//第三列火车开启
    eParam_TrainLocation   = 45,//火车目的地
    eParam_TrainHelpAward  = 46,//帮助别人确认奖励
    eParam_Init_Composite_Item_Num = 47, //合成滞留物品最大数目
    eParam_Danwei_ID = 51, //订单企业名称
    eParam_First_TrainLoadAward = 53,//第一次火车订单需求/物品奖励
    eParam_Market_FlushTime = 54,//黑市刷新时刻（小时）
    eParam_Market_FlushCost = 55,//黑市刷新价格
    eParam_SerVant_OpenBox = 56,//男仆箱子开启价格
    eParam_Item_Give_Exp_Ratio = 58, //生成合成建筑收取道具给经验系数
    eParam_UniteBuild_InviteTimer = 59,//合建邀请等待时间
    eParam_FirstOrder_Define =60,//第一次订单指定ID
    eParam_World_Hom = 64,//世界喇叭
    eParam_City_Hom = 65,//同城喇叭
    eParam_Star_Wait_Resp_Time = 66, //星座连线等待回应时间
    eParam_Star_Paly_Time = 67, //星座连线持续时间
    eParam_Star_Operator_Time = 68, //协作单关
    eParam_Star_Single_Time = 69, //单人单关
    eParam_Buy_Login_Last = 70, //购买查看最近登录数据
    eParam_Buy_View_Wechat = 71, //购买查看微信号
    eParam_Order_Desc = 73, //订单说明
    eParam_Family_OrderNum = 74,//每日订单数量限制
    eParam_SendGift_CD = 75, //送礼cd,现在改为连击判定
    eParam_SendGift_Continue_Notice = 77,//送礼连击倍数通告
    eParam_SlotMachine_Coin_Type = 81, //老虎机消耗货币类型
    eParam_SlotMachine_Coin_Num = 82, //老虎机消耗货币数量
    eParam_Macro_Coin_Type = 83, //万家乐消耗货币类型
    eParam_Macro_Coin_Num = 84, //万家乐消耗货币数量
    eParam_Advertise_Recycle_Time = 94, //广告商品回收
    eParam_Advertise_Recycle_NPC = 95, //回收npcID
    eParam_Grid_Max = 96, //最大摊位数
    eParam_Order_Num = 101, //订单与等级关系
    eParam_Order_OverTime = 102, //订单延长时间



};


class ParamManager : public Singleton<ParamManager>
{
    friend class Singleton<ParamManager>;
    public:
    DWORD GetSingleParam(eParam eparamType);
    std::vector<DWORD> GetVecParam(eParam eparamType);
    private:
    ParamManager(){}


};
#define PARAM_SINGLE(type) ParamManager::getMe().GetSingleParam(type)
#define PARAM_VEC(type) ParamManager::getMe().GetVecParam(type)
enum AdditionalType
{
    AdditionalType_Order = 1,
    AdditionalType_FamilyOerder = 2,
    AdditionalType_BrustEvent = 3,
};
enum eTimerReduce//清除cd需要扣钱的计算
{
    eTimerReduce_First = 1, //订货,生产
    eTimerReduce_Second = 2, //合成
    eTimerReduce_Third = 3,//建筑
    eTimerReduce_Fourth =4,//交易,订单
    eTimerReduce_Fifth = 5,//火车
};
enum eSysNoticeId//系统消息id
{
    eSysNoticeId_InviteBuildNeedAgree = 1, //邀请合建，等待回应
    eSysNoticeId_InviteBuildSuc = 2, //邀请合建成功
    eSysNoticeId_InviteBuildByInvite = 3,//被人邀请合建
    eSysNoticeId_GiftContinue = 4,//虚拟物品连击
    eSysNoticeId_GiftSend = 5,//虚拟物品送礼

};

class SceneUser;
class MiscManager: public Singleton<MiscManager>
{
    friend class  Singleton<MiscManager>;
    public:
    void getAdditionalRewards(SceneUser *puser, AdditionalType type);
    DWORD getMoneyForReduceTimer(eTimerReduce eType,DWORD timer);
    DWORD getMoneyForCreateMaterial( DWORD Totalworth);//购买生产材料时，需求的钻石为道具本身价值/5个 
    DWORD getMoneyForExpandEarthOrWareHouse();//购买扩充仓库，扩地材料需要钻石12个
    DWORD getMoneyForActiveMaterial();//购买激活建筑材料需要钻石8个
    void  SendSysNotice(QWORD charid,eSysNoticeId esysId,std::vector<HelloKittyMsgData::ReplaceWord> &vecArgs);
    private:
    MiscManager(){}
};
#endif// MISC_H__
