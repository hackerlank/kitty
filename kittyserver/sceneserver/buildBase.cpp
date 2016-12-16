#include "buildBase.h"
#include "TimeTick.h"
#include "tbx.h"
#include "SceneUser.h"
#include "key.h"
#include "SceneMapData.h"
#include "buildLandMark.h"
#include "usecardbuild.pb.h"
#include "buffer.h"
#include "buildItemComposite.h"
#include "SceneServer.h"

QWORD BuildBase::generateID = 10000;

BuildBase::BuildBase(SceneUser* owner,const DWORD typeID,const DWORD level,const Point &point,const bool active,const QWORD inBuildID,const DWORD inBuildLevel,const QWORD friendID) : m_owner(owner),m_typeID(typeID),m_level(level),m_point(point)
{
    m_mark = HelloKittyMsgData::Build_Status_Build;
    m_produceTime = SceneTimeTick::currentTime.sec();
    m_rationMark = false;
    m_cardID = 0;
    m_useCardTime = 0;
    ++generateID;
    //m_id = generateID;
    m_id = hashRankKey(SceneService::getMe().getServerID(),SceneTimeTick::currentTime.msecs()) + generateID; 
    m_fromType = HelloKittyMsgData::BFT_Normal;
    initConfBase();
    m_break = false;
    m_event = false;
    m_dwCreateTimer = 0;
    m_lastCDSec = 0;
    m_inBuildID = inBuildID;
    m_inBuildLevel = inBuildLevel;
    m_friendID = friendID;
}

BuildBase::BuildBase(SceneUser* owner,const pb::Conf_t_building *buildConf,const Point &point,const QWORD inBuildID,const DWORD inBuildLevel,const QWORD friendID) : m_owner(owner),m_typeID(buildConf->buildInfo->dependid()),m_level(buildConf->buildInfo->level()),m_point(point)
{
    m_mark = HelloKittyMsgData::Build_Status_Build;
    m_produceTime = SceneTimeTick::currentTime.sec();
    m_rationMark = false;
    m_cardID = 0;
    m_useCardTime = 0;
    ++generateID;
    //m_id = generateID;
    m_id = hashRankKey(SceneService::getMe().getServerID(),SceneTimeTick::currentTime.msecs()) + generateID;
    confBase = buildConf;
    m_fromType = HelloKittyMsgData::BFT_Normal;
    m_break = false;
    m_event = false;
    m_dwCreateTimer = 0;
    m_lastCDSec = 0;
    m_inBuildID = inBuildID;
    m_inBuildLevel = inBuildLevel;
    m_friendID = friendID;
}

BuildBase::BuildBase(SceneUser* owner,const HelloKittyMsgData::BuildBase &buildBase) : m_owner(owner),m_typeID(buildBase.type()),m_level(buildBase.level()),m_point(buildBase.point().x(),buildBase.point().y()),m_mark(buildBase.status()),m_produceTime(buildBase.producetime()),m_rationMark(buildBase.rotationmark()),m_dwCreateTimer(buildBase.createtime()),m_lastCDSec(buildBase.lastcdsec())
{
    m_cardID = 0;
    m_useCardTime = 0;
    //++generateID;
    //m_id = generateID;
    m_id = buildBase.tempid();
    m_fromType = HelloKittyMsgData::BFT_Normal;
    initConfBase();
    m_break = false;
    m_event = false;
    m_inBuildID = buildBase.inbuildid();
    m_inBuildLevel = buildBase.inbuildlevel();
    m_friendID = buildBase.friendid();
}

BuildBase::~BuildBase()
{
}

DWORD BuildBase::getCreateTimer()
{
    return m_dwCreateTimer;
}

void BuildBase::SetCreateTimer(DWORD dwCreatimer)
{
    m_dwCreateTimer = dwCreatimer;
}

