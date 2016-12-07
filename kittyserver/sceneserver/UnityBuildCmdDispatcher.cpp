#include "UnityBuildCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "extractProtoMsg.h"
#include "tbx.h"  
#include "Misc.h"
#include "TimeTick.h"
#include "RedisMgr.h"
#include "key.h"
#include "uuid.h"
#include "SceneMail.h"
//#ifdef __DEBUG
//const DWORD OneDay = 60;
//#else
const DWORD OneDay = 24*3600;
//#endif
bool unitbuildCmdHandle::ReqAllUnitBuildInfo(SceneUser* User,const HelloKittyMsgData::ReqAllUnitBuildInfo *message)
{
    HelloKittyMsgData::ACKAllUnitBuildInfo ack;
    const std::unordered_map<unsigned int, const pb::Conf_t_UniteGridInit *> &tbxMap = tbx::UniteGridInit().getTbxMap();
    std::set<DWORD> setCol;

    for(auto it = tbxMap.begin();it != tbxMap.end();it++)
    {
        setCol.insert(it->first);
    }
    for(auto it = setCol.begin();it != setCol.end();it++)
    {
        HelloKittyMsgData::UnitColInfoForCli *pinfo =  ack.add_allcolinfo();
        User->m_unitybuild.getCliColInfoByColId(pinfo,*it);

    }

    std::string ret;     
    encodeMessage(&ack,ret); 
    User->sendCmdToMe(ret.c_str(),ret.size()); 
    return true;
}

bool unitbuildCmdHandle::ReqOpenColId(SceneUser* User,const HelloKittyMsgData::ReqOpenColId *message)
{
    DWORD param = 0;
    HelloKittyMsgData::UnitBuildResult result = HelloKittyMsgData::UnitBuildResult_Suc;
    do{
        const pb::Conf_t_UniteGrid *pbuildgridConf = tbx::UniteGrid().get_base(message->unitgridid());
        if(pbuildgridConf == NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_ErrGridId;
            break;
        }
        const pb::Conf_t_UniteGridInit *pbuildcolConf = tbx::UniteGridInit().get_base(message->playercolid());
        if(pbuildcolConf == NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_ErrColId;
            break;

        }
        if(pbuildcolConf->setOpenGridId.find(message->unitgridid()) == pbuildcolConf->setOpenGridId.end())
        {
            result = HelloKittyMsgData::UnitBuildResult_GridIdDisMatchColId;
            break;
        }
        if(User->m_unitybuild.getColById(message->playercolid()))
        {
            result = HelloKittyMsgData::UnitBuildResult_HasOpen;
            break;
        }
        std::map<DWORD,DWORD> materialMap;
        if(!pbuildgridConf->setPricetype.empty())
        {

            for(auto it = pbuildgridConf->setPricetype.begin();it != pbuildgridConf->setPricetype.end();it ++)
            {
                materialMap[*it] = pbuildgridConf->UniteGrid->price();
                if(User->checkMaterialMap(materialMap))
                {
                    param = 0;
                    break;
                }
                else
                { 
                    if(param == 0)
                    param = *it;
                    materialMap.clear();
                }

            }
            if(materialMap.empty())
            {
                result = HelloKittyMsgData::UnitBuildResult_NoResource;
                break;

            }

        }

        QWORD unityID = RedisMgr::getMe().get_unitybuilddatabyColId(User->charid,message->playercolid());
        if(unityID > 0)
        {
            result = HelloKittyMsgData::UnitBuildResult_HasOpen;
            break;
        }
        //材料扣除不成功的不能开
        if(pbuildgridConf->UniteGrid->consume() == 1 && !materialMap.empty())//扣除
        {
            User->reduceMaterialMap(materialMap,"openunitybuild  reduce");
        }
        //开启
        HelloKittyMsgData::UnitPlayerColId subInfo;
        subInfo.set_playercolid(message->playercolid());
        subInfo.set_selfunitgridid(message->unitgridid());
        if(pbuildgridConf->UniteGrid->fieldslifedate() > 0)
        {
            subInfo.set_timerout(pbuildgridConf->UniteGrid->fieldslifedate()*OneDay + SceneTimeTick::currentTime.sec());
        }
        else
        {
            subInfo.set_timerout(0);
        }
        subInfo.set_usetimes(0);
        User->m_unitybuild.addUnitPlayerCol(subInfo,true);
    }while(0);
    ReturnResult(User,result,param);
    return true;
}

