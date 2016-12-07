#include "SceneUser.h"
#include "divine.pb.h"
#include "tbx.h"
#include "key.h"
#include "common.pb.h"
#include "zMemDB.h"
#include "zMemDBPool.h"
#include "SceneCommand.h"
#include <string.h>
#include "SceneCommand.h"
#include "SceneTaskManager.h"
#include "CharBase.h"
#include "TimeTick.h"
#include "Misc.h"
#include "guide.pb.h"
#include "SceneUserManager.h"
#include "SceneToOtherManager.h"
#include "SceneServer.h"
#include "resource.pb.h"
#include "RecordFamily.h"


void SceneUser::fillOtherInfo(HelloKittyMsgData::OtherInfo *other,const QWORD charID)
{
    if(other)
    {
        other->set_likecnt(getLikeCnt());
        other->set_islike(isLike(charID));
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
        if(handle)
        {
            other->set_isonline(handle->checkSet("playerset",0,"online",charid));
        }
        other->set_isatten(m_friend.IsFriend(charID));
        other->set_isbuywechat(isBuyWeChat(charID));
        other->set_lastonlinetime(charbase.onlinetime);
        m_leavemessage.fillMessage(other);
    }
}

void SceneUser::ackOtherInfo()
{

    for(auto iter = m_seeRoomSet.begin();iter != m_seeRoomSet.end();++iter)
    {
        QWORD charID = *iter;
        HelloKittyMsgData::AckOtherInfo ackMsg;
        HelloKittyMsgData::OtherInfo *other = ackMsg.mutable_otherinfo();
        fillOtherInfo(other,charID);
        ackMsg.set_charid(charid);

        std::string ret;
        encodeMessage(&ackMsg,ret);
        SceneTaskManager::getMe().broadcastUserCmdToGateway(charID,ret.c_str(),ret.size());
    }

    HelloKittyMsgData::AckOtherInfo ackMsg;
    HelloKittyMsgData::OtherInfo *other = ackMsg.mutable_otherinfo();
    fillOtherInfo(other,0);
    ackMsg.set_charid(charid);

    std::string ret;
    encodeMessage(&ackMsg,ret);
    sendCmdToMe(ret.c_str(),ret.size());
}

bool SceneUser::outRoom(const QWORD charID)
{
    m_seeRoomSet.erase(charID);
    return true;
}

bool SceneUser::ackPersonalInfo(const QWORD charID,const bool isView)
{
    HelloKittyMsgData::AckPersonalInfo ack;
    ack.set_charid(charid);
    HelloKittyMsgData::PersonalInfo *personalInfo = ack.mutable_personalinfo();
    if(personalInfo)
    {
        *personalInfo = m_personInfo;
    }
    std::string bar = (isView || charID == charid) ? m_personInfo.wechat() : "*******";
    personalInfo->set_wechat(bar);
        
    
    HelloKittyMsgData::OtherInfo *other = ack.mutable_otherinfo();
    fillOtherInfo(other,charID);

    std::string ret;
    encodeMessage(&ack,ret);
    return charID == charid ? sendCmdToMe(ret.c_str(),ret.size()) : SceneTaskManager::getMe().broadcastUserCmdToGateway(charID,ret.c_str(),ret.size());
}

bool SceneUser::modifyPresent(const std::string &present)
{
    m_personInfo.set_present(present);
    updatePresent();
    return true;
}

bool SceneUser::modifyVoice(const HelloKittyMsgData::ReqModifyVoice *message)
{
    HelloKittyMsgData::VociceData *voice = m_personInfo.mutable_voice();
    if(voice)
    {
        *voice = message->voice();
    }
    updateVoice();
    return true;
}

