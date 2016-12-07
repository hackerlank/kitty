#include "UnityBuildManager.h"
#include "tbx.h"
#include "zMisc.h"
#include "TimeTick.h"
#include "SceneUser.h"
#include "Misc.h"
#include "zMemDBPool.h"
#include "RedisMgr.h"
#include "SceneMail.h"
#include "SceneTaskManager.h"
#include "SceneMail.h"
#include "SceneUserManager.h"
#include "SceneToOtherManager.h"
#include "key.h"



UnityManager::UnityManager(SceneUser& rUser):m_rUser(rUser)
{

}

UnityManager::~UnityManager()
{

}




void UnityManager::load(const HelloKittyMsgData::Serialize& binary)
{
    for(int i = 0; i != binary.allselfcol_size();i++)
    {
        m_mapUnitColId[binary.allselfcol(i).playercolid()] = binary.allselfcol(i);
    }

}

void UnityManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto it = m_mapUnitColId.begin();it != m_mapUnitColId.end();it++)
    {
        HelloKittyMsgData::UnitPlayerColId *padd = binary.add_allselfcol();
        if(padd)
        {
            *padd = it->second;
        }

    }


}

HelloKittyMsgData::UnitPlayerColId *UnityManager::getColById(DWORD colid)
{
    auto it = m_mapUnitColId.find(colid);
    if(it == m_mapUnitColId.end())
        return NULL;
    return &(it->second);
}

void UnityManager::timercheck()
{
    DWORD nowtimer =   SceneTimeTick::currentTime.sec();
    std::set<DWORD> delset;
    for(auto it = m_mapUnitColId.begin();it != m_mapUnitColId.end();it++)
    {
        HelloKittyMsgData::UnitPlayerColId& rSelf = it->second;
        if(rSelf.timerout() != 0 && rSelf.timerout() <  nowtimer)
        {
            delset.insert(it->first);
        }
    }
    for(auto it = delset.begin();it != delset.end();it++)
    {
        delUnitPlayerCol(*it);
    }


}
void UnityManager::getUnityBuildInfoByInfo(HelloKittyMsgData::UnitRunInfo &info,HelloKittyMsgData::UnitRunInfoForCli* pinfo)
{
    HelloKittyMsgData::UnitRunInfo* psever = pinfo->mutable_serverinfo();
    if(psever == NULL)
        return ;
    *psever = info;
    QWORD otherPlayer = psever->inviteplayer() == m_rUser.charid ?  psever->byinviteplayer()  : psever->inviteplayer();

    HelloKittyMsgData::playerShowbase* pshow = pinfo->mutable_othershow();
    if(pshow)
        SceneUser::getplayershowbase(otherPlayer,*pshow);


}

bool UnityManager::getUnityBuildInfo(QWORD unityID,HelloKittyMsgData::UnitRunInfoForCli* pinfo)
{
    HelloKittyMsgData::UnitRunInfo info;

    if(!RedisMgr::getMe().get_unitybuildata(unityID,info))
        return false;
    getUnityBuildInfoByInfo(info,pinfo);

    return true;


}
DWORD UnityManager::getUnityBuildNums(DWORD buildId)
{
    std::set<QWORD> setOnlyId;
    //已有的
    m_rUser.m_buildManager.getUinityBuildOnlySet(buildId,setOnlyId);
    RedisMgr::getMe().get_unitybuildOnlySet(m_rUser.charid,buildId,setOnlyId);
    return setOnlyId.size();
}

DWORD UnityManager::getUnityBuildNums(QWORD PlayerOther,DWORD buildId)
{
    std::set<QWORD> setOnlyId;
    //已有的
    BuildManager::getUinityBuildOnlySetByPlayerID(PlayerOther,buildId,setOnlyId);
    RedisMgr::getMe().get_unitybuildOnlySet(m_rUser.charid,buildId,setOnlyId);
    return setOnlyId.size();

}



void  UnityManager::addUnitPlayerCol(HelloKittyMsgData::UnitPlayerColId & rInfo,bool bSendCli)
{
    m_mapUnitColId[rInfo.playercolid()] = rInfo;
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(m_rUser.charid);
    if(redishandle)
        redishandle->setSet("unitydata",m_rUser.charid,"opencol",rInfo.playercolid());
    if(bSendCli)
        updateCliColInfoByColId(rInfo.playercolid());




}


