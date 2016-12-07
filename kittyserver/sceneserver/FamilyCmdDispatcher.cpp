#include "FamilyCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "extractProtoMsg.h"
#include "SceneToOtherManager.h"
#include "SceneTaskManager.h"
#include "RecordFamily.h"
#include "RedisMgr.h"
#include "RecordClient.h"
#include "Misc.h"
#include "dataManager.cpp"
#include "wordFilter.h"
#include "TimeTick.h"
#include "SceneMail.h"
void getFamilyOrderList(const CMD::RECORD::FamilyBase& m_base,std::vector<DWORD>& orderlist)
{
    std::vector<std::string> strVec;
    pb::parseTagString(m_base.m_orderlist,",",strVec);
    for(auto it = strVec.begin(); it != strVec.end(); it++)
    {
        DWORD orderId = atoi((*it).c_str());
        if(orderId > 0)
        {
            orderlist.push_back(orderId);
        }
    }

}


bool FamilyCmdHandle::ReqGetFamilyList(SceneUser* User,const HelloKittyMsgData::ReqGetFamilyList *message)
{
    HelloKittyMsgData::AckReqGetFamilyList ack;
    QWORD serchFamilyID = message->familyid();
    ack.set_familyid(serchFamilyID);
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc); 
    if(serchFamilyID > 0)
    {
        if(!RecordFamily::getMe().checkHasfamily(serchFamilyID))
        {
            HelloKittyMsgData::BaseFamilyInfo* pInfo  = ack.add_vecfamily();
            RecordFamily::getMe().readFamily(serchFamilyID,*pInfo,User->charid);
        }
    }
    else
    {
        std::set<QWORD> setApply;
        RecordFamily::getMe().getSelfApplyList(setApply,User->charid);
        for(auto iter = setApply.begin();iter != setApply.end(); iter++)
        {
            HelloKittyMsgData::BaseFamilyInfo* pInfo  = ack.add_vecfamily(); 
            RecordFamily::getMe().readFamily(*iter,*pInfo,User->charid);
        }
        if(setApply.size() < ParamManager::getMe().GetSingleParam(eParam_Family_recommend))
        {
            std::set<QWORD> resApply; 
            RecordFamily::getMe().getFamilyList(resApply,ParamManager::getMe().GetSingleParam(eParam_Family_recommend));
            for(auto it = resApply.begin();(it != resApply.end()) && (setApply.size() < ParamManager::getMe().GetSingleParam(eParam_Family_recommend)) ; it++)
            {
                if(setApply.insert(*it).second)
                {
                    HelloKittyMsgData::BaseFamilyInfo* pInfo  = ack.add_vecfamily(); 
                    RecordFamily::getMe().readFamily(*it,*pInfo,User->charid);

                }

            }

        }
    }
    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

void FamilyCmdHandle::DoReqAddFamily(SceneUser* User,const HelloKittyMsgData::ReqAddFamily *message,HelloKittyMsgData::AckReqAddFamily &ack,bool bIsGm)
{
    QWORD familyID = message->familyid();
    ack.set_familyid(familyID);
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    std::string strName; 
    do{
        if(!RecordFamily::getMe().checkHasfamily(familyID))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyNoExist);
            break;
        }
        CMD::RECORD::FamilyBase base;
        if(!RecordFamily::getMe().readFamily(familyID,base))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyNoExist);
            break;

        }
        const pb::Conf_t_familylevel *pfamilylevel = tbx::familylevel().get_base(base.m_level);
        if(pfamilylevel == NULL)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyNoExist);
            break;
        }

        if(RecordFamily::getMe().GetMemSize(familyID) >= pfamilylevel->familylevel->maxperson())
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyFull);
            break;
        }
        if(RecordFamily::getMe().getFamilyID(User->charid) > 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_HasFamily);
            break;
        }
        if(!bIsGm && !checkFamilyBulid(User))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamilyBuild);
            break;
        }

        ack.set_familyname(base.m_strName);
        if(base.m_limmitType >= HelloKittyMsgData::MAXPublicType)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OtherErr);
            break;

        }
        CharBase charbase;
        if(!RedisMgr::getMe().get_charbase(User->charid,charbase))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OtherErr);
            break;

        }
        if(base.m_lowLevel != 0 && base.m_lowLevel > charbase.level)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_JoinLevelErr);
            break;
        }
        if(base.m_highLevel != 0 && base.m_highLevel < charbase.level)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_JoinLevelErr);
            break;
        }
        zMemDB* handlechar = zMemDBPool::getMe().getMemDBHandle(User->charid);
        if(handlechar)
        {
            if(handlechar->isLock("playerlock",User->charid,"createfamily"))
            {
                ack.set_result(HelloKittyMsgData::FamilyOpResult_ServerBusy);
                break;
            }

        }
        HelloKittyMsgData::FamilyPublicType publicType = static_cast<HelloKittyMsgData::FamilyPublicType>(base.m_limmitType);
        switch(publicType)
        {
            case HelloKittyMsgData::PublicForAll:
                {
                    ack.set_result(RecordFamily::getMe().addMember(User->charid,familyID));
                    if(HelloKittyMsgData::FamilyOpResult_Suc == ack.result())
                    {
                        SceneMailManager::getMe().sendSysMailToPlayerJoinFamily(User->charid,User->charbase.nickname,base.m_strName);
                        SceneMailManager::getMe().sendSysMailToPlayerFriendJoinFamily(RecordFamily::getMe().getFamilyLeader(familyID),User->charbase.nickname);
                        UpdateFamily(familyID);
                    }


                }
                break;
            case HelloKittyMsgData::NoPublic:
                {
                    ack.set_result(HelloKittyMsgData::FamilyOpResult_NoPublic);

                }
                break;
            case HelloKittyMsgData::Needapply:
                {
                    if(RecordFamily::getMe().getSelfApplySize(User->charid) >= ParamManager::getMe().GetSingleParam(eParam_Family_ApplyMax))
                    {
                        ack.set_result(HelloKittyMsgData::FamilyOpResult_ApplyMax);
                        break;
                    }
                    ack.set_result(RecordFamily::getMe().addApply(User->charid,familyID));
                    //给族长一封邮件
                    if(HelloKittyMsgData::FamilyOpResult_Suc2 == ack.result())
                        SceneMailManager::getMe().sendSysMailToPlayerApplyFamily(RecordFamily::getMe().getFamilyLeader(familyID),User->charbase.nickname); 
                    UpdateFamily(familyID); 

                }
                break;
            default:
                ack.set_result(HelloKittyMsgData::FamilyOpResult_OtherErr);
                break;
        }
        TaskArgue arg(Target_InterActive,Attr_Family,Attr_Family,1);
        User->m_taskManager.target(arg);

    }while(0);


}