bool SceneUser::modifyPersonalInfo(const HelloKittyMsgData::ReqEditPersonInalInfo *message)
{
    bool ret = false;
    do
    {
#if 0
        if(!tbx::RoleHeight().get_base(message->personalinfo().height()))
        {
            break;
        }
        if(!tbx::RoleIncome().get_base(message->personalinfo().salary()))
        {
            break;
        }
        if(!tbx::RoleMaritalStatus().get_base(message->personalinfo().marrystatue()))
        {
            break;
        }
        if(!tbx::RoleWeight().get_base(message->personalinfo().bodyweight()))
        {
            break;
        }
#endif
        std::vector<std::string> vec;
        pb::parseTagString(message->personalinfo().city(),",",vec);
        if(vec.size() == 3)
        {
            m_personInfo.set_city(vec[0]);
            double val = atof(vec[1].c_str());
            m_personInfo.set_longitude(val);
            val = atof(vec[2].c_str());
            m_personInfo.set_latitude(val);
        }

        m_personInfo.set_marrystatue(message->personalinfo().marrystatue());
        m_personInfo.set_height(message->personalinfo().height());
        m_personInfo.set_bodyweight(message->personalinfo().bodyweight());
        m_personInfo.set_salary(message->personalinfo().salary());
        m_personInfo.set_birthday(message->personalinfo().birthday());
        m_personInfo.set_wechat(message->personalinfo().wechat());
        m_personInfo.set_qq(message->personalinfo().qq());
#if 0
        HelloKittyMsgData::VociceData *voice = m_personInfo.mutable_voice();
        if(voice)
        {
            *voice = message->personalinfo().voice();
        }
#endif
        zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(charid);
        if(handle)
        {
            handle->set("rolebaseinfo", charid, "cityname", m_personInfo.city().c_str());
            handle->set("rolebaseinfo", charid, "birthday",m_personInfo.birthday().c_str()); 
            handle->set("rolebaseinfo", charid, "wechat", m_personInfo.wechat().c_str());
        }
        ackPersonalInfo(charid);
    }while(false);
    return ret;
}

void SceneUser::updatePresent()
{
    HelloKittyMsgData::AckPresent ack;
    ack.set_charid(charid);
    ack.set_present(m_personInfo.present());

    std::string ret;
    encodeMessage(&ack,ret);
    sendCmdToMe(ret.c_str(),ret.size());
}

void SceneUser::updateVoice()
{
    HelloKittyMsgData::AckVoice ack;
    ack.set_charid(charid);
    HelloKittyMsgData::VociceData *voice = ack.mutable_voice();
    if(voice)
    {
        *voice = m_personInfo.voice();
    }

    std::string ret;
    encodeMessage(&ack,ret);
    sendCmdToMe(ret.c_str(),ret.size());
}

void SceneUser::sendRoomInfo(const void *pstrCmd, const DWORD nCmdLen)
{
    for(auto iter = m_seeRoomSet.begin();iter != m_seeRoomSet.end();++iter)
    {
        SceneTaskManager::getMe().broadcastUserCmdToGateway(*iter,pstrCmd,nCmdLen);
    }
    this->sendCmdToMe(pstrCmd,nCmdLen);
}


bool SceneUser::ackRoom(const QWORD vistorID) 
{
    HelloKittyMsgData::AckEnterRoom ack;
    ack.set_charid(charid);
    HelloKittyMsgData::RoomInfo *roomInfo = ack.mutable_roominfo();
    if(!roomInfo)
    {
        return false;
    }
    fillRoomMsg(roomInfo);

    std::string ret;
    encodeMessage(&ack,ret);
    if(vistorID == charid)
    {
        sendCmdToMe(ret.c_str(),ret.size());
    }
    else
    {
        SceneTaskManager::getMe().broadcastUserCmdToGateway(vistorID,ret.c_str(),ret.size());
    }
    return true;
}