bool unitbuildCmdHandle::ReqResetColId(SceneUser* User,const HelloKittyMsgData::ReqResetColId *message)
{
    HelloKittyMsgData::UnitBuildResult result = HelloKittyMsgData::UnitBuildResult_Suc;
    DWORD param = 0;
    do{
        const pb::Conf_t_UniteGrid *pbuildgridConf = tbx::UniteGrid().get_base(message->unitgridid());
        if(pbuildgridConf == NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_ErrGridId;
            break;
        }
        const pb::Conf_t_UniteGridInit *pbuildcolConf = tbx::UniteGridInit().get_base(message->playercolid());
        if(pbuildcolConf == NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_ErrColId;
            break;

        }
        if(pbuildcolConf->setOpenGridId.find(message->unitgridid()) == pbuildcolConf->setOpenGridId.end())
        {
            result = HelloKittyMsgData::UnitBuildResult_GridIdDisMatchColId;
            break;
        }
        if(User->m_unitybuild.getColById(message->playercolid()))
        {
            result = HelloKittyMsgData::UnitBuildResult_HasOpen;
            break;
        }
        std::map<DWORD,DWORD> materialMap;
        if(!pbuildgridConf->setPricetype.empty())
        {

            for(auto it = pbuildgridConf->setPricetype.begin();it != pbuildgridConf->setPricetype.end();it ++)
            {
                materialMap[*it] = pbuildgridConf->UniteGrid->price();
                if(User->checkMaterialMap(materialMap))
                {
                    param = 0;
                    break;
                }
                else
                {
                    if(param == 0)
                    param = *it;
                    materialMap.clear();
                }

            }
            if(materialMap.empty())
            {
                result = HelloKittyMsgData::UnitBuildResult_NoResource;
                break;

            }

        }

        QWORD unityID = RedisMgr::getMe().get_unitybuilddatabyColId(User->charid,message->playercolid());
        if(unityID == 0)
        {
            result =  HelloKittyMsgData::UnitBuildResult_NoStopRunning;
            break;
        }
        //锁定
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
        if(redishandle ==NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_OtherErr;
            break;
        }
        if(!redishandle->getLock("unitydata",unityID,"lockop",1)) 
        {
            result = HelloKittyMsgData::UnitBuildResult_ServerBusy;
            break;
        }
        //如果当前等级大于等于即将设置的等级，报错
        HelloKittyMsgData::UnitRunInfo binary;
        if(!RedisMgr::getMe().get_unitybuildata(unityID,binary))
        {
            redishandle->delLock("unitydata",unityID,"lockop");
            result = HelloKittyMsgData::UnitBuildResult_OtherErr;
            break;
        }
        if(binary.unitlevel() >= pbuildgridConf->UniteGrid->buildmaxlevel())
        {
            redishandle->delLock("unitydata",unityID,"lockop");

            result =  HelloKittyMsgData::UnitBuildResult_ResetUnityErr;
            break;
        }
        //如果该栏位不处于UnitRunState_RunningStop状态，报错
        if(binary.state() != HelloKittyMsgData::UnitRunState_RunningStop)
        {
            redishandle->delLock("unitydata",unityID,"lockop");

            result =  HelloKittyMsgData::UnitBuildResult_NoStopRunning;
            break;
        }

        //更改刷新时间，合建id，让其成长
        //binary.set_inviteunitgridid(message->unitgridid());
        if(pbuildgridConf->UniteGrid->fieldslifedate() > 0)
        {
            binary.set_timerout(pbuildgridConf->UniteGrid->fieldslifedate()*OneDay + SceneTimeTick::currentTime.sec());
        }
        else
        {
            binary.set_timerout(0);
        }

        binary.set_state(HelloKittyMsgData::UnitRunState_Running);
        binary.set_unitlastchecktimer(SceneTimeTick::currentTime.sec());

        //材料扣除不成功的不能开
        if(pbuildgridConf->UniteGrid->consume() == 1 && !materialMap.empty())//扣除
        {
            User->reduceMaterialMap(materialMap,"openunitybuild  reduce");
        }
        //开启
        HelloKittyMsgData::UnitPlayerColId subInfo;
        subInfo.set_playercolid(message->playercolid());
        subInfo.set_selfunitgridid(message->unitgridid());
        if(pbuildgridConf->UniteGrid->fieldslifedate() > 0)
        {
            subInfo.set_timerout(pbuildgridConf->UniteGrid->fieldslifedate()*OneDay + SceneTimeTick::currentTime.sec());
        }
        else
        {
            subInfo.set_timerout(0);
        }
        subInfo.set_usetimes(0);
        User->m_unitybuild.addUnitPlayerCol(subInfo,false);
        Updateunitbuild(User,binary);
        redishandle->delLock("unitydata",unityID,"lockop");

    }while(0);
    ReturnResult(User,result,param);
    return true;
}