bool FamilyCmdHandle::ReqAddFamilyByGm(SceneUser* User,const HelloKittyMsgData::ReqAddFamily *message)
{
    HelloKittyMsgData::AckReqAddFamily ack;
    DoReqAddFamily(User,message,ack,true);
    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool FamilyCmdHandle::ReqAddFamily(SceneUser* User,const HelloKittyMsgData::ReqAddFamily *message)
{
    HelloKittyMsgData::AckReqAddFamily ack;
    DoReqAddFamily(User,message,ack,false);
    std::string ret;
    encodeMessage(&ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool FamilyCmdHandle::ReqCancelApply(SceneUser* User,const HelloKittyMsgData::ReqCancelApply *message)
{
    HelloKittyMsgData::AckReqCancelApply ack;
    ack.set_familyid(message->familyid());
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    do{
        if(!RecordFamily::getMe().checkPlayerIsApply(User->charid,message->familyid()))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoApply);  
            break;
        }
        ack.set_result(RecordFamily::getMe().removeApply(User->charid,message->familyid()));
        if(HelloKittyMsgData::FamilyOpResult_Suc == ack.result())
        {
            UpdateFamily(message->familyid());
        }
    }while(0);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}


HelloKittyMsgData::FamilyOpResult   FamilyCmdHandle::checkFamilyName(const string & strName)
{
    if(strName.size() > ParamManager::getMe().GetSingleParam(eParam_Family_NameMax))
    {
        return HelloKittyMsgData::FamilyOpResult_NameTooLong;
    }
    if(strName.size() < ParamManager::getMe().GetSingleParam(eParam_Family_NameMin))
    {
        return HelloKittyMsgData::FamilyOpResult_NameTooShort;
    }
    if(wordFilter::getMe().hasForbitWord(strName.c_str()))
    {
        return HelloKittyMsgData::FamilyOpResult_Nameillegal;
    }
    return HelloKittyMsgData::FamilyOpResult_Suc;
}

HelloKittyMsgData::FamilyOpResult   FamilyCmdHandle::checkFamilyNotice(const string & strNotice)
{   
    if(strNotice.size() > ParamManager::getMe().GetSingleParam(eParam_Family_NoticeMax))
    {
        return HelloKittyMsgData::FamilyOpResult_NoticeToLong;
    }
    wordFilter::getMe().doFilter(const_cast<string &>(strNotice));
    return HelloKittyMsgData::FamilyOpResult_Suc; 
}

HelloKittyMsgData::FamilyOpResult FamilyCmdHandle::DoCreateFamily(SceneUser* User,const HelloKittyMsgData::ReqCreateFamily *message,bool bIsGm)
{
    if(RecordFamily::getMe().getFamilyID(User->charid) > 0)
    {
        return HelloKittyMsgData::FamilyOpResult_HasFamily;
    }
    if(!bIsGm && !checkFamilyBulid(User))
    {
        return HelloKittyMsgData::FamilyOpResult_NoFamilyBuild;
    }
    if(message->familyicon() == 0)
    {
        return  HelloKittyMsgData::FamilyOpResult_NoIcon;
    }
    if(message->pulictype() >= HelloKittyMsgData::MAXPublicType)
    {
        return HelloKittyMsgData::FamilyOpResult_PublicSetErr;
    }
    if(message->lowlevel() != 0 && message->highlevel() != 0 && message->lowlevel() > message->highlevel())
    {
        return  HelloKittyMsgData::FamilyOpResult_LevelLimitErr;
    }
    HelloKittyMsgData::FamilyOpResult result = checkFamilyName(message->familyname());
    if(result != HelloKittyMsgData::FamilyOpResult_Suc)
    {
        return result; 
    }
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(User->charid);
    if(handle)
    {
        if(!handle->getLock("playerlock",User->charid,"createfamily",30))
        {
            return HelloKittyMsgData::FamilyOpResult_ServerBusy;
        }

    }
    if(!bIsGm && !delCreateSource(User))
    {
        if(handle)
        {
            handle->delLock("playerlock",User->charid,"createfamily");
        }
        return HelloKittyMsgData::FamilyOpResult_SourceLimit;
    }
    return HelloKittyMsgData::FamilyOpResult_Suc;



}

bool FamilyCmdHandle::ReqCreateFamily(SceneUser* User,const HelloKittyMsgData::ReqCreateFamily *message)
{
    HelloKittyMsgData::FamilyOpResult result = HelloKittyMsgData::FamilyOpResult_Suc;
    RecordClient *recordClient = MgrrecordClient.GetRecordByTableName("t_family"); 

    do{
        if(NULL == recordClient)
        {
            result = HelloKittyMsgData::FamilyOpResult_OtherErr;
            break;
        }
        result = DoCreateFamily(User,message,false);

    }while(0);
    if(result != HelloKittyMsgData::FamilyOpResult_Suc)
    {
        HelloKittyMsgData::AckReqCreateFamily ack;
        ack.set_result(result);
        std::string ret;
        encodeMessage(&ack,ret);
        User->sendCmdToMe(ret.c_str(),ret.size());
    }
    else
    {
        CMD::RECORD::t_WriteFamily_SceneRecord cmd;
        cmd.m_base.m_charid =  User->charid;
        sprintf(cmd.m_base.m_strName,message->familyname().c_str(),message->familyname().size());
        cmd.m_base.m_icon = message->familyicon();
        cmd.m_base.m_limmitType = message->pulictype();
        cmd.m_base.m_lowLevel = message->lowlevel();
        cmd.m_base.m_highLevel =message->highlevel();
        std::string ret;
        encodeMessage(&cmd,sizeof(cmd),ret); 
        recordClient->sendCmd(ret.c_str(),ret.size());

    }
    return true;

}

bool FamilyCmdHandle::ReqCreateFamilyByGm(SceneUser* User,const HelloKittyMsgData::ReqCreateFamily *message)
{
    HelloKittyMsgData::FamilyOpResult result = HelloKittyMsgData::FamilyOpResult_Suc;
    RecordClient *recordClient = MgrrecordClient.GetRecordByTableName("t_family"); 

    do{
        if(NULL == recordClient)
        {
            result = HelloKittyMsgData::FamilyOpResult_OtherErr;
            break;
        }
        result = DoCreateFamily(User,message,true);

    }while(0);
    if(result != HelloKittyMsgData::FamilyOpResult_Suc)
    {
        HelloKittyMsgData::AckReqCreateFamily ack;
        ack.set_result(result);
        std::string ret;
        encodeMessage(&ack,ret);
        User->sendCmdToMe(ret.c_str(),ret.size());
    }
    else
    {
        CMD::RECORD::t_WriteFamily_SceneRecord cmd;
        cmd.m_base.m_charid =  User->charid;
        sprintf(cmd.m_base.m_strName,message->familyname().c_str(),message->familyname().size());
        cmd.m_base.m_icon = message->familyicon();
        cmd.m_base.m_limmitType = message->pulictype();
        cmd.m_base.m_lowLevel = message->lowlevel();
        cmd.m_base.m_highLevel =message->highlevel();
        std::string ret;
        encodeMessage(&cmd,sizeof(cmd),ret); 
        recordClient->sendCmd(ret.c_str(),ret.size());

    }
    return true;

}



void FamilyCmdHandle::DocreateReturn(const CMD::RECORD::t_WriteFamily_RecordScene_Create_Return *cmd)
{
    //释放创建锁
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->charid);
    if(handle)
    {
        handle->delLock("playerlock",cmd->charid,"createfamily");
    }
    SceneUser* User = SceneUserManager::getMe().getUserByID(cmd->charid);
    if(!User)
    {
        LOG_ERR("Can not Find player for  create Family player :%lu ret :%d",cmd->charid,cmd->ret);
        return;

    }
    HelloKittyMsgData::AckReqCreateFamily ack; 
    ack.set_result(cmd->ret == 0 ? HelloKittyMsgData::FamilyOpResult_Suc:HelloKittyMsgData::FamilyOpResult_OtherErr);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());


    if(cmd->ret >0 )//返回资源
    {
        returnCreateSource(User);
    }
    else
    {
        RecordFamily::getMe().removeApply(cmd->charid);
        UpdateFamily(RecordFamily::getMe().getFamilyID(cmd->charid));

    }

}