bool SceneUser::fillRoomMsg(HelloKittyMsgData::RoomInfo *roomInfo) 
{
    if(!roomInfo)
    {
        return false;
    }
    HelloKittyMsgData::playerShowbase* pShow = roomInfo->mutable_selfhead();
    if(pShow)
    {
        SceneUser::getplayershowbase(charid,*pShow);
    }
    roomInfo->set_charisma(m_store_house.getAttr(HelloKittyMsgData::Attr_Charisma));
    roomInfo->set_contribute(m_store_house.getAttr(HelloKittyMsgData::Attr_Contribute));
    roomInfo->set_fans(m_store_house.getAttr(HelloKittyMsgData::Attr_Fans));
    roomInfo->set_compositenum(m_store_house.getAttr(HelloKittyMsgData::Attr_Composite_Num));
    roomInfo->set_finishorder_num(m_store_house.getAttr(HelloKittyMsgData::Attr_finishorder_Num));
    roomInfo->set_finishtrainorder_num(m_store_house.getAttr(HelloKittyMsgData::Attr_finishtrainorder_Num));
    roomInfo->set_helptrainorder_num(m_store_house.getAttr(HelloKittyMsgData::Attr_helptrainorder_Num));
    roomInfo->set_helpotherevent_num(m_store_house.getAttr(HelloKittyMsgData::Attr_helpotherevent_Num));
    roomInfo->set_friends(getFriendManager().GetFriendSize());
    roomInfo->set_buildnum(m_buildManager.getBuildLevelNum(1));
    QWORD familyID = RecordFamily::getMe().getFamilyID(charbase.charid);
    if(familyID > 0)
    {
        CMD::RECORD::FamilyBase base;
        if(RecordFamily::getMe().readFamily(familyID,base))
        {
            roomInfo->set_family(base.m_strName);
        }
    }

    const std::vector<QWORD> &rvecVist =getFriendManager().getVisitFriend();
    for(auto it = rvecVist.rbegin();it != rvecVist.rend();it++)
    {
        HelloKittyMsgData::playerShowbase* pShow = roomInfo->add_visitfriend();
        if(pShow == NULL)
            continue;
        SceneUser::getplayershowbase(*it,*pShow);

    }
    std::set<RankData> rankSet;
    for(auto iter = m_acceptCharismaMap.rbegin();iter != m_acceptCharismaMap.rend();++iter)
    {
        RankData data(iter->first,iter->second);
        rankSet.insert(data);
    }
    for(auto iter = rankSet.begin();iter != rankSet.end() && roomInfo->superfan_size() < 3;++iter)
    {
        const RankData &data = *iter;
        HelloKittyMsgData::SuperFan *superFan = roomInfo->add_superfan();
        if(superFan)
        {
            HelloKittyMsgData::playerShowbase* pShow = superFan->mutable_superfan();
            if(pShow == NULL)
                continue;
            SceneUser::getplayershowbase(data.charID,*pShow);
            superFan->set_contribute(data.value);
        }
    }

    return true;
}


void SceneUser::fillVerifyLevel(HelloKittyMsgData::AckNoCenter &ack)
{
    std::string rankKey = ack.searchtype() == 0 ? "verifylevel" : (ack.searchtype() == 1 ? "man" : "female");
    std::set<RankData> rankSet;
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_VerifyLevel);
    if(!handle)
    {
        return;
    }
    handle->getRevSortSet("verifyrank",rankKey.c_str(),rankSet,0,9);

    HelloKittyMsgData::NeonCenterData *neonData = ack.add_data();
    if(!neonData)
    {
        return;
    }
    neonData->set_type(HelloKittyMsgData::NCT_Verity);
    for(auto iter = rankSet.rbegin();iter != rankSet.rend();++iter)
    {
        const RankData &rankData = *iter;
        QWORD charID = rankData.charID;
        HelloKittyMsgData::NeonCenterCell *temp = neonData->add_data();
        if(!temp)
        {
            continue;
        }
        HelloKittyMsgData::playerShowbase &rbase = *(temp->mutable_headinfo());
        SceneUser::getplayershowbase(charID,rbase);
        temp->set_value(rankData.value);
    }
    return;
}