void BuildBase::save(HelloKittyMsgData::BuildBase *buildBase,const bool saveFlg)
{
    buildBase->set_tempid(m_id);
    buildBase->set_type(m_typeID);
    buildBase->set_level(m_level);
    buildBase->set_status(m_mark);
    buildBase->set_createtime(m_dwCreateTimer);
    buildBase->set_producetime(m_produceTime);
    buildBase->set_rotationmark(m_rationMark);
    buildBase->set_lastcdsec(m_lastCDSec);
    HelloKittyMsgData::Point *point = buildBase->mutable_point();
    point->set_x(m_point.x);
    point->set_y(m_point.y);
    buildBase->set_fromtype(m_fromType);
    buildBase->set_inbuildid(m_inBuildID);
    buildBase->set_inbuildlevel(m_inBuildLevel);
    buildBase->set_friendid(m_friendID);
    if(saveFlg == false)
    {
        HelloKittyMsgData::playerShowbase* pother = buildBase->mutable_othershow();
        if(pother)
            SceneUser::getplayershowbase(m_friendID,*pother);

    }
}

bool BuildBase::flush()
{

    HelloKittyMsgData::AckFlushOneBuild flushBuild;
    flushBuild.set_updatecharid(m_owner->charid);
    HelloKittyMsgData::BuildBase *buildBase = flushBuild.mutable_buildinfo();
    save(buildBase);

    std::string ret;
    encodeMessage(&flushBuild,ret);
    m_owner->broadcastMsgInMap(ret.c_str(),ret.size());
    return true;
}


bool BuildBase::initConfBase()
{
    QWORD key = hashKey(m_typeID,1);
    confBase = tbx::building().get_base(key);
    if(!confBase)
    {
        Fir::logger->error("[初始化建筑错误]:在配表中找不到对应的建筑信息 %u,%u",m_typeID,m_level);
        return false;
    }
    return true;
}

bool BuildBase::checkDependBuildMap(const std::map<DWORD,DWORD> &dependBuildMap)
{
    const std::unordered_map<DWORD,std::set<QWORD> >&buildTypeMap = m_owner->m_buildManager.getBuildTypeMap();
    for(auto iter = dependBuildMap.begin();iter != dependBuildMap.end();++iter)
    {
        auto iterType = buildTypeMap.find(iter->first);
        if(iterType == buildTypeMap.end())
        {
            return false;
        }
        const std::set<QWORD> &tempIdSet = iterType->second;
        for(auto iterid = tempIdSet.begin();iterid != tempIdSet.end();++iterid)
        {
            BuildBase *depend = m_owner->m_buildManager.getBuild(*iterid);
            if(!depend || depend->getLevel() < iter->second)
            {
                return false;
            }
        }
    }
    return true;
}

bool BuildBase::build(const Point &buildPoint,const bool initFlg,const bool rationFlg,const bool addExpFlg)
{
    if(!m_owner->m_kittyGarden.checkPoint(this,buildPoint,rationFlg))
    {
        return false;
    }
    m_owner->m_kittyGarden.addBuildPoint(this,buildPoint,rationFlg);
    m_rationMark = rationFlg;
    //新建角色时，不发消息
    if(!initFlg)
    {
        opSuccessReturn(HelloKittyMsgData::Build_Building);
    }
    //监测是否激活 
    m_owner->m_buildManager.checkBuildActive();
    if(!initFlg)
    {
        HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
        dailyData->set_buildorlevel(1 + dailyData->buildorlevel());
        TaskArgue arg(Target_Build,Attr_Add_Build_or_Level,Attr_Add_Build_or_Level,dailyData->buildorlevel());
        m_owner->m_taskManager.target(arg);
    }

    AchieveArg achieveArg(Achieve_Target_Have,Achieve_Sub_Buid_Level,m_typeID,m_level);
    m_owner->m_achievementManager.target(achieveArg);

    AchieveArg achieveArg1(Achieve_Target_Have,Achieve_Sub_Build_Num,m_typeID,m_level);
    m_owner->m_achievementManager.target(achieveArg1);
#if 0
    //创建建筑加经验(去除从仓库拿出来)
    if(confBase->buildInfo->guestproduce() && addExpFlg)
    {
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"创建建筑增加(%lu,%u,%u)",m_id,m_typeID,m_level);
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Exp,confBase->buildInfo->guestproduce(),temp,true);
    }