bool FamilyCmdHandle::delCreateSource(SceneUser* User)
{
    std::vector<DWORD> vecRes = ParamManager::getMe().GetVecParam(eParam_Family_CreateRes);
    if(vecRes.size() == 2)
    {
        return User->m_store_house.addOrConsumeItem(vecRes[0],vecRes[1],"新建家族扣除",false);
    }
    return true;
}

void FamilyCmdHandle::returnCreateSource(SceneUser* User)
{
    std::vector<DWORD> vecRes = ParamManager::getMe().GetVecParam(eParam_Family_CreateRes);
    if(vecRes.size() == 2)
    {
        User->m_store_house.addOrConsumeItem(vecRes[0],vecRes[1],"新建家族失败返还",true);
    }

}

bool FamilyCmdHandle::ReqAgreeJoin(SceneUser* User,const HelloKittyMsgData::ReqAgreeJoin *message)
{
    HelloKittyMsgData::AckReqAgreeJoin ack;
    ack.set_charid(message->charid());
    ack.set_isagree(message->isagree());
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    do{
        QWORD familyID = RecordFamily::getMe().getFamilyID(User->charid);
        if(familyID == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoLeader);
            break;
        }
        if(RecordFamily::getMe().getFamilyLeader(familyID) != User->charid)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoLeader);
            break;
        }
        if(!RecordFamily::getMe().checkPlayerIsApply(message->charid(),familyID))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoApply);  
            break;
        }
        CMD::RECORD::FamilyBase base;
        if(!RecordFamily::getMe().readFamily(familyID,base))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyNoExist);
            break;

        }
        if(message->isagree() == 0)
        {
            ack.set_result(RecordFamily::getMe().removeApply(message->charid(),familyID));
            if(HelloKittyMsgData::FamilyOpResult_Suc == ack.result())
            {
                SceneMailManager::getMe().sendSysMailToPlayerRefuseJoin(message->charid(),base.m_strName);
                UpdateFamily(familyID);

            }

        }
        else
        {

            const pb::Conf_t_familylevel *pfamilylevel = tbx::familylevel().get_base(base.m_level);
            if(pfamilylevel == NULL)
            {
                ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyNoExist);
                break;
            }

            if(RecordFamily::getMe().GetMemSize(familyID) >= pfamilylevel->familylevel->maxperson())
            {
                ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyFull);
                break;
            }
            ack.set_result(RecordFamily::getMe().addMember(message->charid(),familyID)); 
            if(HelloKittyMsgData::FamilyOpResult_Suc == ack.result())
            {
                zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(message->charid());
                if(handleTemp)
                {
                    std::string nickname = std::string(handleTemp->get("rolebaseinfo",message->charid(),"nickname"));
                    SceneMailManager::getMe().sendSysMailToPlayerJoinFamily(message->charid(),nickname,base.m_strName);
                    SceneMailManager::getMe().sendSysMailToPlayerFriendJoinFamily(User->charid,nickname);
                    UpdateFamily(familyID);

                }
            }


        }


    }while(0);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}