void  UnityManager::delUnitPlayerCol(DWORD colId)
{
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(m_rUser.charid);
    if(redishandle)
        redishandle->delSet("unitydata",m_rUser.charid,"opencol",colId);
    m_mapUnitColId.erase(colId);
    updateCliColInfoByColId(colId);

}


void UnityManager::AddBuildTimes(QWORD PlayerId,DWORD colId)
{
    bool bDelLock = true;
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(PlayerId);
    do{
        SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerId);
        if(user)
        {
            user->m_unitybuild.addUnitBuildTimes(colId);
        }
        else
        {
            DWORD SenceId = 0;
            if(handle)
                SenceId = handle->getInt("playerscene",PlayerId,"sceneid");
            if(SenceId != 0)
            {
                CMD::SCENE::t_UnityBuild_AddTimes sendCmd;
                sendCmd.charID = PlayerId;
                sendCmd.colID = colId;
                std::string ret;
                encodeMessage(&sendCmd,sizeof(sendCmd),ret);
                if(SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
                {
                    bDelLock = false;
                    break;
                }

            }
            else
            {
                SceneUser* u  =  SceneUserManager::getMe().CreateTempUser(PlayerId);
                if(u)
                {
                    u->m_unitybuild.addUnitBuildTimes(colId);
                }
            }

        }
    }while(0);
    if(bDelLock)
    {
        if(handle)
        {
            handle->delLock("playerlock",PlayerId,"newplayer");
        }

    }
}

void UnityManager::addUnitBuildTimes(DWORD colId)
{
    HelloKittyMsgData::UnitPlayerColId * pinfo = getColById(colId);
    if(pinfo == NULL)
        return ;
    const pb::Conf_t_UniteGrid *pbuildgridConf = tbx::UniteGrid().get_base(pinfo->selfunitgridid());
    if(pbuildgridConf == NULL)
        return ;
    if(pbuildgridConf->UniteGrid->fieldsusecount() == 0) 
        return ;
    pinfo->set_usetimes(pinfo->usetimes() + 1);
    if(pinfo->usetimes() >= pbuildgridConf->UniteGrid->fieldsusecount())
    {
        delUnitPlayerCol(pinfo->playercolid()); 
    }
    else
    {
        updateCliColInfoByColId(pinfo->playercolid());
    }

}

void UnityManager::SendMailToInviteFalse(QWORD PlayerOther,QWORD PlayerOtherFriend,DWORD BuildId)
{
    std::string strFriendName;
    std::string strBuildName;
    CharBase charbase;
    std::map<DWORD,DWORD> couponsMap;
    if(RedisMgr::getMe().get_charbase(PlayerOtherFriend,charbase))
    {
        strFriendName = std::string(charbase.nickname);
    }
    QWORD key = hashKey(BuildId,1);
    const pb::Conf_t_building *base = tbx::building().get_base(key);
    if(base)
    {
        strBuildName = base->buildInfo->name();
    }
    const pb::Conf_t_UniteBuild *pbuildConf = tbx::UniteBuild().get_base(BuildId); 
    if(pbuildConf)
    {
        couponsMap[pbuildConf->UniteBuild->pricetype()] = pbuildConf->UniteBuild->price();
    }
    SceneMailManager::getMe().sendSysMailToPlayerForReturnUnityBuild(PlayerOther,strFriendName,strBuildName,couponsMap);
}


void UnityManager::NoticeUpdateUnityForAll(HelloKittyMsgData::UnitRunInfo &info)
{

    //self
    {
        updateCliColInfoByColId(info.inviteplayer() == m_rUser.charid ? info.inviteplayercolid() : info.byinviteplayercolid());

    }
    //other
    {
        QWORD PlayerId = info.inviteplayer() == m_rUser.charid ? info.byinviteplayer() : info.inviteplayer();
        DWORD colID = info.inviteplayer() == PlayerId ? info.inviteplayercolid() : info.byinviteplayercolid();

        SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerId);
        if(user)
        {
            user->m_unitybuild.updateCliColInfoByColId(colID);
        }
        else
        {

            zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(PlayerId);
            DWORD SenceId = 0;
            if(handle)
                SenceId = handle->getInt("playerscene",PlayerId,"sceneid");
            if(SenceId != 0)
            {
                CMD::SCENE::t_UnityBuild_NoticeUpdate sendCmd;
                sendCmd.charID = PlayerId;
                sendCmd.colID = colID;
                std::string ret;
                encodeMessage(&sendCmd,sizeof(sendCmd),ret);
                SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size());

            }

        }

    }


}


