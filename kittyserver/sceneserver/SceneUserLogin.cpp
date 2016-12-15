//此文件为user登录下线的一些逻辑操作

#include "SceneUser.h"
#include "zMetaData.h"
#include <stdarg.h>
#include "SceneServer.h"
#include "zMetaData.h"
#include "TimeTick.h"
#include "SceneUserManager.h"
#include <zlib.h>
#include <bitset>
#include "RecordClient.h"
#include "LoginUserCommand.h"
#include "xmlconfig.h"
#include <limits>
#include "ResType.h"
#include "RedisMgr.h"
#include "json/json.h"
#include "login.pb.h"
#include "extractProtoMsg.h"
#include "serialize.pb.h"
#include "dataManager.h"
#include "tbx.h"
#include "SceneMapDataManager.h"
#include "enterkitty.pb.h"
#include "RecordFamily.h"
#include "Misc.h"
#include "system.pb.h"


bool SceneUser::reg(CMD::SCENE::t_regUser_Gatescene* cmd)
{
    this->charid = cmd->charid;
    this->accid = cmd->accid;

    if(!RedisMgr::getMe().get_charbase(this->charid,this->charbase))
    {
        Fir::logger->debug("[客户端登录_3]:角色注册场景失败(charbase数据异常,%lu)",this->charid);
        return false;
    }
    HelloKittyMsgData::Serialize binary;
    if(!RedisMgr::getMe().get_binary(this->charid,binary))
    {
        Fir::logger->debug("[客户端登录_3]:角色注册场景失败(binary数据异常,%lu)",this->charid);
        return false;
    }

    if(!RedisMgr::getMe().is_login_first(this->charid))
    {
        setupBinaryArchive(binary);
    }
    changeTime();
    bool ret = SceneUserManager::getMe().addUser(this);
    Fir::logger->debug("[客户端登录_3]:角色注册场景%s(注册场景,%lu)",ret ? "成功" : "失败",this->charid);

    std::string now = SceneTimeTick::currentTime.toString();
    Fir::logger->info("[%s][t_login][f_time=%s][f_acc_id=%s][f_char_id=%lu][f_type=%s][f_remark=%s]",now.c_str(),now.c_str(),charbase.account,charid,"login","0.0.0.0");
    return ret;
}


void SceneUser::offline()
{
    Fir::logger->error("[下线]:%u,%lu,%s 角色下线成功", this->accid, this->charid, this->charbase.nickname);
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(handle)
        handle->delSet("playerset",0 ,"online" , this->charid);
    _online = false;
    charbase.offlinetime = SceneTimeTick::currentTime.sec();
    getFriendManager().offline();
    setVisit(0);
    cancelStarGame();
    save();
    m_gateid = 0;
    BroadCastPersonNum();

    std::string now = SceneTimeTick::currentTime.toString();
    Fir::logger->info("[%s][t_login][f_time=%s][f_acc_id=%s][f_char_id=%lu][f_type=%s][f_remark=%lu]",now.c_str(),now.c_str(),charbase.account,charid,"logout",SceneTimeTick::currentTime.sec() - charbase.onlinetime);

    handle = zMemDBPool::getMe().getMemDBHandle(charbase.areaType);
    if(handle)
    {
        handle->setSortSet("area",charid,charbase.areaType,0);
    }
}

bool SceneUser::online(std::string phone_uuid,SceneTask* _gate,const bool reconnect)
{
    if (_online) 
    {
        Fir::logger->debug("[客户端登录_3]:角色注册场景失败(角色已在线,%lu)",this->charid);
        return false;
    }
    _online = true;
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(handle)
    {
        handle->setSet("playerset",0 ,"online" , this->charid);
    }
    Fir::logger->debug("[客户端登录_3]:角色注册场景成功(角色在线成功,%lu)",this->charid);

#if 0
    //----所有上线初始化，放在该行以上------
    //通知会话，场景上线成功

    CMD::SCENE::t_Refresh_LoginScene ret_gate;
    ret_gate.charid = this->charid;
    std::string ret;
    encodeMessage(&ret_gate,sizeof(ret_gate),ret);
    if (_gate)
    {
        _gate->sendCmd(ret.c_str(),ret.size());
    }
#endif

    //----上线初始化完成后的处理 都放在onlineInit这个函数里------
    this->onlineInit(reconnect, _gate);
    Fir::logger->debug("inline %lu",this->charid);
    getFriendManager().online();
    BroadCastPersonNum();
    return true;
}