void FamilyCmdHandle::UpdateFamily(QWORD familyID)
{
    HelloKittyMsgData::AckSelfFamilyInfo ack;
    HelloKittyMsgData::FamilyInfo* pselfinfo = ack.mutable_selfinfo();
    if(pselfinfo ==NULL)
        return ;
    CMD::RECORD::FamilyBase m_base;
    if(!RecordFamily::getMe().readFamily(familyID,m_base))
    {
        return ;

    }
    std::set<QWORD> setMember;
    RecordFamily::getMe().getMember(setMember,m_base.m_familyID);
    for(auto iter = setMember.begin(); iter != setMember.end();iter++)
    {
        HelloKittyMsgData::FamilyMember* pMember = pselfinfo->add_vecmember();
        if(!pMember)
        {
            continue;
        }
        HelloKittyMsgData::playerShowbase* pbase = pMember->mutable_playershow();
        if(!pbase)
        {
            continue;
        }
        if(!SceneUser::getplayershowbase(*iter,*pbase))
        {
            continue;
        }
        CMD::RECORD::FamilyMemberData data ; 
        if(!RecordFamily::getMe().readFamilyMemberBin(*iter,data))
        {
            continue;
        }
        pMember->set_todaycontribution(data.m_contribution);
        pMember->set_contributionlastrank(data.m_contributionranklast);
        pMember->set_contributionlast(data.m_contributionlast);
        pMember->set_job(*iter == RecordFamily::getMe().getFamilyLeader(m_base.m_familyID) ? HelloKittyMsgData::Family_Leader : HelloKittyMsgData::Family_Member); 

    }
    std::set<QWORD> setApply;
    RecordFamily::getMe().getApplyPlayer(setApply,m_base.m_familyID);
    for(auto iter = setApply.begin(); iter != setApply.end();iter++)
    {
        HelloKittyMsgData::FamilyMember* pMember = pselfinfo->add_vecappy();
        if(!pMember)
        {
            continue;
        }
        HelloKittyMsgData::playerShowbase* pbase = pMember->mutable_playershow();
        if(!pbase)
        {
            continue;
        }
        if(!SceneUser::getplayershowbase(*iter,*pbase))
        {
            continue;
        }

    }
    pselfinfo->set_totalcontributionlast(m_base.m_contributionlast);
    std::vector<DWORD> vecOrder;
    getFamilyOrderList(m_base,vecOrder);
    for(auto it = vecOrder.begin();it != vecOrder.end();it++)
    {
        pselfinfo->add_vecorder(*it);
    }

    for(auto it = setMember.begin(); it != setMember.end();it++)
    {
        HelloKittyMsgData::BaseFamilyInfo* pBase = pselfinfo->mutable_baseinfo();
        if(!pBase)
        {
            continue ;
        }
        CMD::RECORD::FamilyMemberData data ; 
        if(!RecordFamily::getMe().readFamilyMemberBin(*it,data))  
        {
            continue;
        }
        pselfinfo->set_selfcontributionlastrank(data.m_contributionranklast);
        pselfinfo->set_selfcontributionlast(data.m_contributionlast);
        pselfinfo->set_selfisgetaward(data.m_isgetaward);
        RecordFamily::getMe().trans(m_base,*pBase,*it);
        std::string ret;    
        encodeMessage(&ack,ret);  
        SceneTaskManager::getMe().broadcastUserCmdToGateway(*it,ret.c_str(),ret.size());

    }



}