void UnityManager::pushunitbuild(HelloKittyMsgData::UnitRunInfo &info)
{
    QWORD PlayerId = info.inviteplayer() == m_rUser.charid ?  info.byinviteplayer()  : info.inviteplayer();
    m_rUser.m_buildManager.pushUinityBuild(info.unitonlyid(),info.unitlevel(),PlayerId,info.unitbuildid());

    bool bDelLock = true;
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(PlayerId);
    do{
        SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerId);
        if(user)
        {
            user->m_buildManager.pushUinityBuild(info.unitonlyid(),info.unitlevel(),m_rUser.charid,info.unitbuildid());
        }
        else
        {
            DWORD SenceId = 0;
            if(handle)
                SenceId = handle->getInt("playerscene",PlayerId,"sceneid");
            if(SenceId != 0)
            {
                CMD::SCENE::t_UnityBuild_Push sendCmd;
                sendCmd.charID = PlayerId;
                sendCmd.onlyID = info.unitonlyid();
                sendCmd.buildID = info.unitbuildid();
                sendCmd.buildlevel = info.unitlevel();
                sendCmd.friendID = m_rUser.charid;
                std::string ret;
                encodeMessage(&sendCmd,sizeof(sendCmd),ret);
                if(SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
                {
                    bDelLock = false;
                    break;
                }

            }
            else
            {
                SceneUser* u  =  SceneUserManager::getMe().CreateTempUser(PlayerId);
                if(u)
                {
                    u->m_buildManager.pushUinityBuild(info.unitonlyid(),info.unitlevel(),m_rUser.charid,info.unitbuildid());
                }
            }

        }
    }while(0);
    if(bDelLock)
    {
        if(handle)
        {
            handle->delLock("playerlock",PlayerId,"newplayer");
        }

    }


}



void UnityManager::synunitbuildlevel(HelloKittyMsgData::UnitRunInfo &info)
{
    QWORD PlayerId = info.inviteplayer() == m_rUser.charid ?  info.byinviteplayer()  : info.inviteplayer();
    m_rUser.m_buildManager.setUinityBuildLevel(info.unitonlyid(),info.unitlevel());

    bool bDelLock = true;
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(PlayerId);
    do{
        SceneUser* user = SceneUserManager::getMe().getUserByID(PlayerId);
        if(user)
        {
            user->m_buildManager.setUinityBuildLevel(info.unitonlyid(),info.unitlevel());
        }
        else
        {
            DWORD SenceId = 0;
            if(handle)
                SenceId = handle->getInt("playerscene",PlayerId,"sceneid");
            if(SenceId != 0)
            {

                CMD::SCENE::t_UnityBuild_Syn sendCmd;
                sendCmd.charID = PlayerId;
                sendCmd.onlyID = info.unitonlyid();
                sendCmd.buildlevel = info.unitlevel();
                std::string ret;
                encodeMessage(&sendCmd,sizeof(sendCmd),ret);
                if(SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size()))
                {
                    bDelLock = false;
                    break;
                }

            }
            else
            {
                SceneUser* user  =  SceneUserManager::getMe().CreateTempUser(PlayerId);
                if(user)
                {
                    user->m_buildManager.setUinityBuildLevel(info.unitonlyid(),info.unitlevel());

                }
            }

        }
    }while(0);
    if(bDelLock)
    {
        if(handle)
        {
            handle->delLock("playerlock",PlayerId,"newplayer");
        }

    }

}

void UnityManager::sendmailforcancel(HelloKittyMsgData::UnitRunInfo &info)
{
    std::string strself;
    std::string strother;
    std::string strBuildName;
    QWORD otherPlayer = info.inviteplayer() == m_rUser.charid ?  info.byinviteplayer()  : info.inviteplayer();
    //得到别人名字
    CharBase charbase;
    if(RedisMgr::getMe().get_charbase(otherPlayer,charbase))
    {
        strother = std::string(charbase.nickname);
    }
    strself = std::string(m_rUser.charbase.nickname);
    QWORD key = hashKey(info.unitbuildid(),1);
    const pb::Conf_t_building *base = tbx::building().get_base(key);
    if(base)
    {
        strBuildName = base->buildInfo->name();
    }

    SceneMailManager::getMe().sendSysMailToPlayerCancelUnityBuild(m_rUser.charid,otherPlayer,strself,strother,strBuildName,info.unitlevel(),strself );

}