bool unitbuildCmdHandle::ReqUnitBuild(SceneUser* User,const HelloKittyMsgData::ReqUnitBuild *message)
{
    DWORD param = 0;
    HelloKittyMsgData::UnitBuildResult result = HelloKittyMsgData::UnitBuildResult_Suc;
    do{
        //检查是否有这个人
        if(message->byinviteplayer() == User->charid)
        {
            result = HelloKittyMsgData::UnitBuildResult_OtherPlayerERR;
            break;
        }
        CharBase charbase;
        if(!RedisMgr::getMe().get_charbase(message->byinviteplayer(),charbase))
        {
            result = HelloKittyMsgData::UnitBuildResult_OtherPlayerERR;
            break;
        }


        //该栏位没有开
        HelloKittyMsgData::UnitPlayerColId *pinfo =  User->m_unitybuild.getColById(message->playercolid());
        if(pinfo == NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_NoOpen;
            break;
        }
        const pb::Conf_t_UniteGridInit *pbuildcolConf = tbx::UniteGridInit().get_base(message->playercolid());
        if(pbuildcolConf == NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_ErrColId;
            break;

        }
        //建筑id不合法
        const pb::Conf_t_UniteGrid *pbuildgridConf = tbx::UniteGrid().get_base(pinfo->selfunitgridid());
        if(pbuildgridConf == NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_ErrGridId;
            break;
        }
        const pb::Conf_t_UniteBuild *pbuildConf = tbx::UniteBuild().get_base(message->unitbuildid());
        if(pbuildConf == NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_ErrBuildId;
            break;
        }
        if(pbuildgridConf->setBuildtype.find(pbuildConf->UniteBuild->type()) == pbuildgridConf->setBuildtype.end())
        {
            result = HelloKittyMsgData::UnitBuildResult_GridIdDisMatchBuildId;
            break;
        }
        //你拥有该建筑，已超上限
        if(pbuildConf->UniteBuild->count() > 0 && User->m_unitybuild.getUnityBuildNums(message->unitbuildid()) >= pbuildConf->UniteBuild->count())
        {
            if(User->m_unitybuild.getUnityBuildNums(message->unitbuildid()) > pbuildConf->UniteBuild->count())
            {
                result = HelloKittyMsgData::UnitBuildResult_BuildNumMore;
                break;
            }
            else if(User->m_unitybuild.getUnityBuildNums(message->byinviteplayer(),message->unitbuildid()) > pbuildConf->UniteBuild->count()) 
            {

                result = HelloKittyMsgData::UnitBuildResult_OtherBuildNumMore;
                break;
            }

        }

        //支付方式不合法
        if(pbuildgridConf->UniteGrid->paymenttype() == 1)
        {
            if(message->paytype() == HelloKittyMsgData::PayType_Other)
            {
                result = HelloKittyMsgData::UnitBuildResult_PayTypeErr;
                break;
            }
        }
        //检查资源
        std::map<DWORD,DWORD> materialMap;
        materialMap[pbuildConf->UniteBuild->pricetype()] = pbuildConf->UniteBuild->price();
        if(message->paytype() == HelloKittyMsgData::PayType_Self)
        {
            if(!User->checkMaterialMap(materialMap))
            {
                param = pbuildConf->UniteBuild->pricetype();
                result = HelloKittyMsgData::UnitBuildResult_NoResource;
                break;

            }
        }
        //贡献值判定
        if(pbuildgridConf->UniteGrid->needrelationship() > 0)
        {
            if(User->getContribute(message->byinviteplayer()) < pbuildConf->UniteBuild->relationship())
            {
                result = HelloKittyMsgData::UnitBuildResult_LessContribute;
                break;
            }
        }
        //加锁
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
        if(redishandle ==NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_OtherErr;
            break;
        }
        if(!redishandle->getLock("unitydata",User->charid,"lockcreate",1)) 
        {
            result = HelloKittyMsgData::UnitBuildResult_ServerBusy;
            break;
        }
        if(!redishandle->getLock("unitydata",message->byinviteplayer(),"lockcreate",1)) 
        {
            redishandle->delLock("unitydata",User->charid,"lockcreate");
            result = HelloKittyMsgData::UnitBuildResult_ServerBusy;
            break;
        }
        //该栏位已被占用
        std::set<DWORD> setUse;
        RedisMgr::getMe().get_unitybuildcol(User->charid,setUse,0);
        if(setUse.find(message->playercolid()) != setUse.end())
        {
            redishandle->delLock("unitydata",User->charid,"lockcreate");
            redishandle->delLock("unitydata",message->byinviteplayer(),"lockcreate");
            result = HelloKittyMsgData::UnitBuildResult_ColUse;
            break;
        }
        //对方没有空置栏位
        std::set<DWORD> OthersetUse;
        RedisMgr::getMe().get_unitybuildcol(message->byinviteplayer(),OthersetUse,0);
        std::set<QWORD> OtherOpenSet;
        zMemDB* redishandle2 = zMemDBPool::getMe().getMemDBHandle(message->byinviteplayer());
        if(redishandle2)
            redishandle2->getSet("unitydata",message->byinviteplayer(),"opencol",OtherOpenSet);
        //偏列所有栏位
        std::set<DWORD> useCol;
        std::set<DWORD> emptycol;
        for(auto it = pbuildcolConf->setOtherColId.begin();it != pbuildcolConf->setOtherColId.end();it++)
        {
            if(OthersetUse.find(*it) != OthersetUse.end())
            {
                continue;
            }
            if(OtherOpenSet.find(*it) == OtherOpenSet.end())
            {
                emptycol.insert(*it);
            }
            else
            {
                useCol.insert(*it);
            }


        }
        if(emptycol.empty() && useCol.empty())
        {
            redishandle->delLock("unitydata",User->charid,"lockcreate");
            redishandle->delLock("unitydata",message->byinviteplayer(),"lockcreate");
            result = HelloKittyMsgData::UnitBuildResult_FullCol;
            break;
        }
        //先找空位
        DWORD OtherUseCol = 0;
        bool bUseOpen = false;
        if(!emptycol.empty())
        {
            OtherUseCol = *(emptycol.begin());
        }
        else
        {
            OtherUseCol = *(useCol.begin());
            bUseOpen = true;
        }
        //是否需要发邀请
        bool bNeedInvite = bUseOpen || message->paytype() == HelloKittyMsgData::PayType_Other || pbuildgridConf->UniteGrid->needotherargee() > 0;
        if(message->paytype() == HelloKittyMsgData::PayType_Self)
        {
            User->reduceMaterialMap(materialMap,"UnityBuild  reduce");
        }
        //构造合建
        HelloKittyMsgData::UnitRunInfo info;
        info.set_unitonlyid(utils::unique_id_t::generate(HelloKittyMsgData::GUIDType_UNITYBUILD,SceneService::getMe().getServerID()));
        info.set_paytype(message->paytype());
        info.set_inviteplayer(User->charid);
        info.set_inviteplayercolid(message->playercolid());
        info.set_invitespeedtimes(0);
        info.set_byinviteplayer(message->byinviteplayer());
        info.set_byinviteplayercolid(OtherUseCol);
        info.set_byinvitespeedtimes(0);
        info.set_lastspeedtimer(0);
        info.set_inviteunitgridid(pinfo->selfunitgridid());
        info.set_timerout(pinfo->timerout());
        info.set_unitbuildid(message->unitbuildid());
        info.set_unitscore(0);
        info.set_unitlevel(0);
        info.set_unitlastchecktimer(SceneTimeTick::currentTime.sec());
        info.set_state(bNeedInvite ? HelloKittyMsgData::UnitRunState_Invite : HelloKittyMsgData::UnitRunState_Running);
        HelloKittyMsgData::vecAward* paward = info.mutable_vecaward();
        if(paward ==NULL)
        {
            Fir::logger->error("new err");
        }

        if(!bNeedInvite)
        {
            if(pbuildgridConf->UniteGrid->fieldsusecount() > 0)
            {
                pinfo->set_usetimes(pinfo->usetimes() + 1);
                if(pinfo->usetimes() >= pbuildgridConf->UniteGrid->fieldsusecount())
                {
                    User->m_unitybuild.delUnitPlayerCol(pinfo->playercolid());

                }
                else
                {
                    User->m_unitybuild.updateCliColInfoByColId(pinfo->playercolid());
                }
            }
            //邀请方
            //发送邀请成功系统消息
            std::vector<HelloKittyMsgData::ReplaceWord> vargs;
            HelloKittyMsgData::ReplaceWord pParamself;
            pParamself.set_key(HelloKittyMsgData::ReplaceType_NONE);
            CharBase charbase;
            if(RedisMgr::getMe().get_charbase(message->byinviteplayer(),charbase))
                pParamself.set_value(charbase.nickname);
            else
                pParamself.set_value("");
            vargs.push_back(pParamself);
            MiscManager::getMe().SendSysNotice(User->charid,eSysNoticeId_InviteBuildSuc,vargs);
            //被邀请方
            HelloKittyMsgData::ReplaceWord pParamother;
            pParamother.set_key(HelloKittyMsgData::ReplaceType_NONE);
            pParamother.set_value(User->charbase.nickname);
            vargs.clear();
            vargs.push_back(pParamother);
            MiscManager::getMe().SendSysNotice(message->byinviteplayer(),eSysNoticeId_InviteBuildByInvite,vargs);
            //被邀请方邮件
            const pb::Conf_t_itemInfo *base = tbx::itemInfo().get_base(message->unitbuildid());
            if(base)
                SceneMailManager::getMe().sendSysMailToPlayerInviteBuildNotNeedAgree(message->byinviteplayer(),base->itemInfo->item(),User->charbase.nickname);


        }
        else
        {
            //邀请方
            std::vector<HelloKittyMsgData::ReplaceWord> vargs;
            HelloKittyMsgData::ReplaceWord  pParamself;
            pParamself.set_key(HelloKittyMsgData::ReplaceType_NONE);
            CharBase charbase;
            if(RedisMgr::getMe().get_charbase(message->byinviteplayer(),charbase))
                pParamself.set_value(charbase.nickname);
            else
                pParamself.set_value("");
            vargs.push_back(pParamself);
            MiscManager::getMe().SendSysNotice(User->charid,eSysNoticeId_InviteBuildNeedAgree,vargs);

            //被邀请方
            vargs.clear();
            HelloKittyMsgData::ReplaceWord  pParamother;
            pParamother.set_key(HelloKittyMsgData::ReplaceType_NONE);
            pParamother.set_value(User->charbase.nickname);
            vargs.push_back(pParamother);
            MiscManager::getMe().SendSysNotice(message->byinviteplayer(),eSysNoticeId_InviteBuildByInvite,vargs);
            //被邀请方邮件
            const pb::Conf_t_itemInfo *base = tbx::itemInfo().get_base(message->unitbuildid());
            if(base)
                SceneMailManager::getMe().sendSysMailToPlayerInviteBuildNeedAgree(message->byinviteplayer(),base->itemInfo->item(),User->charbase.nickname);


        }
        Addunitbuild(User,info);
        redishandle->delLock("unitydata",User->charid,"lockcreate");
        redishandle->delLock("unitydata",message->byinviteplayer(),"lockcreate");


    }while(0);
    ReturnResult(User,result,param);
    return true;

}