void SceneUser::fillHot(HelloKittyMsgData::AckNoCenter &ack)
{
    std::string rankKey = ack.searchtype() == 0 ? "charisma" : (ack.searchtype() == 1 ? "man" : "female");

    std::set<RankData> rankSet;
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Charisma);
    if(!handle)
    {
        return;
    }
    handle->getRevSortSet("charismarank",rankKey.c_str(),rankSet,0,9);

    HelloKittyMsgData::NeonCenterData *neonData = ack.add_data();
    if(!neonData)
    {
        return;
    }
    neonData->set_type(HelloKittyMsgData::NCT_Hot);
    for(auto iter = rankSet.rbegin();iter != rankSet.rend();++iter)
    {
        const RankData &rankData = *iter;
        QWORD charID = rankData.charID;
        HelloKittyMsgData::NeonCenterCell *temp = neonData->add_data();
        if(!temp)
        {
            continue;
        }
        HelloKittyMsgData::playerShowbase &rbase = *(temp->mutable_headinfo());
        SceneUser::getplayershowbase(charID,rbase);
        temp->set_value(rankData.value);
    }
    return;
}

bool SceneUser::ackNoCenter(const DWORD typeID)
{
    HelloKittyMsgData::AckNoCenter ack;
    ack.set_searchtype(typeID);
    fillVerifyLevel(ack);
    fillHot(ack);

    std::string ret;
    encodeMessage(&ack,ret);
    sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool SceneUser::ackNpcRoomAndPerson(const QWORD npcID,HelloKittyMsgData::AckRoomAndPersonalInfo &ackMsg)
{
    HelloKittyMsgData::EnterGardenInfo info;
    if(!getStaticNpc(npcID,info))
    {
        return false;
    }
    ackMsg.set_charid(npcID);
    HelloKittyMsgData::RoomInfo *roomInfo = ackMsg.mutable_roominfo();
    if(roomInfo)
    {
        *roomInfo = info.roominfo();
    }
    HelloKittyMsgData::playerShowbase* pShow = roomInfo->mutable_selfhead();
    if(pShow)
    {
        SceneUser::getplayershowbase(npcID,*pShow);
    }
    HelloKittyMsgData::PersonalInfo *personalInfo = ackMsg.mutable_personalinfo();
    if(personalInfo)
    {
    }
    HelloKittyMsgData::OtherInfo *other = ackMsg.mutable_otherinfo();
    if(other)
    {
    }
    return true;
}

bool SceneUser::ackRoomAndPerson(const QWORD charID)
{
    HelloKittyMsgData::AckRoomAndPersonalInfo ackMsg;
    ackMsg.set_charid(charid);
    HelloKittyMsgData::RoomInfo *roomInfo = ackMsg.mutable_roominfo();
    if(roomInfo)
    {
        fillRoomMsg(roomInfo);
    }
    HelloKittyMsgData::PersonalInfo *personalInfo = ackMsg.mutable_personalinfo();
    if(personalInfo)
    {
        *personalInfo = m_personInfo;
    }

    HelloKittyMsgData::OtherInfo *other = ackMsg.mutable_otherinfo();
    fillOtherInfo(other,charID);

    std::string ret;
    encodeMessage(&ackMsg,ret);
    m_seeRoomSet.insert(charID);
    return charID == charid ? sendCmdToMe(ret.c_str(),ret.size()) : SceneTaskManager::getMe().broadcastUserCmdToGateway(charID,ret.c_str(),ret.size());
}

void SceneUser::opLike(const QWORD charID,const bool addFlg)
{
    if(addFlg && m_likeSet.find(charID) == m_likeSet.end())
    {
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charID);
        if(handle)
        {
            m_likeSet.insert(charID);
            const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Unity_Like);
            if(emailConf)
            {
                std::string nickName = std::string(handle->get("rolebaseinfo",charID,"nickname"));
                std::vector<HelloKittyMsgData::ReplaceWord> argVec;
                HelloKittyMsgData::ReplaceWord arg;
                arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
                arg.set_value(nickName);
                argVec.push_back(arg);
                std::map<DWORD,DWORD> itemMap;
                EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,itemMap);
            }
        }
        
    }
    else
    {
        m_likeSet.erase(charID);
    }
    ackOtherInfo();
}

bool SceneUser::isLike(const QWORD charID)
{
    return m_likeSet.find(charID) != m_likeSet.end();
}