void UnityManager::sendmailforfinish(HelloKittyMsgData::UnitRunInfo &info)
{
    std::string strself;
    std::string strother;
    std::string strBuildName;
    QWORD otherPlayer = info.inviteplayer() == m_rUser.charid ?  info.byinviteplayer()  : info.inviteplayer();
    //得到别人名字
    CharBase charbase;
    if(RedisMgr::getMe().get_charbase(otherPlayer,charbase))
    {
        strother = std::string(charbase.nickname);
    }
    strself = std::string(m_rUser.charbase.nickname); 
    QWORD key = hashKey(info.unitbuildid(),1);
    const pb::Conf_t_building *base = tbx::building().get_base(key);
    if(base)
    {
        strBuildName = base->buildInfo->name();
    }
    SceneMailManager::getMe().sendSysMailToPlayerFinishUnityBuild(m_rUser.charid,otherPlayer,strself,strother,strBuildName,info.unitlevel());
}

bool UnityManager::getCliColInfoByColId(HelloKittyMsgData::UnitColInfoForCli *pinfo,DWORD ColID)
{
    if(pinfo == NULL)
    {
        return false;
    }
    QWORD qwOnyID = RedisMgr::getMe().get_unitybuilddatabyColId(m_rUser.charid,ColID);
    pinfo->set_playercolid(ColID);
    if(qwOnyID > 0)
    {
        HelloKittyMsgData::UnitRunInfoForCli *pRun = pinfo->mutable_unitinfo();
        if(pRun == NULL)
        {
            return false;
        }
        getUnityBuildInfo(qwOnyID,pRun);
        switch(pRun->serverinfo().state())
        {
            case HelloKittyMsgData::UnitRunState_Invite:
                {
                    if(pRun->serverinfo().inviteplayer() != m_rUser.charid)
                    {
                        pinfo->set_state(HelloKittyMsgData::UnitColInfoForCliState_ByInvite);
                    }
                    else
                    {

                        pinfo->set_state(HelloKittyMsgData::UnitColInfoForCliState_Invite);
                    }
                }
                break;
            case HelloKittyMsgData::UnitRunState_Running:
                {
                    pinfo->set_state(HelloKittyMsgData::UnitColInfoForCliState_Running);
                }
                break;
            case HelloKittyMsgData::UnitRunState_RunningDone:
                {
                    pinfo->set_state(HelloKittyMsgData::UnitColInfoForCliState_RunningDone);
                }
                break;
            case HelloKittyMsgData::UnitRunState_RunningStop:
                {
                    pinfo->set_state(HelloKittyMsgData::UnitColInfoForCliState_RunningStop);
                }
                break;
            default:
                break;
        }

    }
    else if(getColById(ColID) != NULL) 
    {
        HelloKittyMsgData::UnitPlayerColId *pself = getColById(ColID);
        HelloKittyMsgData::UnitPlayerColId *pSave = pinfo->mutable_selfinfo();
        if(pself && pSave)
        {
            *pSave = *pself;
        }
        pinfo->set_state(HelloKittyMsgData::UnitColInfoForCliState_Open);
    }
    else
    {
        pinfo->set_state(HelloKittyMsgData::UnitColInfoForCliState_None);

    }
    return true;

}

void UnityManager::updateCliColInfoByColId(DWORD ColID)
{
    HelloKittyMsgData::ACKUpdateUnitBuild ack;
    if(!getCliColInfoByColId(ack.mutable_colinfo(),ColID))
    {
        return ;

    }
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());
}