void unitbuildCmdHandle::Addunitbuild(SceneUser* User,HelloKittyMsgData::UnitRunInfo &info)
{
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(); 
    if(redishandle == NULL)
        return ;
    //先存到redis
    RedisMgr::getMe().set_unitybuildata(info);
    //通知recorder 已经存好
    redishandle->setSet("unitydata",QWORD(0),"update",info.unitonlyid());

    //设置关联
    zMemDB* redishandle1 = zMemDBPool::getMe().getMemDBHandle(info.inviteplayer());
    if(redishandle1)
    {
        redishandle1->setSet("unitydata",info.inviteplayer(),"Onlyunity",info.unitonlyid());
    }
    zMemDB* redishandle2 = zMemDBPool::getMe().getMemDBHandle(info.byinviteplayer());
    if(redishandle2)
    {
        redishandle2->setSet("unitydata",info.byinviteplayer(),"Onlyunity",info.unitonlyid());
    }
    User->m_unitybuild.NoticeUpdateUnityForAll(info);
}

void unitbuildCmdHandle::Updateunitbuild(SceneUser* User,HelloKittyMsgData::UnitRunInfo &info)
{
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(); 
    if(redishandle == NULL)
        return ;
    //先存到redis
    RedisMgr::getMe().set_unitybuildata(info);
    //通知recorder 已经存好
    redishandle->setSet("unitydata",QWORD(0),"update",info.unitonlyid());
    User->m_unitybuild.NoticeUpdateUnityForAll(info);
}

