#ifndef TASK_ATTR_H
#define TASK_ATTR_H
#include "zType.h"

enum TargetRetType
{
    Target_Ret_None = 0,//什么都不做
    Target_Ret_Update = 1,//需要跟新
    Target_Ret_Finish = 2,//任务完成
};

enum TaskType
{
    Task_Type_Default = 0,//错误
    Task_Type_Main = 1,//主线任务
    Task_Type_Day = 2, //日常任务
    Task_Type_Burst = 3, //突发任务
};

enum TaskTargetType
{
    Target_Default = 0, //默认值
    Target_Build = 1,  //建筑(建造或者升级)
    Target_InterActive = 2, //互动型
    Target_Avartar = 3, //换装(时装)
    Target_Atlas = 4, //图鉴收集
    Target_Role_Level = 5, //玩家等级提升
    Target_Divine = 6, //占扑
    Target_Vister_Garden = 7, //访问别的乐园
    Target_Add_Source = 8,//获得资源
    Target_Toy = 9, //扭蛋
};

enum AttrID
{
    Attr_Default = 0,//默认值
    Attr_Build = 1,//建筑
    Attr_Add_Build_or_Level = 2,//每天建造或升级数量

    Attr_Frined = 10,//好友
    Attr_Family = 11,//家庭
    Attr_Friend_Event = 12, //好友事件
    Attr_Enter_Garden = 13,//访问别人家乐园
    Attr_Eme_Event = 14,  //突发事件
    Attr_Add_Friend = 15, //每天增加好友数量
    Attr_Add_Finish_Burst_Event = 16, //每天完成的突发事件数量

    Attr_Fashion_Dress = 50, //穿上时装事件
    Attr_Fashion_Has = 51, //拥有时装事件
    Attr_Fashion_Change = 52, //更换时装事件
    Attr_Fashion_Level = 53, //升级时装
    Attr_Fashion_Get = 54, //获得时装事件

    Attr_Open_Avartar = 61,//开启图鉴
    Attr_Avartar = 62,//获得图鉴
    Attr_Add_Atlas = 63, //每天获得的图鉴数量

    Attr_Role_Level = 70,//角色等级

    Attr_Divine = 80,//占扑

    Attr_Money_Gold = 90,//货币金币
    Attr_Money_Gem = 91, //货币钻石
    Attr_Visitor = 92,//游客
    Attr_Happy_val = 93,//愉悦值
    Attr_Add_Money_Gold = 94, //增加金币
    Attr_Add_Money_Gem = 95,  //增加的钻石
    Attr_Add_Visitor = 96, //增加的游客
    Attr_Add_Happy_Val = 97, //增加的愉悦值
    Attr_Open_Grid_Val = 98, //扩地格子数
    Attr_Finish_Order = 99,  //完成订单数
    Attr_Carnival_Num = 100, //开启嘉年华次数
    Attr_Popular_Now = 101 , //当前人气值
    Attr_Popular_Max = 102, //最大人气值
    Attr_Cost_Gem = 103, //每天消耗钻石数
    Attr_Finish_Task = 104, //每日完成任务数
    Attr_Order_Num = 105,   //每日订货增加数量
    Attr_Composite_Num = 106, //合成道具数量
    Attr_Trade_Num = 107,   //每日交易次数数量
    Attr_Train_Get = 108,   //每日火车收获高级材料数量
    Attr_Help_Train = 109,  //每日帮助好友装火车数量
    Attr_Finish_Daily_Task = 110,  //每日完成日常任务数
    Attr_Acculate_Popular_Now = 111, //累积当前人气值
    Attr_Order_Accept_Num = 112,   //每日接收订货增加数量
    Attr_SystemOrder_Get = 113,   //采购获得 
    Attr_Composite_Get = 114,   //合成
    Attr_Produce_Get = 115,  //生产
    

    Attr_Toy = 140, //扭蛋

    Attr_Main_Task = 150, //主线任务
    Attr_Day_Task = 160, //日常任务
    Attr_Total_Task = 170, //任务总量

    Attr_KittyGraden_Process = 180,//乐园综合分
};

struct TaskArgue
{
    TaskTargetType targerType;
    AttrID attrID;
    DWORD key;
    DWORD value;
    bool initFlg;
    TaskArgue(const TaskTargetType &targerType = Target_Default,const AttrID &attrID = Attr_Default,const DWORD key = 0,const DWORD value = 0,const bool initFlg = false) : targerType(targerType),attrID(attrID),key(key),value(value),initFlg(initFlg)
    {
    }
};

//成就
enum AchieveTargetType
{
    Achieve_Target_Default = 0, //默认值
    Achieve_Target_Have = 1,  //资源拥有
    Achieve_Target_Exchange = 2, //兑换
};

enum AchieveSubTargetType
{
    Achieve_Sub_Default = 0,//默认值
    Achieve_Sub_Buid_Level = 1,//建筑升级类
    Achieve_Sub_Build_Num = 2, //建筑数量类
    Achieve_Sub_Sorce_Day = 3,//资源拥有天数类
    Achieve_Sub_Sorce_Num = 4,//资源拥有数量类
    Achieve_Sub_Money_Exchange = 10, //金钱兑换类
};

struct AchieveArg
{
    AchieveTargetType targerType;
    AchieveSubTargetType subType;
    DWORD key;
    DWORD value;
    bool initFlg;
    AchieveArg(const AchieveTargetType &targerType = Achieve_Target_Default,const AchieveSubTargetType &subType = Achieve_Sub_Default,const DWORD key = 0,const DWORD value = 0,const bool initFlg = false) : targerType(targerType),subType(subType),key(key),value(value),initFlg(initFlg)
    {
    }
};



#endif
