/**
 * \file
 * \version  $Id: SceneMailManager.h 42 2013-04-10 07:33:59Z  $
 * \author  王海军, wanghaijun@ztgame.com 
 * \date 2006年01月04日 16时55分37秒 CST
 * \brief 网关到场景数据缓冲发送
 *
 * 
 */


#ifndef _SceneMailMANAGER_H_
#define _SceneMailMANAGER_H_

#include "zSingleton.h"
#include "event.pb.h"

/**
 ** \brief 定义服务器信息采集连接的客户端管理容器
 **/
class SceneMailManager : public Singleton<SceneMailManager>
{
    friend class Singleton<SceneMailManager>;
    public:
    ~SceneMailManager(){};
    void sendSysMailToPlayerForEvent(QWORD PlayerId,DWORD eventId,QWORD OwerId,const HelloKittyMsgData::vecAward& rvecAward);
    void sendSysMailToPlayerForAdditionalRewards(QWORD PlayerId,DWORD AdditionalID,const HelloKittyMsgData::vecAward& rvecAward);
    void sendSysMailToPlayerForConfirmLoad(QWORD PlayerId,const std::string& ownername,const HelloKittyMsgData::vecAward& rvecAward);
    void sendSysMailToPlayerForReturnUnityBuild(QWORD PlayerId,const std::string& Othername,const std::string&buildname,const std::map<DWORD,DWORD>& couponsMap);
    void sendSysMailToPlayerForReturnUnityBuild(QWORD SelfId,QWORD OtherId,const std::string& strname,const std::string&buildname,const std::map<DWORD,DWORD>& couponsMap);
    void sendSysMailToPlayerCancelUnityBuild(QWORD charid,QWORD otherPlayer,const std::string& strself,const std::string& strother,const std::string& strBuildName,DWORD unitlevel,const std::string& strcancel);
    void sendSysMailToPlayerFinishUnityBuild(QWORD charid,QWORD otherPlayer,const std::string& strself,const std::string& strother,const std::string& strBuildName,DWORD unitlevel);
    void sendSysMailToPlayerInviteBuildNeedAgree(QWORD charid,const std::string& strBuildName,const std::string& strother);
   void sendSysMailToPlayerInviteBuildNotNeedAgree(QWORD charid,const std::string& strBuildName,const std::string& strother);
   //21       族长    申请中  1.XXXXX玩家申请家入您的家族，等待您的批复。
   void sendSysMailToPlayerApplyFamily(QWORD charid,const std::string& strother);

   //22 申请人  成功加入    2.欢迎你 XXXXXX ，你已成功加入我们的大家族。                
   void sendSysMailToPlayerJoinFamily(QWORD charid,const std::string& strself,const std::string& strfamily);
   //23     族长    成功加入    3.XXXXX玩家加入您的家族！       
   void sendSysMailToPlayerFriendJoinFamily(QWORD charid,const std::string& strother);
   //24       成员    退出    4.您已退出 XXXX家族 ，有关家族的一切活动将无法再参与。                  
   void sendSysMailToPlayerQuitFamily(QWORD charid,const std::string& strother);
   //25     族长    退出家族    5.家族成员 XXXXX 玩家，离开了您的家族。         
   void sendSysMailToPlayerLeaveFamily(QWORD charid,const std::string& strother);
   //26       申请人  拒绝加入    6.很抱歉的通知您，你申请加入 XXXX家族 被族长拒绝！                  
   void sendSysMailToPlayerRefuseJoin(QWORD charid,const std::string& strother); 
   //27     成员    踢出家族    7.很抱歉的通知您，因为种种原因，你被族长从家族名单中移除！                  
    void sendSysMailToPlayerKickFamily(QWORD charid);

    private:
    SceneMailManager(){};

};

#endif