void unitbuildCmdHandle::Delunitbuild(SceneUser* User,HelloKittyMsgData::UnitRunInfo &binary)
{
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(); 
    if(redishandle == NULL)
        return ;
    zMemDB* redishandle1 = zMemDBPool::getMe().getMemDBHandle(binary.inviteplayer());
    if(redishandle1)
    {
        redishandle1->delSet("unitydata",binary.inviteplayer(),"Onlyunity",binary.unitonlyid());
    }
    zMemDB* redishandle2 = zMemDBPool::getMe().getMemDBHandle(binary.byinviteplayer());
    if(redishandle2)
    {
        redishandle2->delSet("unitydata",binary.byinviteplayer(),"Onlyunity",binary.unitonlyid());
    }
    redishandle->del("unitydata", binary.unitonlyid(), "unitybuild");
    redishandle->setSet("unitydata",QWORD(0),"del",binary.unitonlyid());
    User->m_unitybuild.NoticeUpdateUnityForAll(binary);


}

bool unitbuildCmdHandle::ReqAgreeUnitBuild(SceneUser* User,const HelloKittyMsgData::ReqAgreeUnitBuild *message)
{
    HelloKittyMsgData::UnitBuildResult result = HelloKittyMsgData::UnitBuildResult_Suc;
    DWORD param = 0;
    QWORD qwOnyID = RedisMgr::getMe().get_unitybuilddatabyColId(User->charid,message->colid());
    do{
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
        if(redishandle ==NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_OtherErr;
            break;
        }
        if(!redishandle->getLock("unitydata",qwOnyID,"lockop",1)) 
        {
            result = HelloKittyMsgData::UnitBuildResult_ServerBusy;
            break;
        }
        //没有这个合建栏
        HelloKittyMsgData::UnitRunInfo binary;
        if(!RedisMgr::getMe().get_unitybuildata(qwOnyID,binary))
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_ErrOnlyID;
            break;
        }
        //你不是被邀请方
        if(binary.byinviteplayer() != User->charid)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_NotByInvitePlayer;
            break;
        }
        //不处于邀请状态
        if(binary.state() != HelloKittyMsgData::UnitRunState_Invite)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_NotInviteState;
            break;

        }
        const pb::Conf_t_UniteGrid *pbuildgridConf = tbx::UniteGrid().get_base(binary.inviteunitgridid());
        if(pbuildgridConf == NULL)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_ErrGridId;
            break;
        }


        //同意的处理
        if(message->isagree())
        {
#if 0
            zMemDB* playerhandle = zMemDBPool::getMe().getMemDBHandle(binary.inviteplayer() % MAX_MEM_DB+1);
            if(playerhandle ==NULL ||!playerhandle->getLock("playerlock",binary.inviteplayer(),"newplayer",30))   
            {
                redishandle->delLock("unitydata",qwOnyID,"lockop");
                result = HelloKittyMsgData::UnitBuildResult_ServerBusy;
                break;
            }
#endif
            //你的资源不足
            if(binary.paytype() == HelloKittyMsgData::PayType_Other)
            {
                const pb::Conf_t_UniteBuild *pbuildConf = tbx::UniteBuild().get_base(binary.unitbuildid());
                if(pbuildConf == NULL)
                {
                    redishandle->delLock("unitydata",qwOnyID,"lockop");
                    result = HelloKittyMsgData::UnitBuildResult_ErrBuildId;
                    break;
                }
                std::map<DWORD,DWORD> materialMap;
                materialMap[pbuildConf->UniteBuild->pricetype()] = pbuildConf->UniteBuild->price();
                if(!User->checkMaterialMap(materialMap))
                { 
                    redishandle->delLock("unitydata",qwOnyID,"lockop");
                    result = HelloKittyMsgData::UnitBuildResult_NoResource;
                    param = pbuildConf->UniteBuild->pricetype();
                    break;
                }
                User->reduceMaterialMap(materialMap,"UnityBuild  reduce");


            }
            //如果对应栏位的有效期，大于合建有效期，更改合建有效期
            if(binary.timerout() > 0)
            {

                HelloKittyMsgData::UnitPlayerColId *pinfo =  User->m_unitybuild.getColById(binary.byinviteplayercolid());
                if(pinfo != NULL)
                {
                    if(pinfo->timerout() == 0 || pinfo->timerout() > binary.timerout())
                    {
                        binary.set_timerout(pinfo->timerout());
                    }
                }
            }
            //增加申请者合建次数，可能会导致对方栏位失效（离线，或在线处理）
            if(pbuildgridConf->UniteGrid->fieldsusecount() > 0)
            {
                UnityManager::AddBuildTimes(binary.inviteplayer(),binary.inviteplayercolid());
            }
            //开启运行状态
            binary.set_unitlastchecktimer(SceneTimeTick::currentTime.sec());
            binary.set_state(HelloKittyMsgData::UnitRunState_Running);
            Updateunitbuild(User,binary);
            HelloKittyMsgData::ReplaceWord  pParam ;
            pParam.set_key(HelloKittyMsgData::ReplaceType_NONE);
            pParam.set_value(User->charbase.nickname);
            std::vector<HelloKittyMsgData::ReplaceWord> vargs;
            vargs.push_back(pParam);
            MiscManager::getMe().SendSysNotice(binary.inviteplayer(),eSysNoticeId_InviteBuildSuc,vargs);


        }
        //不同意的处理,是否扣其他，待续
        else
        {
            //如果申请方支付，发邮件补偿
            if(binary.paytype() == HelloKittyMsgData::PayType_Self)
            {
                UnityManager::SendMailToInviteFalse(binary.inviteplayer(),binary.byinviteplayer(),binary.unitbuildid());
            }
            Delunitbuild(User,binary);
        }
        redishandle->delLock("unitydata",qwOnyID,"lockop"); 
    }while(0);
    ReturnResult(User,result,param);
    return true;
}