#endif
    return true;
}

bool BuildBase::upGrade(const DWORD effectID)
{
    //DWORD effectID = 4;
    QWORD key = hashKey(m_typeID,m_level+1);
    const pb::Conf_t_BuildUpGrade *tempConf = tbx::BuildUpGrade().get_base(key);
    if(!tempConf)
    {
        opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Build_Full_Level);
        return false;
    }

    const std::map<DWORD,DWORD> &effectMap = tempConf->getEffectMap();
    auto iter = effectMap.find(effectID);
    if(iter == effectMap.end())
    {
        return false;
    }
#if 0
    if(iter->second != 1) 
    {
        if(m_effectMap.find(effectID) == m_effectMap.end() || m_effectMap[effectID] + 1 > iter->second)
        {
            return false;
        }
    }
#endif
    if(!m_owner->checkMaterialMap(tempConf->getMaterialMap(),true))
    {
        opFailReturn(HelloKittyMsgData::Item_Not_Enough);
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"建筑升级(%lu,%u,%u,%u)",m_id,m_typeID,m_level,effectID);
    if(!m_owner->reduceMaterialMap(tempConf->getMaterialMap(),temp))
    {
        opFailReturn(HelloKittyMsgData::Item_Not_Enough);
        return false;
    }
    m_level += 1;
    if(m_effectMap.find(effectID) != m_effectMap.end())
    {
        m_effectMap[effectID] += 1;
    }
    else
    {
        m_effectMap[effectID] = 1;
    }
    const pb::Conf_t_BuildEffect *effectConf = tbx::BuildEffect().get_base(hashKey(effectID,m_effectMap[effectID]));
    if(!effectConf)
    {
        return false;
    }
    //滞留空间
    if(effectConf->effect->effecttype() == Build_Effect_Reten_Num)
    {
        BuildTypeCompositeItem *base = dynamic_cast<BuildTypeCompositeItem*>(this);
        if(base)
        {
            base->checkEffect(Build_Effect_Reten_Num);
        }
    }

    opSuccessReturn(HelloKittyMsgData::Build_Up_Grade); 

    HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
    dailyData->set_buildorlevel(1 + dailyData->buildorlevel());

    TaskArgue arg(Target_Build,Attr_Add_Build_or_Level,Attr_Add_Build_or_Level,dailyData->buildorlevel());
    m_owner->m_taskManager.target(arg); 

    TaskArgue arg1(Target_Build,Attr_Build,m_typeID,m_level);
    m_owner->m_taskManager.target(arg1); 


    AchieveArg achieveArg(Achieve_Target_Have,Achieve_Sub_Buid_Level,m_typeID,m_level);
    m_owner->m_achievementManager.target(achieveArg);

    AchieveArg achieveArg1(Achieve_Target_Have,Achieve_Sub_Build_Num,m_typeID,m_level);
    m_owner->m_achievementManager.target(achieveArg1);

    updateEffect(effectID);
    return true;
}


bool BuildBase::subLevel(const DWORD level)
{
    if(m_level <= level)
    {
        return false;
    }
    m_level -= level;
    initConfBase();
    return true;
}

#if 0
bool BuildBase::upGrade(const bool gmFlg)
{
    QWORD key = hashKey(m_typeID,m_level+1);
    const pb::Conf_t_building *tempConf = tbx::building().get_base(key);
    if(!tempConf)
    {
        opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Build_Full_Level);
        return false;
    }
#if 0
    if(!gmFlg && m_owner->charbase.level < confBase->buildInfo->premiselevel())
    {
        m_owner->opErrorReturn(HelloKittyMsgData::Build_Premise_Limit);
        return false;
    }
