#ifndef E_MAIL_MANAGER_H
#define E_MAIL_MANAGER_H
#include "email.pb.h"
#include "zType.h"
#include <map>
#include "serialize.pb.h"
#include "dataManager.h"

//最大100封邮件
const DWORD MAX_EMAIL_NUM = 100;
enum SystemEmailID
{
    Email_Auction_ID = 1, //自动投票返还
    Email_WareFull_ID = 2, //包裹已满
    Email_ServerRecoup_ID = 3, //服务器维护补偿
    Email_FriendEvent_ID = 4, //好友事件奖励
    Email_OperationActive_ID = 5, //运营活动
    Email_AdditionalReward_ID = 6, //额外奖励
    Email_Auction_Success_ID = 7, //竞拍成功
    Email_BuyNormalGift_ID = 8, //购买常规礼品
    Email_Get_Atlas_ID = 9, //Atlas获得
    Email_LoadTrainOther_Confirm = 10,//帮助别人装载，确认获得奖励
    Email_SuShi_Rank_Reward = 11, //寿司排行奖励
    Email_Unity_InvityFal_Return =12 ,//合建邀请失败返回
    Email_Unity_Cancel =13 ,//合建终止
    Email_Unity_Finish =14 ,//合建完成
    Email_Unity_Like = 15,//点赞
    Email_InviteBuild_NeedAgree = 19,// 合建邀请通知（需要同意）
    Email_InviteBuild_NotNeedAgree = 20,// 合建邀请通知（不需要同意)
    Email_ApplyFamily =21,//族长    申请中  1.XXXXX玩家申请家入您的家族，等待您的批复。
    Email_JoinFamily = 22,//22      申请人  成功加入    2.欢迎你 XXXXXX ，你已成功加入我们的大家族。
    Email_FriendJionFamily = 23,//族长    成功加入    3.XXXXX玩家加入您的家族！   
    Email_QuitFamily =24,//24       成员    退出    4.您已退出 XXXX家族 ，有关家族的一切活动将无法再参与。
    Email_OtherLeaveFamily = 25,//25     族长    退出家族    5.家族成员 XXXXX 玩家，离开了您的家族。 
    Email_RefuseJoinFamily = 26,//26       申请人  拒绝加入    6.很抱歉的通知您，你申请加入 XXXX家族 被族长拒绝！ 
    Email_KickoffFamily = 27,//27     成员    踢出家族    7.很抱歉的通知您，因为种种原因，你被族长从家族名单中移除！
    Email_Star_Reward = 28, //星座连线奖励
    Email_Sushi_Reward = 29, //寿司游戏通关奖励
    Email_Star_Rank_Reward = 30, //星座
    Email_Contribute_Rank_Reward = 31, //月贡献榜
    Email_Charisma_Rank_Reward = 32,  //魅力榜
    Email_Popular_Rank_Reward = 33,  //人气榜
    Email_Active_Code = 34,  //兑换激活码
};

class SceneUser;
class EmailManager
{
    public:
        EmailManager(SceneUser *owner);
    public:
        //刷新所有邮件
        bool flushEmail();
        //更新邮件
        bool updateEmail(const QWORD id);
        //邮件操作
        bool opEmail(const HelloKittyMsgData::ReqOpEmail *message);
        //存档
        bool save(HelloKittyMsgData::Serialize& binary);
        //加载
        bool load(const HelloKittyMsgData::Serialize& binary);
        //接收邮件
        bool acceptEmail(HelloKittyMsgData::EmailInfo &emailInfo);
        //发送邮件
        bool sendEmail(const HelloKittyMsgData::EmailBase *emailBase);
        //删除超时邮件
        bool delTimeOut(const bool notify = true);
        //系统发送道具邮件(默认不需要弹窗)
        static bool sendEmailBySys(const QWORD accepter,const char *title,const char *conten,const std::vector<HelloKittyMsgData::ReplaceWord> &argVec,const std::map<DWORD,DWORD> &itemMap,const bool popup = false);
        //发送邮件
        static bool sendEmail(HelloKittyMsgData::EmailInfo &emailInfo);
        //插入数据库
        static bool insertDB(const HelloKittyMsgData::EmailInfo &emailInfo);
#if 0
        //系统发送道具邮件(默认不需要弹窗)
        static bool sendEmailBySys(const char *nickname,const char *title,const char *conten,const std::vector<HelloKittyMsgData::ReplaceWord> &argVec,const std::map<DWORD,DWORD> &itemMap,const bool popup = false);
#endif
        //请求刷新邮件
        bool reqEmail(const HelloKittyMsgData::ReqEmail *cmd);
        //全服系统邮件
        static bool sendEmailBySys(const char *title,const char *conten,const std::vector<HelloKittyMsgData::ReplaceWord> &argVec,const std::map<DWORD,DWORD> &itemMap,const bool popup = false);
    private:
        //获得邮件
        HelloKittyMsgData::EmailInfo* getEmail(const QWORD id);
        //删除邮件
        bool delEmail(const QWORD id,const bool notify = true,const bool system = true);
        //提取附件
        bool getItem(const QWORD id);
        //从数据库中加载邮件
        bool loadDB();
    private:
        SceneUser *m_owner;
        //邮件id对应邮件
        std::map<QWORD,HelloKittyMsgData::EmailInfo> m_emailMap;
        //邮件id
        static QWORD mailID;
        //true表示邮箱已满
        bool m_full;
};

#endif