bool unitbuildCmdHandle::ReqStopBuild(SceneUser* User,const HelloKittyMsgData::ReqStopBuild *message)
{
    //不到一级不可以终止
    //UnitRunState_Running ,UnitRunState_RunningStop 这两个可以终止
    HelloKittyMsgData::UnitBuildResult result = HelloKittyMsgData::UnitBuildResult_Suc;
    QWORD qwOnyID = RedisMgr::getMe().get_unitybuilddatabyColId(User->charid,message->colid());
    do{
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
        if(redishandle ==NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_OtherErr;
            break;
        }
        if(!redishandle->getLock("unitydata",qwOnyID,"lockop",1)) 
        {
            result = HelloKittyMsgData::UnitBuildResult_ServerBusy;
            break;
        }
        //没有这个合建栏
        HelloKittyMsgData::UnitRunInfo binary;
        if(!RedisMgr::getMe().get_unitybuildata(qwOnyID,binary))
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_ErrOnlyID;
            break;
        }
        const pb::Conf_t_UniteGrid *pbuildgridConf = tbx::UniteGrid().get_base(binary.inviteunitgridid());
        if(pbuildgridConf == NULL)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_ErrGridId;
            break;
        }
        if(pbuildgridConf->UniteGrid->othercanstop() == 0)
        {
            if(binary.inviteplayer() != User->charid)
            {
                redishandle->delLock("unitydata",qwOnyID,"lockop"); 
                result = HelloKittyMsgData::UnitBuildResult_NOPower;
                break;

            }
        }
        else
        {
            if(binary.inviteplayer() != User->charid && binary.byinviteplayer() != User->charid)
            {
                redishandle->delLock("unitydata",qwOnyID,"lockop"); 
                result = HelloKittyMsgData::UnitBuildResult_NOPower;
                break;

            }
        }

        //判定状态
        if(binary.state() != HelloKittyMsgData::UnitRunState_Running && binary.state() != HelloKittyMsgData::UnitRunState_RunningStop)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_StopErrState;
            break;

        }
        if(binary.state() == HelloKittyMsgData::UnitRunState_Running && binary.unitlevel() == 0)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_LowLevel;
            break;

        }
        Delunitbuild(User,binary);
        User->m_unitybuild.sendmailforcancel(binary);
        redishandle->delLock("unitydata",qwOnyID,"lockop"); 
    }while(0);
    ReturnResult(User,result);
    return true;
}