void FamilyCmdHandle::NoticeKick(QWORD charid)
{
    HelloKittyMsgData::AcKKickFamily ack;
    std::string ret; 
    encodeMessage(&ack,ret); 
    SceneTaskManager::getMe().broadcastUserCmdToGateway(charid,ret.c_str(),ret.size());
}

bool FamilyCmdHandle::ReqselfFamilyInfo(SceneUser* User,const HelloKittyMsgData::ReqselfFamilyInfo *message)
{

    HelloKittyMsgData::AckReqselfFamilyInfo ack;
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    do{
        QWORD familyID = RecordFamily::getMe().getFamilyID(User->charid);
        if(familyID == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        HelloKittyMsgData::FamilyInfo* pselfinfo = ack.mutable_selfinfo();
        if(!pselfinfo)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OtherErr);
            break;
        }
        HelloKittyMsgData::BaseFamilyInfo* pBase = pselfinfo->mutable_baseinfo();
        if(!pBase)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OtherErr);
            break;
        }
        CMD::RECORD::FamilyBase m_base;

        if(!RecordFamily::getMe().readFamily(familyID,m_base))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OtherErr);
            break;
        }
        RecordFamily::getMe().trans(m_base,*pBase,User->charid);
        pselfinfo->set_totalcontributionlast(m_base.m_contributionlast);
        std::vector<DWORD> vecOrder;
        getFamilyOrderList(m_base,vecOrder);
        for(auto it = vecOrder.begin();it != vecOrder.end();it++)
        {
            pselfinfo->add_vecorder(*it);
        }
        std::set<QWORD> setMember;

        RecordFamily::getMe().getMember(setMember,familyID);
        for(auto iter = setMember.begin(); iter != setMember.end();iter++)
        {
            HelloKittyMsgData::FamilyMember* pMember = pselfinfo->add_vecmember();
            if(!pMember)
            {
                continue;
            }
            HelloKittyMsgData::playerShowbase* pbase = pMember->mutable_playershow();
            if(!pbase)
            {
                continue;
            }
            if(!SceneUser::getplayershowbase(*iter,*pbase))
            {
                continue;
            }
            CMD::RECORD::FamilyMemberData data ; 
            if(!RecordFamily::getMe().readFamilyMemberBin(*iter,data))
            {
                continue;
            }
            pMember->set_todaycontribution(data.m_contribution);
            pMember->set_contributionlastrank(data.m_contributionranklast);
            pMember->set_contributionlast(data.m_contributionlast);
            if(*iter == User->charid)
            {
                pselfinfo->set_selfcontributionlastrank(data.m_contributionranklast);
                pselfinfo->set_selfcontributionlast(data.m_contributionlast);
                pselfinfo->set_selfisgetaward(data.m_isgetaward);
            }
            pMember->set_job(*iter == RecordFamily::getMe().getFamilyLeader(familyID) ? HelloKittyMsgData::Family_Leader : HelloKittyMsgData::Family_Member); 

        }

        std::set<QWORD> setApply;
        RecordFamily::getMe().getApplyPlayer(setApply,familyID);
        for(auto iter = setApply.begin(); iter != setApply.end();iter++)
        {
            HelloKittyMsgData::FamilyMember* pMember = pselfinfo->add_vecappy();
            if(!pMember)
            {
                continue;
            }
            HelloKittyMsgData::playerShowbase* pbase = pMember->mutable_playershow();
            if(!pbase)
            {
                continue;
            }
            if(!SceneUser::getplayershowbase(*iter,*pbase))
            {
                continue;
            }

        }


    }while(0);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}