bool SceneUser::isBuyWeChat(const QWORD charID)
{
    return m_buyedSet.find(charID) != m_buyedSet.end();
}

DWORD SceneUser::getLikeCnt()
{
    return m_likeSet.size();
}

bool SceneUser::reqNeonMark(const HelloKittyMsgData::ReqNeonMark *mark)
{
    std::set<RankData> rankSet;
    switch(mark->neonmark())
    {
        case HelloKittyMsgData::NTM_Verity:
            {
                zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_VerifyLevel);
                handle->getRevSortSet("verifyrank","verifylevel",rankSet);
            }
            break;
        case HelloKittyMsgData::NTM_Login:
            {
#if 0
                DWORD now = SceneTimeTick::currentTime.sec();
                if(now - charbin.buytimelogin() >= 24 * 2600)
                {
                    opErrorReturn(HelloKittyMsgData::Buy_Login_OutTime);
                    return true;
                }
#endif
                zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Login);
                handle->getRevSortSet("loginrank","login",rankSet);
            }
            break;
    }
    HelloKittyMsgData::AckNeonMark ackMsg;
    ackMsg.set_buytime(charbin.buytimelogin());
    ackMsg.set_neonmark(mark->neonmark());
    for(auto iter = rankSet.begin();iter != rankSet.end();++iter)
    {
        const RankData &rankData = *iter;
        HelloKittyMsgData::NeonUserInfo *neonUserInfo = ackMsg.add_userinfo();
        if(!neonUserInfo)
        {
            continue;
        }
        HelloKittyMsgData::playerShowbase* pShow = neonUserInfo->mutable_selfhead();
        if(!pShow)
        {
            continue;
        }
        zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(rankData.charID);
        if(!handle)
        {
            break;
        }
        SceneUser::getplayershowbase(rankData.charID,*pShow);
        neonUserInfo->set_city(handle->get("rolebaseinfo",rankData.charID,"cityname"));
        neonUserInfo->set_born(handle->get("rolebaseinfo",rankData.charID,"birthday"));
        neonUserInfo->set_logintime(handle->getInt("rolebaseinfo",rankData.charID,"logintime"));
        neonUserInfo->set_verifytime(handle->getInt("rolebaseinfo",rankData.charID,"verifylevel"));
        neonUserInfo->set_wechat(handle->get("rolebaseinfo",rankData.charID,"wechat"));
    }

    std::string ret;
    encodeMessage(&ackMsg,ret);
    sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

void SceneUser::ackPicture()
{
    HelloKittyMsgData::AckPicture ackMsg;
    for(int cnt = 0;cnt < m_personInfo.picture_size();++cnt)
    {
        HelloKittyMsgData::PictureInfo *picture = ackMsg.add_picture();
        if(picture)
        {
            *picture = m_personInfo.picture(cnt);
        }
    }

    std::string msg;
    encodeMessage(&ackMsg,msg);
    sendCmdToMe(msg.c_str(),msg.size());
}