bool unitbuildCmdHandle::ReqAddSpeedBuild(SceneUser* User,const HelloKittyMsgData::ReqAddSpeedBuild *message)
{
    QWORD qwOnyID = RedisMgr::getMe().get_unitybuilddatabyColId(User->charid,message->colid());
    DWORD param = 0;

    HelloKittyMsgData::UnitBuildResult result = HelloKittyMsgData::UnitBuildResult_Suc;
    do{
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
        if(redishandle ==NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_OtherErr;
            break;
        }
        if(!redishandle->getLock("unitydata",qwOnyID,"lockop",1)) 
        {
            result = HelloKittyMsgData::UnitBuildResult_ServerBusy;
            break;
        }
        //没有这个合建栏
        HelloKittyMsgData::UnitRunInfo binary;
        if(!RedisMgr::getMe().get_unitybuildata(qwOnyID,binary))
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_ErrOnlyID;
            break;
        }
        const pb::Conf_t_UniteGrid *pbuildgridConf = tbx::UniteGrid().get_base(binary.inviteunitgridid());
        if(pbuildgridConf == NULL)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_ErrGridId;
            break;
        }
        const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(binary.unitbuildid(),binary.unitlevel()));
        if(pbuildConf == NULL)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop");
            result = HelloKittyMsgData::UnitBuildResult_ErrBuildId;
            break;
        }

        if(binary.inviteplayer() != User->charid && binary.byinviteplayer() != User->charid)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_NOPower;
            break;

        }

        //判定状态,在运行中不能加速
        if(binary.state() != HelloKittyMsgData::UnitRunState_Running)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_NORunning;
            break;

        }
        //玩家加速次数不够，不能加速
        if(binary.inviteplayer() == User->charid)
        {
            if(binary.invitespeedtimes() >= pbuildgridConf->UniteGrid->gemspeedupnum())
            {
                redishandle->delLock("unitydata",qwOnyID,"lockop");
                result = HelloKittyMsgData::UnitBuildResult_MoreSpeedTimes;
                break;
            }


        }
        else 
        {
            if(binary.byinvitespeedtimes() >= pbuildgridConf->UniteGrid->gemspeedupnum())
            {
                redishandle->delLock("unitydata",qwOnyID,"lockop");
                result = HelloKittyMsgData::UnitBuildResult_MoreSpeedTimes;
                break;
            }

        }
        std::map<DWORD,DWORD> materialMap;
        materialMap[HelloKittyMsgData::Attr_Gem] = pbuildgridConf->UniteGrid->acccountnum();
        if(!User->checkMaterialMap(materialMap))
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop");
            result = HelloKittyMsgData::UnitBuildResult_NoResource;
            param  = HelloKittyMsgData::Attr_Gem;
            break;

        }
        //开始加速
        User->reduceMaterialMap(materialMap,"speed unitybuild by Gem");
        DWORD score  = pbuildgridConf->UniteGrid->growthtate();
        DWORD curscore = binary.unitscore();
        curscore += score; 
        DWORD nextscore = pbuildConf->UniteBuildlevel->nextlevelgrow();
        binary.set_unitscore(curscore);
        if(curscore >= nextscore) 
        {
            binary.set_state(HelloKittyMsgData::UnitRunState_RunningDone);
        }
        if(binary.inviteplayer() == User->charid)
        {
            binary.set_invitespeedtimes(binary.invitespeedtimes() +1);

        }
        else 
        {
            binary.set_byinvitespeedtimes(binary.byinvitespeedtimes() +1);
        }
        binary.set_lastspeedtimer(SceneTimeTick::currentTime.sec());
        Updateunitbuild(User,binary);
        redishandle->delLock("unitydata",qwOnyID,"lockop"); 
    }while(0);
    ReturnResult(User,result,param);
    return true;
}