bool SceneUser::unreg()
{
    //玩家处于离线状态，除了登录消息，不接收任何其它用户指令
    this->offline();
    return true;
}

bool SceneUser::initNewRole()
{
    const pb::Conf_t_newRoleAttr *confBase = tbx::newRoleAttr().get_base(1);
    if(!confBase)
    {
        Fir::logger->debug("错误:SceneUser::initNewRole找不到对应的初始数据表");
        return false;
    }
    //等级
    charbase.level = 1;
    //性别
    charbase.sex = 0;
    //嘉年华数值
    m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Carnival_Val,120,"新建角色",true);
    //所穿时装
    HelloKittyMsgData::DressData *temp = charbin.mutable_dress();
    if(temp)
    {
        temp->set_id(0);
        temp->set_level(0);
    }
    //头像
    HelloKittyMsgData::playerhead *head = charbin.mutable_head();
    if(head)
    {
        HelloKittyMsgData::PicInfo *picInfo = head->mutable_value();
        if(picInfo)
        {
            picInfo->set_key(0);
        }
    }

    //愉悦值的3个档次数据
    HelloKittyMsgData::HappyData *happData = charbin.mutable_happy_low();
    if(happData)
    {
        happData->set_grade(HAPPY_LOW);
        happData->set_frequency(0);
        happData->set_time(0);
    }
    happData = charbin.mutable_happy_mid();
    if(happData)
    {
        happData->set_grade(HAPPY_MID);
        happData->set_frequency(0);
        happData->set_time(0);
    }
    happData = charbin.mutable_happy_hight();
    if(happData)
    {
        happData->set_grade(HAPPY_HIGHT);
        happData->set_frequency(0);
        happData->set_time(0);
    }
    HelloKittyMsgData::DressData *dressData = charbin.mutable_dress();
    if(dressData)
    {
        dressData->set_id(0);
        dressData->set_level(0);
    }

    for(int cnt = 0;cnt < 4;++cnt)
    {
        HelloKittyMsgData::PictureInfo *picture = m_personInfo.add_picture();
        if(picture)
        {
            picture->set_id(cnt +1);
            HelloKittyMsgData::PicInfo *picInfo = picture->mutable_photo();
            if(picInfo)
            {
                picInfo->set_key(0);
            }
        }
    }

    //初始仓库和角色属性
    m_store_house.init(confBase->getItemMap());

    //嘉年华商城
    initCarnivalShopData();

    //初始化日常数据刷新时间
    initDailyData();

    //初始化kitty乐园地图
    const MapData *mapConf = SceneMapDataManager::getMe().getMapData(KittyGardenMapID);
    m_kittyGarden.fullMapData(mapConf);

    //初始化建筑管理器
    m_buildManager.init(confBase);

    //初始化交易模块
    m_trade.initTradeInfo();

    //初始化任务列表
    m_taskManager.init();

    //初始化成就信息
    m_achievementManager.init();

    //黑市和男仆
    m_market.init();
    
#if 0
    //初始化npc摊位
    m_trade.openNpcStall();
#endif

    return true;
}