#endif
    if(!gmFlg && !m_owner->checkMaterialMap(confBase->getMaterialMap()))
    {
        opFailReturn(HelloKittyMsgData::Material_Not_Enough);
        return false;
    }
    if(!gmFlg && !m_owner->m_store_house.checkAttr(HelloKittyMsgData::Attr_Gold,confBase->buildInfo->requiregold()))
    {
        opFailReturn(HelloKittyMsgData::Gold_Not_Enough);
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"建筑升级(%lu,%u,%u)",m_id,m_typeID,m_level);
    if(!gmFlg && !m_owner->reduceMaterialMap(confBase->getMaterialMap(),temp))
    {
        opFailReturn(HelloKittyMsgData::Material_Not_Enough);
        return false;
    }

    bzero(temp,sizeof(temp));
    snprintf(temp,sizeof(temp),"建筑(%lu,%u)升级消耗",m_id,m_typeID);
    if(!gmFlg && !m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,confBase->buildInfo->requiregold(),temp,false))
    {
        opFailReturn(HelloKittyMsgData::Gold_Not_Enough);
        return false;
    }
    m_level += 1;
    initConfBase();
    opSuccessReturn(HelloKittyMsgData::Build_Up_Grade); 

    HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
    dailyData->set_buildorlevel(1 + dailyData->buildorlevel());

    TaskArgue arg(Target_Build,Attr_Add_Build_or_Level,Attr_Add_Build_or_Level,dailyData->buildorlevel());
    m_owner->m_taskManager.target(arg); 

    TaskArgue arg1(Target_Build,Attr_Build,m_typeID,m_level);
    m_owner->m_taskManager.target(arg1); 


    AchieveArg achieveArg(Achieve_Target_Have,Achieve_Sub_Buid_Level,m_typeID,m_level);
    m_owner->m_achievementManager.target(achieveArg);

    AchieveArg achieveArg1(Achieve_Target_Have,Achieve_Sub_Build_Num,m_typeID,m_level);
    m_owner->m_achievementManager.target(achieveArg1);

    //升级工人小屋
    if(m_typeID == WORK_HOUSER_ID)
    {
        m_owner->m_buildManager.addWorker();
    }

#if 0
    //升级建筑加经验
    if(confBase->buildInfo->guestproduce())
    {
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"升级建筑增加(%lu,%u,%u)",m_id,m_typeID,m_level);
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Exp,confBase->buildInfo->guestproduce(),temp,true);
    }
#endif
    return true;
}
#endif

bool BuildBase::changeStatus(const HelloKittyMsgData::BuildStatueType &status)
{
    m_mark = status;
    return true;
}

bool BuildBase::movePlace(const HelloKittyMsgData::ReqBuildMovePlace *message)
{
    Point buildPoint(message->point());
    Point oldPoint(m_point);
    if(!move(buildPoint,message->moveflg()))
    {
        return false;
    }
    m_rationMark = message->moveflg();
    opSuccessReturn(HelloKittyMsgData::Build_Move,oldPoint);
    return true;
}

bool BuildBase::move(const Point &buildPoint,const bool rationFlg)
{
    if(!m_owner->m_kittyGarden.checkPoint(this,buildPoint,rationFlg))
    {
        return false;
    }
    m_owner->m_kittyGarden.eraseBuildPoint(this);
    m_owner->m_kittyGarden.addBuildPoint(this,buildPoint,rationFlg);
    m_point = buildPoint;
    return true;
}