bool FamilyCmdHandle::ReqQuitFamily(SceneUser* User,const HelloKittyMsgData::ReqQuitFamily *message)
{
    HelloKittyMsgData::AckReqQuitFamily ack;
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    do{
        QWORD familyID = RecordFamily::getMe().getFamilyID(User->charid);
        if(familyID == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        CMD::RECORD::FamilyBase m_base;
        if(!RecordFamily::getMe().readFamily(familyID,m_base))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        if(RecordFamily::getMe().GetMemSize(familyID) > 1)//家族人数大于1，族长不能退
        {

            if(RecordFamily::getMe().getFamilyLeader(familyID) == User->charid)
            {
                ack.set_result(HelloKittyMsgData::FamilyOpResult_LeaderCanNotQuit);
            }
            else
            {
                ack.set_result(RecordFamily::getMe().removeMember(User->charid,familyID));
            }
            if(HelloKittyMsgData::FamilyOpResult_Suc == ack.result())
            {
                SceneMailManager::getMe().sendSysMailToPlayerLeaveFamily(RecordFamily::getMe().getFamilyLeader(familyID),User->charbase.nickname);
                UpdateFamily(familyID);
            }

        }
        else
        {
            ack.set_result(RecordFamily::getMe().removeMember(User->charid,familyID));
            if(RecordFamily::getMe().GetMemSize(familyID) == 0)
            {
                RecordFamily::getMe().doDel(familyID);
            }


        }
        if(HelloKittyMsgData::FamilyOpResult_Suc == ack.result())
            SceneMailManager::getMe().sendSysMailToPlayerQuitFamily(User->charid,m_base.m_strName);

    }while(0);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool FamilyCmdHandle::ReqUpdateOtherFamilyJob(SceneUser* User,const HelloKittyMsgData::ReqUpdateOtherFamilyJob *message)
{
    HelloKittyMsgData::AckUpdateOtherFamilyJob ack;
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    do{
        QWORD familyID = RecordFamily::getMe().getFamilyID(User->charid);
        if(familyID == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        if(RecordFamily::getMe().getFamilyLeader(familyID) != User->charid)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoLeader);
            break;
        }
        if(User->charid == message->charid())
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OpSelf);
            break;

        }
        if(RecordFamily::getMe().getFamilyID(message->charid()) != familyID)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        CMD::RECORD::FamilyBase base;
        if(!RecordFamily::getMe().readFamily(familyID,base))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyNoExist);
            break;

        }
        base.m_charid = message->charid();
        RecordFamily::getMe().update(base);
        UpdateFamily(familyID);

    }while(0);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());

    return true;
}

bool FamilyCmdHandle::ReqKickFamilyMember(SceneUser* User,const HelloKittyMsgData::ReqKickFamilyMember *message)
{
    HelloKittyMsgData::AckReqKickFamilyMember ack;
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    do{
        QWORD familyID = RecordFamily::getMe().getFamilyID(User->charid);
        if(familyID == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        if(RecordFamily::getMe().getFamilyLeader(familyID) != User->charid)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoLeader);
            break;
        }
        if(User->charid == message->charid())
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OpSelf);
            break;

        }
        if(RecordFamily::getMe().getFamilyID(message->charid()) != familyID)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        ack.set_result(RecordFamily::getMe().removeMember(message->charid(),familyID));
        if(HelloKittyMsgData::FamilyOpResult_Suc == ack.result())
        {
            SceneMailManager::getMe().sendSysMailToPlayerKickFamily(message->charid());
            UpdateFamily(familyID);
            NoticeKick(message->charid());

        }

    }while(0);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool FamilyCmdHandle::ReqSetFamilyBaseInfo(SceneUser* User,const HelloKittyMsgData::ReqSetFamilyBaseInfo *message)
{
    HelloKittyMsgData::AckReqSetFamilyBaseInfo ack;
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    do{
        QWORD familyID = RecordFamily::getMe().getFamilyID(User->charid);
        if(familyID == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        if(RecordFamily::getMe().getFamilyLeader(familyID) != User->charid)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoLeader);
            break;
        }
        if(message->familyicon() == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoIcon);
            break;
        }
        if(message->pulictype() >= HelloKittyMsgData::MAXPublicType)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_PublicSetErr);
            break;
        }
        if(message->lowlevel() != 0 && message->highlevel() != 0 && message->lowlevel() > message->highlevel())
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_LevelLimitErr);
            break;
        }
        ack.set_result(checkFamilyNotice(message->familynotice()));
        if(ack.result() != HelloKittyMsgData::FamilyOpResult_Suc)
        {
            break;
        }
        CMD::RECORD::FamilyBase base;
        if(!RecordFamily::getMe().readFamily(familyID,base))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyNoExist);
            break;

        }
        base.m_icon = message->familyicon();
        base.m_limmitType = message->pulictype();
        base.m_lowLevel = message->lowlevel();
        base.m_highLevel =message->highlevel();
        sprintf(base.m_notice,message->familynotice().c_str(),message->familynotice().size());
        RecordFamily::getMe().update(base);
        UpdateFamily(familyID);

    }while(0);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}