bool SceneUser::flushUserInfo()
{   
    HelloKittyMsgData::AckFlushUserInfo send;
    HelloKittyMsgData::UserBaseInfo *userBaseInfo = send.mutable_userbase();
    userBaseInfo->set_name(charbase.nickname);
    userBaseInfo->set_charid(this->charid);
    HelloKittyMsgData::SexType sexType = charbase.sex ? HelloKittyMsgData::Female : HelloKittyMsgData::Male;
    userBaseInfo->set_sex(sexType);
    *(userBaseInfo->mutable_head()) = charbin.head();
    DWORD dwfreeNum = PARAM_SINGLE(eParam_LeaveMessage);
    userBaseInfo->set_todayfreeprivate(charbin.dailydata().todayprivatelmnum() >= dwfreeNum ? 0 : dwfreeNum - charbin.dailydata().todayprivatelmnum());
    userBaseInfo->set_taskguideid(charbin.taskguidid());
    userBaseInfo->set_gametime(SceneTimeTick::currentTime.sec());
    userBaseInfo->set_giftpackflg(charbin.giftpackflg());
    userBaseInfo->set_cashpackflg(charbin.cashpackflg());
    userBaseInfo->set_familyordernum(charbin.dailydata().familyordernum());
    userBaseInfo->set_buyloginlast(charbin.buytimelogin());
    userBaseInfo->set_activecode(charbin.activecode());
    if(charbin.taskguidid() > 0)
    {
        const pb::st_Guide* pinfo= pb::Conf_t_Guide::getGuideById(charbin.taskguidid());
        if(pinfo)
        {
            userBaseInfo->set_taskguidenextstep(pinfo->getNextstep(charbin.taskguidstep(),true));
        }
    }
    HelloKittyMsgData::SuShiData *suShi = charbin.mutable_dailydata()->mutable_sushidata();
    if(suShi)
    {
        userBaseInfo->set_sushicnt(suShi->cnt());
    }
    const HelloKittyMsgData::DailyData &dailyData = charbin.dailydata();
    for(int cnt = 0;cnt < dailyData.stardata_size();++cnt)
    {
        const HelloKittyMsgData::StarData &starInfo = dailyData.stardata(cnt);
        userBaseInfo->add_starcnt(starInfo.cnt());
    }
    userBaseInfo->set_nextguideid(pb::Conf_t_NewGuide::getNextGuide(charbase.guideid,true));
    userBaseInfo->set_hassetname(charbase.hassetname);

    HelloKittyMsgData::DressData *dressData = userBaseInfo->mutable_dress();
    if(dressData)
    {
        *dressData = charbin.dress();
    }

    //填充全局buffer
    fullUserInfoBuffer(*userBaseInfo);
    //填充建筑信息
    m_buildManager.fullMessage(*userBaseInfo);
    //填充地图信息
    m_kittyGarden.fullMessage(*userBaseInfo);
    //填充建筑的产出 
    m_buildManager.fullBuildProduce(userBaseInfo);
    //填充事件信息
    m_eventmanager.fullMessage(this->charid,userBaseInfo->mutable_eventinit());
    userBaseInfo->set_familyid(RecordFamily::getMe().getFamilyID(this->charid));
    //突发事件
    m_burstEventManager.fullMessage(*userBaseInfo);
    //解锁数据
    for(auto iter = m_unLockBuildSet.begin();iter != m_unLockBuildSet.end();++iter)
    {
        userBaseInfo->add_unlockbuild(*iter);
    }
    //火车订单
    m_managertrain.fullMessage(*userBaseInfo);
    //订货系统
    m_managerordersystem.fullMessage(*userBaseInfo);
    //功能开启
    for(int i =0 ; i != charbin.funiconid_size();i++)
    {
        userBaseInfo->add_funiconid(charbin.funiconid(i));
    }
    m_unitybuild.fullMessage(*userBaseInfo);
    //贡献值
    for(auto iter = m_contrubuteMap.begin();iter != m_contrubuteMap.end();++iter)
    {
        HelloKittyMsgData::Key64Val32Pair *pair = userBaseInfo->add_contributelist();
        if(pair)
        {
            pair->set_key(iter->first);
            pair->set_val(iter->second);
        }
    }

    const std::map<DWORD,DWORD>attrMap = m_store_house.getAttrMap();
    for(auto iter = attrMap.begin();iter != attrMap.end();++iter)
    {
        HelloKittyMsgData::Key32Val32Pair *pair = userBaseInfo->add_attrval();
        if(pair)
        {
            pair->set_key(iter->first);
            pair->set_val(iter->second);
        }
    }


    //刷新角色面板消息
    std::string ret;
    if(encodeMessage(&send,ret))
    {
        this->sendCmdToMe(ret.c_str(),ret.size());
    }
    return true;
}
bool SceneUser::flushKittyGardenInfo(QWORD sendCharID, HelloKittyMsgData::AckEnterGarden &send)
{   
    HelloKittyMsgData::EnterGardenInfo *gardenInfo = send.mutable_gardeninfo();
    HelloKittyMsgData::playerShowbase* pbase = gardenInfo->mutable_playershow();
    if(!pbase)
        return false;
    getplayershowbase(this->charid,*pbase);
    gardenInfo->set_exp(m_store_house.getAttr(HelloKittyMsgData::Attr_Exp));

    //填充建筑信息
    m_buildManager.fullMessage(*gardenInfo);
    //填充地图信息
    m_kittyGarden.fullMessage(*gardenInfo);
    //填充事件信息
    m_eventmanager.fullMessage(sendCharID,gardenInfo->mutable_eventinit());
    //时装信息
    HelloKittyMsgData::DressData *dressData = gardenInfo->mutable_dress();
    if(dressData)
    {
        *dressData = charbin.dress();
    }
    //火车订单
    m_managertrain.fullMessage(*gardenInfo);
    //空闲信息
    HelloKittyMsgData::RoomInfo *roomInfo = gardenInfo->mutable_roominfo();
    if(roomInfo)
    {
        fillRoomMsg(roomInfo);
    }

    return true;
}