bool BuildBase::clickACtive()
{
    if(!(m_mark & HelloKittyMsgData::Build_Status_Finish))
    {
        return false;
    }

    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"激活建筑(%lu,%u,%u)",m_id,m_typeID,m_level);
    if(!confBase->getActiveMaterialMap().empty())
    {
        if(!m_owner->checkMaterialMap(confBase->getActiveMaterialMap(),true) || !m_owner->m_store_house.addOrConsumeItem(confBase->getActiveMaterialMap(),temp,false))
        {
            return false;
        }
    }
    m_mark = HelloKittyMsgData::Build_Status_Click_Active;
    if(m_owner->m_kittyGarden.checkActive(this,getPoint(),getRationMask()))
    {
        m_mark = HelloKittyMsgData::Build_Status_Normal;
    }

    flush();

    //处理人气值
    bzero(temp,sizeof(temp));
    snprintf(temp,sizeof(temp),"建造激活增加(%u,%u)",m_typeID,m_level);
    if(confBase->buildInfo->costpopular())
    {
        if(m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Now_Temp,confBase->buildInfo->costpopular(),temp,false))
        {
            m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Now,confBase->buildInfo->costpopular(),temp,true);
        }
    }
    if(confBase->buildInfo->addpopular())
    {
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Popular_Max,confBase->buildInfo->addpopular(),temp,true);
    }
    m_owner->checkFunOpen(pb::Conf_t_openfun::eOpen_Build,m_typeID);
    m_owner->triggerTaskGuid(pb::Conf_t_Guide::TIGGERTYPE_BUILD,m_typeID);

    return true;
}

bool BuildBase::loop()
{
    bool updateFlg = false;
    DWORD now = SceneTimeTick::currentTime.sec(); 
    do
    {
        //处于建造cd状态
        if(m_mark & HelloKittyMsgData::Build_Status_Build)
        {
            if(m_owner->stopCd(System_Build))
            {
                break;
            }
            if(now - m_dwCreateTimer < confBase->buildInfo->time())
            {
                m_lastCDSec = confBase->buildInfo->time() - (now - m_dwCreateTimer);
                break;
            }
            m_lastCDSec = 0;
            m_mark = HelloKittyMsgData::Build_Status_Finish;
            updateFlg = true;
        }
        //处于建造完成状态
        if(m_mark & HelloKittyMsgData::Build_Status_Finish)
        {
            break;
        }
        //处于建筑被玩家激活，但是没有被道路激活状态
        if(m_mark & HelloKittyMsgData::Build_Status_Click_Active)
        {
            break;
        }
        if(m_cardID)
        {
            break;
        }
        const pb::Conf_t_itemInfo* base = tbx::itemInfo().get_base(m_cardID); 
        if(!base)
        {
            break;
        }
        m_cardID = 0;
        m_useCardTime = 0;
        updateCard();
    }while(0);
    if(updateFlg)
    {
        flush();
    }
    return true;
}

bool BuildBase::opSuccessReturn(const HelloKittyMsgData::BuildSuccessCodeType &code,const Point &oldPoint)
{
    HelloKittyMsgData::AckBuildOpReturnSuccess opReturn;
    opReturn.set_updatecharid(m_owner->charid);
    opReturn.set_code(code);
    opReturn.set_tempid(m_id);
    HelloKittyMsgData::BuildBase *buildBase = opReturn.mutable_buildinfo();
    if(!buildBase)
    {
        return false;
    }
    save(buildBase);

    HelloKittyMsgData::Point *pt = opReturn.mutable_oldpoint();
    pt->set_x(oldPoint.x);
    pt->set_y(oldPoint.y);

    std::string ret;
    encodeMessage(&opReturn,ret);
    m_owner->broadcastMsgInMap(ret.c_str(),ret.size());
    return true;
}