bool SceneUser::resPicture(const CMD::RES::t_RspAddRes *cmd)
{
    bool ret = false;
    do
    {
        if(DWORD(m_personInfo.picture_size()) < cmd->resID) 
        {
            break;
        }
        //成功
        if(cmd->commit == 1)
        {
            HelloKittyMsgData::PictureInfo &picture = const_cast<HelloKittyMsgData::PictureInfo&>(m_personInfo.picture(cmd->resID-1));
            HelloKittyMsgData::PicInfo *picInfo = picture.mutable_photo();
            if(picInfo)
            {
                picInfo->set_url(cmd->url);
                picInfo->set_key(cmd->key);
            }
            ackPicture();
        }
        //失败
        else if(cmd->commit == 2)
        {
            ackPicture();
        }
        //请求上传
        else
        {
            HelloKittyMsgData::AckAddPicture ackMsg;
            ackMsg.set_upurl(Fir::global["upresurl"]);
            HelloKittyMsgData::PictureInfo *picture = ackMsg.mutable_picture();
            if(picture)
            {
                HelloKittyMsgData::PicInfo *picInfo = picture->mutable_photo();
                if(picInfo)
                {
                    picInfo->set_url(cmd->url);
                    picInfo->set_key(cmd->key);
                }
                picture->set_id(cmd->resID);
            }
            std::string msg;
            encodeMessage(&ackMsg,msg);
            sendCmdToMe(msg.c_str(),msg.size());
        }
        Fir::logger->debug("[上传图片确认返回](%lu,%s,%u,%u)",charid,cmd->url,cmd->key,cmd->commit);
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::resHead(const CMD::RES::t_RspAddRes *cmd)
{
    bool ret = false;
    do
    {
        if(cmd->commit)
        {
            HelloKittyMsgData::playerhead *head = charbin.mutable_head();
            if(!head)
            {
                break;
            }
            HelloKittyMsgData::PicInfo *picInfo = head->mutable_value();
            if(!picInfo)
            {
                break;
            }
            if(picInfo)
            {
                picInfo->set_url(cmd->url);
                picInfo->set_key(cmd->key);
            }
            head->set_headid(cmd->resID);
        }
        else
        {
            HelloKittyMsgData::AckSetHead ackMsg;
            ackMsg.set_upurl(Fir::global["upresurl"]);
            ackMsg.set_result(HelloKittyMsgData::GuideResult_Suc);
            HelloKittyMsgData::playerhead *tempHead = ackMsg.mutable_head();
            if(tempHead)
            {
                HelloKittyMsgData::PicInfo *picInfo = tempHead->mutable_value();
                if(picInfo)
                {
                    picInfo->set_url(cmd->url);
                    picInfo->set_key(cmd->key);
                }
                tempHead->set_headid(cmd->resID);
            }
            std::string msg;
            encodeMessage(&ackMsg,msg);
            sendCmdToMe(msg.c_str(),msg.size());
        }
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::addPicture(const HelloKittyMsgData::ReqAddPicture *message)
{
    bool ret = false;
    do
    {
        if(DWORD(m_personInfo.picture_size()) < message->id())
        {
            break;
        }
        using namespace CMD::RES;
        t_AddRes addRes;
        addRes.charID = charid;
        addRes.resID = message->id(); 
        addRes.resType = HelloKittyMsgData::RT_Picture;
        addRes.key = zMisc::randBetween(0,100000);
        addRes.time = SceneTimeTick::currentTime.sec() + 10;

        std::string msg;
        encodeMessage(&addRes,sizeof(addRes),msg);
        SceneService::getMe().sendCmdToSuperServer(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::movePicture(const HelloKittyMsgData::ReqMovePicture *message)
{
    bool ret = false;
    do
    {
        if(DWORD(m_personInfo.picture_size()) < message->src() || DWORD(m_personInfo.picture_size()) < message->des())
        {
            break;
        }
        HelloKittyMsgData::PictureInfo *src = const_cast<HelloKittyMsgData::PictureInfo*>(&m_personInfo.picture(message->src() - 1));
        HelloKittyMsgData::PictureInfo *des = const_cast<HelloKittyMsgData::PictureInfo*>(&m_personInfo.picture(message->des() - 1));
        HelloKittyMsgData::PictureInfo temp = *src;
        HelloKittyMsgData::PicInfo *picInfo = src->mutable_photo();
        if(picInfo)
        {
            *picInfo = des->photo();
        }
        picInfo = des->mutable_photo();
        if(picInfo)
        {
            *picInfo = temp.photo();
        }
        HelloKittyMsgData::AckMovePicture ackMsg;
        HelloKittyMsgData::PictureInfo *picture = ackMsg.add_picture();
        if(picture)
        {
            *picture = *src;
        }
        picture = ackMsg.add_picture();
        if(picture)
        {
            *picture = *des;
        }

        std::string msg;
        encodeMessage(&ackMsg,msg);
        sendCmdToMe(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::setPictureHead(const HelloKittyMsgData::ReqSetPictureHead *message)
{
    bool ret = false;
    do
    {
        HelloKittyMsgData::playerhead *nowHead = this->charbin.mutable_head();
        *nowHead = message->head();

        HelloKittyMsgData::AckSetHead Ack;
        Ack.set_result(HelloKittyMsgData::GuideResult_Suc);
        *(Ack.mutable_head()) = *(this->charbin.mutable_head());

        std::string msg;
        encodeMessage(&Ack,msg);
        sendCmdToMe(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::buyLoginLast()
{
    bool ret = false;
    do
    {
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"购买查看最近登录");
        DWORD money = ParamManager::getMe().GetSingleParam(eParam_Buy_Login_Last);
        if(money && !m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,money,temp,false))
        {
            opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
            break;
        }
        charbin.set_buytimelogin(SceneTimeTick::currentTime.sec());
        HelloKittyMsgData::ReqNeonMark cmd;
        cmd.set_neonmark(HelloKittyMsgData::NTM_Login);
        reqNeonMark(&cmd);
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::viewWechat(const QWORD charID)
{
    bool ret = false;
    do
    {
        if(m_viewWechatSet.find(charID) != m_viewWechatSet.end())
        {
            break;
        }
        HelloKittyMsgData::AckViewWechat ackMsg;
        std::string msg;
        encodeMessage(&ackMsg,msg);
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"购买查看微信号");
        DWORD money = ParamManager::getMe().GetSingleParam(eParam_Buy_View_Wechat);
        if(money && !m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,money,temp,false))
        {
            opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
            break;
        }
        m_viewWechatSet.insert(charID);
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charID);
        if (!handle)
        {
            Fir::logger->error("不能获取内存连接句柄");
            break;
        }
        DWORD senceId = handle->getInt("playerscene",charID,"sceneid");
        if(!senceId)
        {
            SceneUser* user  =  SceneUserManager::getMe().CreateTempUser(charID);
            if(user)
            {
                user->addBuyWechat(charid);
                ret = user->ackRoomAndPerson(charid);
                sendCmdToMe(msg.c_str(),msg.size());
            }
            break;
            
        }
        SceneUser* user = SceneUserManager::getMe().getUserByID(charID);
        if(user)
        {
            user->addBuyWechat(charid);
            ret = user->ackRoomAndPerson(charid);
            sendCmdToMe(msg.c_str(),msg.size());
        }
        else
        {
            CMD::SCENE::t_ViewWechat cmd;
            cmd.charID = charID;
            cmd.viewer = charid;

            std::string msg;
            encodeMessage(&cmd,sizeof(cmd),msg);
            SceneClientToOtherManager::getMe().SendMsgToOtherScene(senceId,msg.c_str(),msg.size());
        }
        ret = true;
    }while(false);
    return ret;
}

void SceneUser::loadViewList(const HelloKittyMsgData::Serialize& binary)
{
    for(int cnt = 0;cnt < binary.viewwechat_size();++cnt)
    {
        m_viewWechatSet.insert(binary.viewwechat(cnt));
    }
}

void SceneUser::saveViewList(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_viewWechatSet.begin();iter != m_viewWechatSet.end();++iter)
    {
        binary.add_viewwechat(*iter);
    }
}

void SceneUser::addBuyWechat(const QWORD charID)
{
    m_buyedSet.insert(charID);
}

void SceneUser::loadBuyWeChatList(const HelloKittyMsgData::Serialize& binary)
{
    for(int cnt = 0;cnt < binary.buywechat_size();++cnt)
    {
        m_buyedSet.insert(binary.buywechat(cnt));
    }
}

void SceneUser::saveBuyWeChatList(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_buyedSet.begin();iter != m_buyedSet.end();++iter)
    {
        binary.add_buywechat(*iter);
    }
}

void SceneUser::loadLikeList(const HelloKittyMsgData::Serialize& binary)
{
    for(int cnt = 0;cnt < binary.likelist_size();++cnt)
    {
        m_likeSet.insert(binary.likelist(cnt));
    }
}

void SceneUser::saveLikeList(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_likeSet.begin();iter != m_likeSet.end();++iter)
    {
        binary.add_likelist(*iter);
    }
}