bool UnityManager::DoGift(DWORD ColID,QWORD FriendCharid,DWORD giftID)
{
    QWORD qwOnyID = RedisMgr::getMe().get_unitybuilddatabyColId(m_rUser.charid,ColID);
    if(qwOnyID ==0)
    {
        ReturnResult(HelloKittyMsgData::UnitBuildResult_ErrGridId);
        return false;
    }
    const pb::Conf_t_VirtualGiftShop *pvirtualShop = tbx::VirtualGiftShop().get_base(giftID);
    if(pvirtualShop ==NULL || pvirtualShop->virtualShop->other() <= 0)
    {
        ReturnResult(HelloKittyMsgData::UnitBuildResult_NoGift);
        return false;

    }
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
    if(redishandle ==NULL)
    {
        ReturnResult(HelloKittyMsgData::UnitBuildResult_OtherErr);
        return false;
    }
    if(!redishandle->getLock("unitydata",qwOnyID,"lockop",1))
    {
        ReturnResult(HelloKittyMsgData::UnitBuildResult_ServerBusy);
        return false;
    }
    HelloKittyMsgData::UnitRunInfo info;
    if(!RedisMgr::getMe().get_unitybuildata(qwOnyID,info))
    {
        ReturnResult(HelloKittyMsgData::UnitBuildResult_ErrGridId);
        return false;
    }
    if(info.state() != HelloKittyMsgData::UnitRunState_Running)
    {
        ReturnResult(HelloKittyMsgData::UnitBuildResult_NORunning);
        return false;
    }
    if(info.inviteplayer() == m_rUser.charid)
    {
        if(info.byinviteplayer() != FriendCharid)
        {
            ReturnResult(HelloKittyMsgData::UnitBuildResult_ErrGiftPlayer);
            redishandle->delLock("unitydata",qwOnyID,"lockop");
            return false;
        }
    }
    else if(info.byinviteplayer() == m_rUser.charid)
    {
        if(info.inviteplayer() != FriendCharid)
        {
            ReturnResult(HelloKittyMsgData::UnitBuildResult_ErrGiftPlayer);
            redishandle->delLock("unitydata",qwOnyID,"lockop");
            return false;
        }
    }
    else
    {
        ReturnResult(HelloKittyMsgData::UnitBuildResult_NOPower);
        redishandle->delLock("unitydata",qwOnyID,"lockop");
        return false;
    }
    const pb::Conf_t_UniteGrid *pbuildgridConf = tbx::UniteGrid().get_base(info.inviteunitgridid());
    if(pbuildgridConf == NULL)
    {
        ReturnResult(HelloKittyMsgData::UnitBuildResult_OtherErr);

        redishandle->delLock("unitydata",qwOnyID,"lockop");

        return false;
    }
    const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(info.unitbuildid(),info.unitlevel()));
    if(pbuildConf == NULL)
    {
        ReturnResult(HelloKittyMsgData::UnitBuildResult_OtherErr);
        redishandle->delLock("unitydata",qwOnyID,"lockop");
        return false;
    }

    DWORD curscore = info.unitscore();
    curscore += pvirtualShop->virtualShop->other(); 
    DWORD nextscore = pbuildConf->UniteBuildlevel->nextlevelgrow();
    info.set_unitscore(curscore);
    if(curscore >= nextscore) 
    {
        info.set_state(HelloKittyMsgData::UnitRunState_RunningDone);
    }
    HelloKittyMsgData::vecAward* pAwardvec =  info.mutable_vecaward();
    if(pAwardvec)
    {
        bool bFind = false;
        for(int i =0 ; i != pAwardvec->award_size();i++)
        {
            HelloKittyMsgData::Award* pward = pAwardvec->mutable_award(i);
            if(pward->awardtype() == giftID)
            {
                pward->set_awardval(pward->awardval()+1);
                bFind = true;
                break;
            }
        }
        if(!bFind)
        {
            HelloKittyMsgData::Award* pward = pAwardvec->add_award();
            if(pward)
            {
                pward->set_awardtype(giftID);
                pward->set_awardval(1);
            }
        }

    }
    RedisMgr::getMe().set_unitybuildata(info);
    //通知recorder 已经存好
    redishandle->setSet("unitydata",QWORD(0),"update",info.unitonlyid());
    NoticeUpdateUnityForAll(info);
    //Lock
    redishandle->delLock("unitydata",qwOnyID,"lockop");
    return true;

}

bool UnityManager::fullMessage(HelloKittyMsgData::UserBaseInfo &binary)
{
    const std::unordered_map<unsigned int, const pb::Conf_t_UniteGridInit *> &tbxMap = tbx::UniteGridInit().getTbxMap();
    std::set<DWORD> setCol;

    for(auto it = tbxMap.begin();it != tbxMap.end();it++)
    {
        setCol.insert(it->first);
    }
    for(auto it = setCol.begin();it != setCol.end();it++)
    {
        HelloKittyMsgData::UnitColInfoForCli *pinfo =  binary.add_unitybuildcolinfo();
        getCliColInfoByColId(pinfo,*it);

    }
    return true;

}

void UnityManager::ReturnResult(HelloKittyMsgData::UnitBuildResult result)
{
    HelloKittyMsgData::ACKOpUnitBuild ack;
    std::string ret;
    ack.set_result(result);
    encodeMessage(&ack,ret); 
    m_rUser.sendCmdToMe(ret.c_str(),ret.size()); 


}