bool BuildBase::opFailReturn(const HelloKittyMsgData::ErrorCodeType &commonError,const HelloKittyMsgData::BuildFailCodeType &code)
{
    HelloKittyMsgData::AckBuildOpReturnFail opReturn;
    opReturn.set_commoncode(commonError);
    opReturn.set_code(code);
    opReturn.set_tempid(m_id);
    HelloKittyMsgData::BuildBase *buildBase = opReturn.mutable_buildinfo();
    if(!buildBase)
    {
        return false;
    }
    save(buildBase);
    std::string ret;
    encodeMessage(&opReturn,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

void BuildBase::processChangeStatus(const bool loginFlg)
{
    DWORD now = SceneTimeTick::currentTime.sec();
    if(isActive())
    {
        m_produceTime = now;
    }
}

void BuildBase::saveAsWareHouseBuildBase(WareHouseBuildBase &info)
{
    info.typeID = m_typeID;
    info.level = m_level;
    info.produceTime = m_produceTime;
    info.mark = m_mark;
    info.num = 1;
    info.inBuildID = m_inBuildID;
    info.inBuildLevel = m_inBuildLevel;
    info.friendID = m_friendID;
}

void BuildBase::loadWareHouseBuildBase(const WareHouseBuildBase &info)
{
    m_typeID = info.typeID;
    m_level = info.level;
    m_mark = info.mark;
    m_inBuildID = info.inBuildID;
    m_inBuildLevel = info.inBuildLevel;
    m_friendID = info.friendID;
}



bool BuildBase::recycle(const DWORD itemID,const DWORD itemNum)
{
    DWORD nowTime = SceneTimeTick::currentTime.sec();
    if((nowTime - m_produceTime) < confBase->buildInfo->selltime())
    {
        opFailReturn(HelloKittyMsgData::Build_In_CD);
        return false;
    }
    if(!m_owner->m_store_house.hasEnoughItem(itemID,itemNum) || !m_owner->m_store_house.sallSystem(itemID,itemNum))
    {
        m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough);
        return false;
    }
    m_produceTime = nowTime;
    flush();
    return true;
}

bool BuildBase::fullUserCard(HelloKittyMsgData::UserBaseInfo &useInfo)
{
    if(!m_cardID)
    {
        return false;
    }
    HelloKittyMsgData::UseCardInfo *temp = useInfo.add_usecard();
    if(!temp)
    {
        return false;
    }
    temp->set_tempid(m_id);
    temp->set_cardid(m_cardID);
    temp->set_usecardtime(m_useCardTime);
    return true;
}

bool BuildBase::fullUserCard(HelloKittyMsgData::AckReconnectInfo &reconnect)
{
    if(!m_cardID)
    {
        return false;
    }
    HelloKittyMsgData::UseCardInfo *temp = reconnect.add_usecard();
    if(!temp)
    {
        return false;
    }
    temp->set_tempid(m_id);
    temp->set_cardid(m_cardID);
    temp->set_usecardtime(m_useCardTime);
    return true;
}
bool BuildBase::saveCard(HelloKittyMsgData::Serialize& binary)
{
    if(!m_cardID)
    {
        return false;
    }
    HelloKittyMsgData::UseCardInfo *temp = binary.add_usecard();
    if(!temp)
    {
        return false;
    }
    temp->set_tempid(m_id);
    temp->set_cardid(m_cardID);
    temp->set_usecardtime(m_useCardTime);
    return true;
}

bool BuildBase::loadCard(const HelloKittyMsgData::UseCardInfo &temp)
{
    m_cardID = temp.cardid();
    m_useCardTime = temp.usecardtime();
    return true;
}

bool BuildBase::loadEffect(const HelloKittyMsgData::BuildEffect &temp)
{
    for(int cnt = 0;cnt < temp.effect_size();++cnt)
    {
        const HelloKittyMsgData::EffectData &data = temp.effect(cnt);
        m_effectMap.insert(std::pair<DWORD,DWORD>(data.effectid(),data.level()));
    }
    return true;
}

bool BuildBase::saveEffect(HelloKittyMsgData::Serialize& binary)
{
    if(m_effectMap.empty())
    {
        return true;
    }
    HelloKittyMsgData::BuildEffect *temp = binary.add_buildeffect();
    if(!temp)
    {
        return false;
    }
    temp->set_tempid(m_id);
    for(auto iter = m_effectMap.begin();iter != m_effectMap.end();++iter)
    {
        HelloKittyMsgData::EffectData *data = temp->add_effect();
        if(data)
        {
            data->set_effectid(iter->first);
            data->set_level(iter->second);
        }
    }
    return true;
}

bool BuildBase::flushEffect(HelloKittyMsgData::UserBaseInfo& binary)
{
    if(m_effectMap.empty())
    {
        return true;
    }
    HelloKittyMsgData::BuildEffect *temp = binary.add_buildeffect();
    if(!temp)
    {
        return false;
    }
    temp->set_tempid(m_id);
    for(auto iter = m_effectMap.begin();iter != m_effectMap.end();++iter)
    {
        HelloKittyMsgData::EffectData *data = temp->add_effect();
        if(data)
        {
            data->set_effectid(iter->first);
            data->set_level(iter->second);
        }
    }
    return true;
}



bool BuildBase::updateCard()
{
    HelloKittyMsgData::AckUpdateCard ackMessage;
    HelloKittyMsgData::UseCardInfo *temp = ackMessage.mutable_cardinfo();
    if(!temp)
    {
        return false;
    }
    temp->set_tempid(m_id);
    temp->set_cardid(m_cardID);
    temp->set_usecardtime(m_useCardTime);

    std::string ret;
    encodeMessage(&ackMessage,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool BuildBase::useCard(const DWORD card)
{
    const pb::Conf_t_itemInfo* base = tbx::itemInfo().get_base(card);
    if(m_cardID || !base)
    {
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"建筑使用卡牌(%lu,%u,%u)",m_id,m_typeID,m_level);
    if(!m_owner->m_store_house.addOrConsumeItem(card,1,temp,false))
    {
        return m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,card);
    }
    m_cardID = card;
    m_useCardTime = SceneTimeTick::currentTime.sec();
    m_owner->m_buildManager.opCardSet(m_id,m_cardID);
    updateCard();
    return true;
}

bool BuildBase::clearCard()
{
    const pb::Conf_t_itemInfo* base = tbx::itemInfo().get_base(m_cardID);
    if(!m_cardID || !base)
    {
        return false;
    }
    m_owner->m_buildManager.opCardSet(m_id,m_cardID,false);
    m_cardID = 0;
    m_useCardTime = 0; 
    updateCard();
    return true;
}

void BuildBase::collectPt(std::set<Point> &ptSet)
{
    const Point &gridPoint = confBase->getGridPoint();
    Point gridPt(gridPoint);
    //如果翻转
    if(m_rationMark)
    {
        gridPt.x = gridPoint.y;
        gridPt.y = gridPoint.x;
    }
    for(int width = 0;width < gridPt.x;++width)
    {
        for(int height = 0;height < gridPt.y;++height)
        {
            Point tempPoint(m_point.x+width,m_point.y+height);
            ptSet.insert(tempPoint);
        }
    }
}

bool BuildBase::parseBuildCD()
{
    if(!(m_mark & HelloKittyMsgData::Build_Status_Build))
    {
        return false;
    }
    DWORD now = SceneTimeTick::currentTime.sec();
    DWORD lastSec = confBase->buildInfo->time() - (now - m_dwCreateTimer); 
    DWORD money = MiscManager::getMe().getMoneyForReduceTimer(eTimerReduce_Third,lastSec);
    char reMark[100] = {0};
    snprintf(reMark,sizeof(reMark),"建筑购买cd(%lu,%u,%u,%u)",m_id,m_typeID,m_level,lastSec);
    if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,money,reMark,false))
    {
        m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
        return false;
    }
    m_mark = HelloKittyMsgData::Build_Status_Finish;
    m_lastCDSec = 0;
    flush();
    return true;
}

bool BuildBase::updateEffect(const DWORD effectID)
{
    HelloKittyMsgData::AckUpdateEffect update;
    update.set_tempid(m_id);
    //  if(m_effectMap.find(effectID) == m_effectMap.end())
    {
        for(auto iter = m_effectMap.begin();iter != m_effectMap.end();++iter)
        {
            HelloKittyMsgData::EffectData *effectData = update.add_effect();
            if(effectData)
            {
                effectData->set_effectid(iter->first);
                effectData->set_level(iter->second);
            }
        }
    }
#if 0
    else
    {
        HelloKittyMsgData::EffectData *effectData = update.add_effect();
        if(effectData)
        {
            effectData->set_effectid(effectID);
            effectData->set_level(m_effectMap[effectID]);
        }
    }
#endif

    std::string ret;
    encodeMessage(&update,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}