bool unitbuildCmdHandle::ReqActiveBuild(SceneUser* User,const HelloKittyMsgData::ReqActiveBuild *message)
{
    //状态不对，不可激活
    //激活材料不足，不可激活
    HelloKittyMsgData::UnitBuildResult result = HelloKittyMsgData::UnitBuildResult_Suc;
    DWORD param = 0;
    QWORD qwOnyID = RedisMgr::getMe().get_unitybuilddatabyColId(User->charid,message->colid());

    do{
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
        if(redishandle ==NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_OtherErr;
            break;
        }
        if(!redishandle->getLock("unitydata",qwOnyID,"lockop",1)) 
        {
            result = HelloKittyMsgData::UnitBuildResult_ServerBusy;
            break;
        }
        //没有这个合建栏
        HelloKittyMsgData::UnitRunInfo binary;
        if(!RedisMgr::getMe().get_unitybuildata(qwOnyID,binary))
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_ErrOnlyID;
            break;
        }
        const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(binary.unitbuildid(),binary.unitlevel()));
        if(pbuildConf == NULL)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop");
            result = HelloKittyMsgData::UnitBuildResult_ErrBuildId;
            break;
        }
        const pb::Conf_t_UniteGrid *pbuildgridConf = tbx::UniteGrid().get_base(binary.inviteunitgridid());
        if(pbuildgridConf == NULL)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_ErrGridId;
            break;
        }
        if(binary.inviteplayer() != User->charid && binary.byinviteplayer() != User->charid)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_NOPower;
            break;

        }
        //判定状态,在运行中不能加速
        if(binary.state() != HelloKittyMsgData::UnitRunState_RunningDone)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_NotRunningDone;
            break;

        }
        if(!User->checkMaterialMap(pbuildConf->materialMap))
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop");
            result = HelloKittyMsgData::UnitBuildResult_NoResource;
            param = pbuildConf->materialMap.begin()->first;
            break;

        }
        QWORD otherplayer = binary.inviteplayer() == User->charid ? binary.byinviteplayer() : binary.inviteplayer();
        zMemDB* otherplayerhandle = zMemDBPool::getMe().getMemDBHandle(otherplayer);
        if(otherplayerhandle ==NULL ||!otherplayerhandle->getLock("playerlock",otherplayer,"newplayer",30))   
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop");
            result = HelloKittyMsgData::UnitBuildResult_ServerBusy;
            break;
        }
        //开始加速
        User->reduceMaterialMap(pbuildConf->materialMap,"active unitybuildlevel");
        //先升级
        binary.set_unitlevel(binary.unitlevel() +1);
        //1级给双方放建筑
        if(binary.unitlevel() == 1)
        {
            User->m_unitybuild.pushunitbuild(binary);
        }
        else
        {
            //否则 同步双方建筑等级

            User->m_unitybuild.synunitbuildlevel(binary);
        }
        bool full = false;
        const pb::Conf_t_UniteBuildlevel *pbuildConfNext = tbx::UniteBuildlevel().get_base(hashKey(binary.unitbuildid(),binary.unitlevel()));

        //如果到顶级，完成升级，取消合建
        if(pbuildConfNext ==NULL || binary.unitlevel() >= pbuildgridConf->UniteGrid->buildmaxlevel())
        {
            full = true;
        }
        if(full)
        {
            Updateunitbuild(User,binary);
            Delunitbuild(User,binary);
            User->m_unitybuild.sendmailforfinish(binary);

        }
        else
        {
            //否则 如果下一级也可升，状态不变
            if(binary.unitscore() < pbuildConfNext->UniteBuildlevel->nextlevelgrow())
            {
                //否则 如果超时，设置超时状态

                if(binary.timerout() != 0 && binary.timerout() < SceneTimeTick::currentTime.sec())
                {
                    binary.set_state(HelloKittyMsgData::UnitRunState_RunningStop);
                }
                else
                {
                    //否则 设置开始成长，重置检查时间
                    binary.set_state(HelloKittyMsgData::UnitRunState_Running);
                    binary.set_unitlastchecktimer(SceneTimeTick::currentTime.sec());
                }

            }
            Updateunitbuild(User,binary);
        }

        redishandle->delLock("unitydata",qwOnyID,"lockop"); 
    }while(0);
    ReturnResult(User,result,param);
    return true;
}

bool unitbuildCmdHandle::ReqCancelInvite(SceneUser* User,const HelloKittyMsgData::ReqCancelInvite *message)
{
    HelloKittyMsgData::UnitBuildResult result = HelloKittyMsgData::UnitBuildResult_Suc;
    QWORD qwOnyID = RedisMgr::getMe().get_unitybuilddatabyColId(User->charid,message->colid());
    do{
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
        if(redishandle ==NULL)
        {
            result = HelloKittyMsgData::UnitBuildResult_OtherErr;
            break;
        }
        if(!redishandle->getLock("unitydata",qwOnyID,"lockop",1)) 
        {
            result = HelloKittyMsgData::UnitBuildResult_ServerBusy;
            break;
        }
        //没有这个合建栏
        HelloKittyMsgData::UnitRunInfo binary;
        if(!RedisMgr::getMe().get_unitybuildata(qwOnyID,binary))
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_ErrOnlyID;
            break;
        }
        //你不是邀请方
        if(binary.inviteplayer() != User->charid)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_NotInvitePlayer;
            break;
        }
        //不处于邀请状态
        if(binary.state() != HelloKittyMsgData::UnitRunState_Invite)
        {
            redishandle->delLock("unitydata",qwOnyID,"lockop"); 
            result = HelloKittyMsgData::UnitBuildResult_NotInviteState;
            break;
        }

        //如果申请方支付，发邮件补偿
        if(binary.paytype() == HelloKittyMsgData::PayType_Self)
        {
            UnityManager::SendMailToInviteFalse(binary.inviteplayer(),binary.byinviteplayer(),binary.unitbuildid());
        }
        Delunitbuild(User,binary);
        redishandle->delLock("unitydata",qwOnyID,"lockop"); 
    }while(0);
    ReturnResult(User,result);
    return true;

}

void unitbuildCmdHandle::ReturnResult(SceneUser* User,HelloKittyMsgData::UnitBuildResult result,DWORD param)
{
    HelloKittyMsgData::ACKOpUnitBuild ack;
    std::string ret;
    ack.set_result(result);
    ack.set_param(param);
    encodeMessage(&ack,ret); 
    User->sendCmdToMe(ret.c_str(),ret.size()); 


}