bool FamilyCmdHandle::ReqFamilyRanking(SceneUser* User,const HelloKittyMsgData::ReqFamilyRanking *message)
{
    HelloKittyMsgData::AckReqFamilyRanking ack; 
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    do{//TODO:
        QWORD familyID = RecordFamily::getMe().getFamilyID(User->charid);
        if(familyID == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        CMD::RECORD::FamilyBase base;
        if(!RecordFamily::getMe().readFamily(familyID,base))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyNoExist);
            break;

        }
        zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle();
        if(!handle2)
            break;
        //1- 10名
        for(int i = 1; i <= int(ParamManager::getMe().GetSingleParam(eParam_Family_TopLen)) ;i++)
        {
            QWORD  fID =  handle2->getInt("family",i,"Rank");
            if(fID > 0)
            {
                HelloKittyMsgData::BaseFamilyInfo* pInfo  = ack.add_vecfamily(); 
                RecordFamily::getMe().readFamily(fID,*pInfo,User->charid);
            }
        }
        //自己前两名
        if(base.m_ranking == 0)
        {
            HelloKittyMsgData::BaseFamilyInfo* pInfo  = ack.add_vecfamily(); 
            RecordFamily::getMe().readFamily(familyID,*pInfo,User->charid);

        }
        else 
        {
            int nowRank = base.m_ranking;
            for(int rank = nowRank -ParamManager::getMe().GetSingleParam(eParam_Family_NearRank); rank <= int(nowRank +ParamManager::getMe().GetSingleParam(eParam_Family_NearRank)) ;rank++)
            {
                if(rank <= int(ParamManager::getMe().GetSingleParam(eParam_Family_TopLen)))
                {
                    continue;
                }
                QWORD  fID =  handle2->getInt("family",rank,"Rank");
                if(fID > 0)
                {
                    HelloKittyMsgData::BaseFamilyInfo* pInfo  = ack.add_vecfamily(); 
                    RecordFamily::getMe().readFamily(fID,*pInfo,User->charid);
                }

            }
        }
        //自己后两名
    }while(0);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;



}

bool FamilyCmdHandle::checkFamilyBulid(SceneUser* User)
{
    return User->m_buildManager.getAnyBuildById(ParamManager::getMe().GetSingleParam(eParam_Family_CreateNeedBulid)) > 0 ;

}