bool SceneUser::flushKittyGardenInfo()
{   
    HelloKittyMsgData::AckEnterGarden send;
    HelloKittyMsgData::EnterGardenInfo *gardenInfo = send.mutable_gardeninfo();
    HelloKittyMsgData::playerShowbase* pbase = gardenInfo->mutable_playershow();
    if(!pbase)
        return false;
    getplayershowbase(this->charid,*pbase);
    gardenInfo->set_exp(m_store_house.getAttr(HelloKittyMsgData::Attr_Exp));

    //填充建筑信息
    m_buildManager.fullMessage(*gardenInfo);
    //填充地图信息
    m_kittyGarden.fullMessage(*gardenInfo);
    //火车订单
    m_managertrain.fullMessage(*gardenInfo);

    //空闲信息
    HelloKittyMsgData::RoomInfo *roomInfo = gardenInfo->mutable_roominfo();
    if(roomInfo)
    {
        fillRoomMsg(roomInfo);
    }
    std::string ret;
    if(encodeMessage(&send,ret))
    {
        this->sendCmdToMe(ret.c_str(),ret.size());
    }
    return true;
}

void SceneUser::onlineInit(const bool reconnectFlg,SceneTask* _gate)
{

    //----所有上线初始化，放在该行以上------
    //通知会话，场景上线成功

    CMD::SCENE::t_Refresh_LoginScene ret_gate;
    ret_gate.charid = this->charid;
    std::string ret;
    encodeMessage(&ret_gate,sizeof(ret_gate),ret);
    if(_gate)
    {
        m_gateid = _gate->getID();
        _gate->sendCmd(ret.c_str(),ret.size());
    }
    else
        m_gateid = 0;

    if(!charbase.onlinetime)
    {
        initNewRole();
    }

    //重置日常数据
    if(isDailyData())
    {
        brushDailyData();
    }

    //初始化全局buffer
    initBuffer();

    //登录时间
    charbase.onlinetime = SceneTimeTick::currentTime.sec();     
    zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(charid);
    if(handle)
    {
        handle->setInt("rolebaseinfo",charid,"logintime",charbase.onlinetime);
    }
    handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Login);
    if(handle)
    {
        handle->setSortSet("loginrank",charid,"login",charbase.onlinetime);
    }

    //保存数据
    save();

    if(reconnectFlg)
    {
        reconnect();
    }
    else
    {
        //刷新角色面板消息
        flushUserInfo();

        //刷新摊位信息
        m_trade.flushSaleBooth();

        //刷新kitty乐园
        m_kittyGarden.flushMap();

        //上线处理嘉年华宝箱
        onLineCarnivalBox();

        //更新拍卖广场状态
        updateCenterStatus();

        //发送扭蛋次数
        ackToyTime();
        ackCoinToyDailyTime();

        //活动
        m_active.ReqgetPlayerActiveList();

    }
    //通知客人，主人上线了
    notifyVistor();

    handle = zMemDBPool::getMe().getMemDBHandle(charbase.areaType);
    if(handle)
    {
        handle->setSortSet("area",charid,charbase.areaType,1);
    }
}

void SceneUser::reconnect()
{
    //重连消息
    HelloKittyMsgData::AckReconnectInfo ackReconnect;
    m_buildManager.fullMessage(ackReconnect);
    m_buildManager.fullBuildProduce(ackReconnect);
    m_orderManager.fullInfo(ackReconnect);
    m_managerordersystem.fullMessage(ackReconnect);


    std::string ret;
    encodeMessage(&ackReconnect,ret);
    sendCmdToMe(ret.c_str(),ret.size());

    HelloKittyMsgData::AckHeartBeat ackBet;
    ackBet.set_gametime(SceneTimeTick::currentTime.sec());
    ret.clear();
    encodeMessage(&ackBet,ret);
    sendCmdToMe(ret.c_str(),ret.size());

    m_kittyGarden.flushMap();
}

