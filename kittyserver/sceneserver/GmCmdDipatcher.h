#ifndef GM_CMD_DISPATCHER_H
#define GM_CMD_DISPATCHER_H
#include "zCmdHandle.h"
#include "SceneTask.h"
#include "dispatcher.h"
#include "gm.pb.h"
#include "Fir.h"
#include "zType.h"
#include <map>
#include <vector>
#include "gmtest.pb.h"

class SceneUser;
class GMCmdHandle : public zCmdHandle
{
    public:
        GMCmdHandle()
        {
        }

        void init()
        {
            SceneTask::scene_user_gm_dispatcher.func_reg<HelloKittyMsgData::ReqGM>(ProtoCmdCallback<SceneUser,HelloKittyMsgData::ReqGM>::ProtoFunction(this,&GMCmdHandle::reqGm));
            initGmFunMap();
        }
        //gm函数
        typedef bool (*GmFun)(SceneUser* user,const std::vector<std::string> &commandVec); 
        //gm命令
        bool reqGm(SceneUser* user,const HelloKittyMsgData::ReqGM *cmd);
        static std::map<std::string,std::string> m_helpstr;

    private:
        //是否已经注册函数
        static bool s_initFlg;
        //注册函数容器
        static std::map<std::string,GmFun> s_gmFunMap;
        //注册函数
        void initGmFunMap();
        //给道具
        static bool addItem(SceneUser* user,const std::vector<std::string> &commandVec);
        //获得pid
        static bool getPid(SceneUser* user,const std::vector<std::string> &commandVec);
        //建筑升级
        static bool buildLevel(SceneUser* user,const std::vector<std::string> &commandVec);
        //增加图鉴
        static bool addAtlas(SceneUser* user,const std::vector<std::string> &commandVec);
        //删除某个图鉴
        static bool deleteAtlas(SceneUser* user,const std::vector<std::string> &commandVec);
        //删除所有图鉴
        static bool clearAtlas(SceneUser* user,const std::vector<std::string> &commandVec);
        //完成任务
        static bool finishTask(SceneUser* user,const std::vector<std::string> &commandVec);
        //开启一个任务
        static bool openTask(SceneUser* user,const std::vector<std::string> &commandVec);
        //更改愉悦值维持天数
        static bool changeHappyDay(SceneUser* user,const std::vector<std::string> &commandVec);
        //完成成就
        static bool finishAchieve(SceneUser* user,const std::vector<std::string> &commandVec);
        //更改系统时间
        static bool changeTime(SceneUser* user,const std::vector<std::string> &commandVec);
        //打印现在时间
        static bool nowTime(SceneUser* user,const std::vector<std::string> &commandVec);
        //清空角色所用数据
        static bool clearUserData(SceneUser* user,const std::vector<std::string> &commandVec);
        //随机刷新报纸
        static bool randPaper(SceneUser* user,const std::vector<std::string> &commandVec);
        //增加访问家园人数
        static bool addVistor(SceneUser* user,const std::vector<std::string> &commandVec);
        static bool addfriend(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool delfriend(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool getfriendlist(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool getfanslist(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool getotherlist(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool opBulid(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool visit(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool createfamily(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool joinfamily(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool familyinfo(SceneUser* User,const std::vector<std::string> &commandVec);   
        static bool familyorder(SceneUser* User,const std::vector<std::string> &commandVec); 
        static bool familyaward(SceneUser* User,const std::vector<std::string> &commandVec); 
        static bool openevent(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool calfamily(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool help(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool addservernotice(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool removenotice(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool getAllnotice(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool luatest(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool setName(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool finishguide(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool recordstaticnpc(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool recordactivenpc(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool leavemsg(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool getleavemsg(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool closeguide(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool ReqOpToy(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool RecordPath(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool setlevel(SceneUser* User,const std::vector<std::string> &commandVec);

        static bool ReqAllUnitBuildInfo(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool ReqOpenColId(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool ReqResetColId(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool ReqUnitBuild(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool ReqAgreeUnitBuild(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool ReqCancelInvite(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool ReqStopBuild(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool ReqAddSpeedBuild(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool ReqActiveBuild(SceneUser* User,const std::vector<std::string> &commandVec);
        static bool getunitybuildrank(SceneUser* User,const std::vector<std::string> &commandVec); 
        static bool GetName(SceneUser* User,const std::vector<std::string> &commandVec); 



        //领取建筑产出
        static bool rewardBuild(SceneUser* user,const std::vector<std::string> &commandVec);
        //发送系统邮件
        static bool sendEmail(SceneUser* user,const std::vector<std::string> &commandVec);
        //清空时装
        static bool clearDress(SceneUser* user,const std::vector<std::string> &commandVec);
        //清空图纸
        static bool clearPaper(SceneUser* user,const std::vector<std::string> &commandVec);
        //god指令
        static bool god(SceneUser* user,const std::vector<std::string> &commandVec);
        //重置每天数据
        static bool resetDailyData(SceneUser* user,const std::vector<std::string> &commandVec);
        //重新加载配表文件(注意，仅限于excel)
        static bool reLoadConf(SceneUser* user,const std::vector<std::string> &commandVec);
        //全服发送系统邮件
        static bool sendSysEmail(SceneUser* user,const std::vector<std::string> &commandVec);
        //进入或者退出游艺大厅
        static bool enterCenter(SceneUser* user,const std::vector<std::string> &commandVec);
        //退出或者进入拍卖房间
        static bool enterRoom(SceneUser* user,const std::vector<std::string> &commandVec);
        //打印拍卖信息
        static bool auctionInfo(SceneUser* user,const std::vector<std::string> &commandVec);
        //进行举牌
        static bool auction(SceneUser* user,const std::vector<std::string> &commandVec);
        //打印排行榜
        static bool logRank(SceneUser* user,const std::vector<std::string> &commandVec);
        //通过账号获得角色id
        static bool getCharID(SceneUser* user,const std::vector<std::string> &commandVec);
        //测试
        static bool testGmToolTest(SceneUser* user,const std::vector<std::string> &commandVec);
        //封号
        static bool ReqForBid(SceneUser* user,const std::vector<std::string> &commandVec);
        //更改心跳包
        static bool changeHeartTime(SceneUser* user,const std::vector<std::string> &commandVec);
        //更改礼品状态
        static bool changeGiftStatus(SceneUser* user,const std::vector<std::string> &commandVec);
        //增加buffer
        static bool addBuffer(SceneUser* user,const std::vector<std::string> &commandVec);
        //加贡献值
        static bool addContribute(SceneUser* user,const std::vector<std::string> &commandVec);
        //上传图片
        static bool ReqAddPicture(SceneUser* user,const std::vector<std::string> &commandVec);
        //充值
        static bool recharge(SceneUser* user,const std::vector<std::string> &commandVec);
        static bool upLevel(SceneUser* user,const std::vector<std::string> &commandVec);
};

#endif