bool FamilyCmdHandle::ReqFinishFamilyOrder(SceneUser* User,const HelloKittyMsgData::ReqFinishFamilyOrder *message)
{
    HelloKittyMsgData::AckFinishFamilyOrder ack;
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    ack.set_orderid(message->orderid());
    do{
        QWORD familyID = RecordFamily::getMe().getFamilyID(User->charid);
        if(familyID == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        //订单数太多
        if(User->charbin.dailydata().familyordernum() >= ParamManager::getMe().GetSingleParam(eParam_Family_OrderNum))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_TooMoreOrder);
            break;
        }

        CMD::RECORD::FamilyBase base;
        if(!RecordFamily::getMe().readFamily(familyID,base))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyNoExist);
            break;

        }
        std::vector<DWORD> orderlist;
        getFamilyOrderList(base,orderlist);
        bool bfind = false;
        for(auto it = orderlist.begin();it != orderlist.end();it++)
        {
            if(*it == message->orderid())
            {
                bfind = true;
                break;
            }
        }
        if(!bfind)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OrderErr);
            break;
        }
        const pb::Conf_t_familyorder *pConf = tbx::familyorder().get_base(message->orderid());
        if(!pConf)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OrderErr);
            break;
        }
        HelloKittyMsgData::vecAward awarditem;
        const HelloKittyMsgData::Award* pconfaward = pConf->getRandAward();
        if(pconfaward)
        {
            HelloKittyMsgData::Award* pAward = awarditem.add_award();
            if(pAward)
                *pAward = *pconfaward;
        }

        if(!User->checkPush(awarditem))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OrderPacket); 
            break;
        }

        std::map<DWORD,DWORD> materialMap;
        for(auto i = 0; i != pConf->needitem.award_size(); i++)
        {
            materialMap[pConf->needitem.award(i).awardtype()] = pConf->needitem.award(i).awardval();
        }
        //查物
        if(!User->checkMaterialMap(materialMap))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OrderSource); 
            break;
        }
        User->reduceMaterialMap(materialMap,"familyorder use");
        User->pushItem(awarditem,"familyorder award");
        CMD::RECORD::FamilyMemberData data ; 
        if(RecordFamily::getMe().readFamilyMemberBin(User->charid,data))
        {
            data.m_contribution += pConf->familyorder->score();
            //todo update  CMD::RECORD::FamilyMemberData data 
            RecordFamily::getMe().updateMember(familyID,User->charid,data);

        }
        for(int i =0 ; i != awarditem.award_size();i++)
        {
            const HelloKittyMsgData::Award& raward = awarditem.award(i);
            HelloKittyMsgData::Award* packaward =  ack.add_award();
            if(packaward)
                *packaward = raward;

        }
        User->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Train_Order_Number,1);
        MiscManager::getMe().getAdditionalRewards(User,AdditionalType_FamilyOerder);
        HelloKittyMsgData::DailyData *dailyData = User->charbin.mutable_dailydata();
        if(dailyData)
        {
            dailyData->set_familyordernum(dailyData->familyordernum() +1);
            ack.set_familyordernum(dailyData->familyordernum());
        }
        UpdateFamily(familyID);




    }while(0);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;


}

bool FamilyCmdHandle::ReqGetlastFamilyAward(SceneUser* User,const HelloKittyMsgData::ReqGetlastFamilyAward *message)
{
    HelloKittyMsgData::AckGetlastFamilyAward ack;
    ack.set_result(HelloKittyMsgData::FamilyOpResult_Suc);
    do{
        QWORD familyID = RecordFamily::getMe().getFamilyID(User->charid);
        if(familyID == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;
        }
        CMD::RECORD::FamilyMemberData data ; 
        if(!RecordFamily::getMe().readFamilyMemberBin(User->charid,data))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_NoFamily);
            break;

        }
        if(data.m_contributionlast == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_noaward);
            break;
        }
        if(data.m_isgetaward > 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_awarddone);
            break;
        }
        CMD::RECORD::FamilyBase base;
        if(!RecordFamily::getMe().readFamily(familyID,base))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_FamilyNoExist);
            break;

        }
        if(base.m_contributionlast == 0)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OtherErr);
            break;

        }
        //全员奖励
        QWORD key = pb::Conf_t_familyscore::getKeybyScore(base.m_contributionlast);
        const pb::Conf_t_familyscore *pConf = tbx::familyscore().get_base(key);
        if(pConf == NULL)
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OtherErr);
            break;
        }
        HelloKittyMsgData::vecAward allawarditem = pConf->awarditem;
        //个人排名奖励   
        HelloKittyMsgData::vecAward selfAward;
        const pb::Conf_t_familypersonscore *pRankConf = tbx::familypersonscore().get_base(data.m_contributionranklast);
        if(pRankConf)
        {
            selfAward  = pRankConf->awarditem;
        }
        //合并
        HelloKittyMsgData::vecAward allselfitem = allawarditem;
        for(int i= 0; i != selfAward.award_size();i++)
        {
            HelloKittyMsgData::Award* pallaward =  allselfitem.add_award();
            if(pallaward)
                *pallaward = selfAward.award(i);
        }

        if(!User->checkPush(allselfitem))
        {
            ack.set_result(HelloKittyMsgData::FamilyOpResult_OrderPacket); 
            break;
        }
        User->pushItem(allselfitem,"GetlastFamilyAward award");
        data.m_isgetaward = 1;
        RecordFamily::getMe().updateMember(familyID,User->charid,data);
        for(int i =0 ; i != allawarditem.award_size();i++)
        {
            const HelloKittyMsgData::Award& raward = allawarditem.award(i);
            HelloKittyMsgData::Award* packaward =  ack.add_allaward();
            if(packaward)
                *packaward = raward;

        }
        for(int i =0 ; i != selfAward.award_size();i++)
        {
            const HelloKittyMsgData::Award& raward = selfAward.award(i);
            HelloKittyMsgData::Award* packaward =  ack.add_selfaward();
            if(packaward)
                *packaward = raward;

        }

    }while(0);
    std::string ret;    
    encodeMessage(&ack,ret);  
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;


}

