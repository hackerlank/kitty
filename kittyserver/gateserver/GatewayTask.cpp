/**
 * \file
 * \version  $Id: GatewayTask.cpp 67 2013-04-23 09:44:20Z  $
 * \author  ,@163.com
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 定义登陆连接任务
 *
 */


#include <iostream>
#include <vector>
#include <list>
#include <iterator>
#include <unordered_map>
#include <stdlib.h>
#include <iconv.h>

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "zMisc.h"
#include "GatewayTask.h"
#include "GatewayServer.h"
#include "Command.h"
#include "RecordClient.h"
#include "SceneCommand.h"
#include "SceneClient.h"
#include "GatewayTaskManager.h"
#include "TimeTick.h"
#include "wordFilter.h"
#include "EncDec/EncDec.h"
#include "LoginUserCommand.h"
#include "login.pb.h"
#include "serialize.pb.h"
#include "item.pb.h"
#include "build.pb.h"
#include "common.pb.h"
#include "warehouse.pb.h"
#include "kittygarden.pb.h"
#include "task.pb.h"
#include "atlas.pb.h"
#include "gm.pb.h"
#include "friend.pb.h"
#include "event.pb.h"
#include "achievement.pb.h"
#include "enterkitty.pb.h"
#include "carnival.pb.h"
#include "email.pb.h"
#include "produce.pb.h"
#include "usecardbuild.pb.h"
#include "paper.pb.h"
#include "divine.pb.h"
#include "burstevent.pb.h"
#include "family.pb.h"
#include "littergame.pb.h"
#include "auction.pb.h"
#include "order.pb.h"
#include "chat.pb.h"
#include "giftpackage.pb.h"
#include "rank.pb.h"
#include "guide.pb.h"
#include "active.pb.h"
#include "toy.pb.h"
#include "room.pb.h"
#include "trainorder.pb.h"
#include "ordersystem.pb.h"
#include "signin.pb.h"
#include "composite.pb.h"
#include "recharge.pb.h"
#include "uuid.h"
#include "playeractive.pb.h"

DWORD GatewayTask::checkTimeInterval = 60000;
GateUserCmdDispatcher GatewayTask::gate_user_cmd_dispatcher("gateusercmd");
GateTaskCmdDispatcher GatewayTask::gate_task_cmd_dispatcher("gatetaskusercmd");
unsigned long long GatewayTask::lastCmdTime = 0;
std::map<const google::protobuf::Descriptor*,ServerType> GatewayTask::s_protoNameToServerTypeMap;

/**
 * \brief 构造函数
 *
 * \param pool 所属连接池指针
 * \param sock TCP/IP套接口
 * \param addr 地址
 */
GatewayTask::GatewayTask(
        zTCPTaskPool *pool,
        const int sock,
        const struct sockaddr_in *addr) :
    zTCPTask(pool, sock, addr, false, false),
    //v_lastSampleTime(GatewayTimeTick::currentTime),
    _retset_gametime(3600), 
    initTime(),
    lastCheckTime(60),
    _login_gametime(1)
{
    //v_lastSampleTime.addDelay(sampleInterval);
    v_lastSampleTime=0;
    //	lastCheckTime.addDelay(checkTimeInterval);
    haveCheckTime = true;
    v_samplePackets = 0;
    accid = 0;
    loginTempID = 0;
    versionVerified = false;
    verifyAccid = false;
    inlonginstate = false;
    acctype = 0; //UUID登录，还是通行证登录
    dwTimestampServer=0;
    qwGameTime=0;
    m_pUser=NULL;
    reconnect = false;

    //mSocket.setEncMethod(CEncrypt::ENCDEC_RC5);
    //mSocket.enc.set_key_rc5((const unsigned char *)Fir::global["rc5_key"].c_str(), 16, 12);
    //unsigned char key[16] = {28, 196, 25, 36, 193, 125, 86, 197, 35, 92, 194, 41, 31, 240, 37, 223};
    //unsigned char key[16] = { 0x2c,0xc5,0x29,0x25,0xd1,0x7c,0xa6,0xc6,0x33,0x5d,0xd2,0x2a,0x2f,0xf1,0x35,0xdf};
    //unsigned char key[16] = { 0x3f,0x79,0xd5,0xe2,0x4a,0x8c,0xb6,0xc1,0xaf,0x31,0x5e,0xc7,0xeb,0x9d,0x6e,0xcb};
    //mSocket.set_key_rc5((const unsigned char *)key, 16, 12);

    //设置zSocket的加密密钥
    //DES_cblock des_key = {'a','a','a','a','a','a','a','a'};
    //mSocket.setEncMethod(CEncrypt::ENCDEC_DES);
    //mSocket.set_key_des(&des_key);

    timeout_count = 0;
    lang = 0;
}

/**
 * \brief 析构函数
 *
 */
GatewayTask::~GatewayTask()
{
    if(m_pUser)
    {
        m_pUser->gatewaytask=NULL;
        SAFE_DELETE(m_pUser);
    }
}

/**
 * \brief 验证版本号
 * \param ptNullCmd 待验证的指令
 * \return 验证版本号码是否成功
 */
bool GatewayTask::verifyVersion(const HelloKittyMsgData::ReqVersion *message)
{
    if(!message) return false;
    if(GatewayService::getMe().verify_client_version == 0 || message->clientversion() >= GatewayService::getMe().verify_client_version)
    {
        versionVerified = true;
        return true;
    }
    Fir::logger->debug("[客户端登录_3]:版本成功(%s,%f,%f)",this->getIP(), message->clientversion(),GatewayService::getMe().verify_client_version);
    this->login_return(HelloKittyMsgData::VersionError);
    return false;
}

/**
 * \brief 验证登陆网关指令
 * 验证登陆网关的指令
 * \param ptNullCmd 待验证的登陆指令
 * \return 验证登陆指令是否成功
 */
bool GatewayTask::verifyACCID(const HelloKittyMsgData::ReqLoginGateway *message)
{
    if(!message)
    {
        return false;
    }

    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if (!handle) 
    {   
        Fir::logger->debug("[客户端登录_3]:请求登录失败(memdb连接失败,%u,%s)",message->platid(),message->account().c_str());
        login_return(HelloKittyMsgData::TimeOut);
        return false;
    }   

    // 应该在need_gate_id上登录
    DWORD need_gate_id = handle->getInt("gatewaylogin",message->platid(),message->account().c_str(), "gate_id");
    // 当前gateway是
    DWORD cur_gate_id = GatewayService::getMe().getServerID();
    if(need_gate_id != cur_gate_id)
    {   
        Fir::logger->debug("[客户端登录_3]:请求登录失败(gateid不对,%u,%u,%u,%s)",cur_gate_id,need_gate_id,message->platid(),message->account().c_str());
        return false;
    }   
    if(message->lang() >= HelloKittyMsgData::Elang_MAX)
    {
        this->lang = HelloKittyMsgData::Elang_Simplified_Chinese;
    }
    else
    {
        this->lang  = message->lang();
    }
    handle->setInt("gatewaylogin", message->platid(), message->account().c_str(), "state", GATEWAY_USER_LOGINSESSION_STATE_REG);

    this->acctype = message->platid();//登录类型
    this->account = message->account().c_str();
    this->changeLoginState();

    this->charid = handle->getInt("rolebaseinfo",message->platid(), message->account().c_str());
    if(this->charid == 0)
    {
        Fir::logger->debug("[客户端登录_3]:请求登录成功(等待创建角色,%lu,%u,%s)",charid,acctype,account.c_str());
        verifyAccid = true;
        return true;
    }

    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(this->charid);
    if (!handle2) 
    {   
        Fir::logger->debug("[客户端登录_3]:请求登录失败(memdb连接失败,%u,%s)",message->platid(),message->account().c_str());
        login_return(HelloKittyMsgData::UuidError);
        return false;
    } 

    CharBase charbase;
    if (handle2->getBin("charbase",this->charid,"charbase",(char*)&charbase) == 0)
    {
        Fir::logger->debug("[客户端登录_3]:请求登录失败(rides获取不到记录,%lu,%u,%s)",charid,acctype,account.c_str());
        return false;
    }
    Fir::logger->debug("[客户端登录_3]:请求登录成功(rides获取到记录,%lu,%u,%s)",charid,acctype,account.c_str());
    verifyAccid = true;
    return true;
}

bool GatewayTask::reqReconnect(const HelloKittyMsgData::ReqReconnectGateway *message)
{
    if(!message)
    {
        return false;
    }

    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(message->usertype());
    if (!handle) 
    {   
        Fir::logger->debug("[客户端重连_1]:请求重连失败(memdb连接失败,%u,%s)",message->usertype(),message->account().c_str());
        login_return(HelloKittyMsgData::TimeOut);
        return false;
    }   

    // 应该在need_gate_id上登录
    DWORD need_gate_id = handle->getInt("gatewaylogin",message->usertype(),message->account().c_str(), "gate_id");
    // 当前gateway是
    DWORD cur_gate_id = GatewayService::getMe().getServerID();
    if(!need_gate_id)
    {   
        Fir::logger->debug("[客户端重连_1]:请求重连失败(gateid不对,%u,%u,%u,%s)",cur_gate_id,need_gate_id,message->usertype(),message->account().c_str());
        return false;
    }   
    if(message->lang() >= HelloKittyMsgData::Elang_MAX)
    {
        this->lang = HelloKittyMsgData::Elang_Simplified_Chinese;
    }
    else
    {
        this->lang  = message->lang();
    }
    handle->setInt("gatewaylogin", message->usertype(), message->account().c_str(), "state", GATEWAY_USER_LOGINSESSION_STATE_REG);

    this->acctype = message->usertype();//登录类型
    this->account = message->account().c_str();
    this->changeLoginState();
    this->charid = handle->getInt("rolebaseinfo",message->usertype(), message->account().c_str());
    if(this->charid == 0)
    {
        Fir::logger->debug("[客户端重连_1]:无角色失败(无角色,%lu,%u,%s)",charid,acctype,account.c_str());
        return false;
    }
    verifyAccid = true;
    this->reconnect = true;
    return true;
}


/**
 * \brief 连接确认
 *
 * \return 如果没有数据返回0,验证成功返回1,验证失败返回-1
 */
int GatewayTask::verifyConn()
{
    int retcode = mSocket.recvToBuf_NoPoll();
    if (retcode > 0)
    {
        while(true)
        {
            unsigned char acceptCmd[zSocket::MAX_DATASIZE];
            int nCmdLen = mSocket.recvToCmd_NoPoll(acceptCmd, sizeof(acceptCmd));
            //这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
            if (nCmdLen <= 0)
            {
                return 0;
            }

            BYTE messageType = *(BYTE*)acceptCmd;
            nCmdLen -= sizeof(BYTE);
            BYTE *pstrCmd = acceptCmd + sizeof(BYTE);
            if(nCmdLen <= 0 || messageType != PROTOBUF_TYPE)
            {
                Fir::logger->debug("[客户端登录_3]:版本验证失败(消息非法,%u,%s)",acctype,account.c_str());
                return -1;
            }

            if(msgParseProto(pstrCmd,nCmdLen) && verifyAccid)
            {
                Fir::logger->debug("[客户端登录_3]:请求登录成功(验证accid,%u,%s)",acctype,account.c_str());
                return 1;
            }

            Fir::logger->debug("[客户端登录_3]:请求登录失败(消息错误,%u,%s)",acctype,account.c_str());
            return -1;
        }
    }
    else
        return retcode;
}

/**
 * \brief 连接同步等待
 *
 *
 * \return 错误返回-1,成功返回1,继续等待返回0
 */
int GatewayTask::waitSync()
{
    return 1;
}

/**
 * \brief 需要主动断开客户端的连接
 *
 * \param method 连接断开方式
 */
void GatewayTask::Terminate(const TerminateMethod method)
{
    bool is_terminate = isTerminate();
    zTCPTask::Terminate(method);

    if (m_pUser)
    {
        Fir::logger->trace("[GS],charid=%lu,%s,注销 ip=%s",m_pUser->charid,m_pUser->nickname,getIP());
        if(!is_terminate)
        {
            m_pUser->unreg();
            m_pUser->final();
        }

        if (this->isUnique())
        {
            this->uniqueRemove();
            this->initUnique();
        }
    }
    else
    {
        Fir::logger->trace("注销但用户已经不存在");
    }
    // */
}

/**
 * \brief 新用户登陆
 *
 */
void GatewayTask::addToContainer()
{
    GateUser* pUser = GateUserManager::getMe().getUserAccount(this->acctype,this->account);
    if(pUser)
    {
        return ;
    }
    pUser = new GateUser(this->acctype, this->account, this);
    if(!pUser)
    {
        return ;
    }
    pUser->charid = this->charid;
    pUser->accid  = this->accid; 
    pUser->acctype = this->acctype;
    pUser->strMacAddr = this->strMacAddr;
    pUser->strFlat = this->strFlat;
    pUser->strPhoneUuid = this->strPhoneUuid;
    pUser->account = this->account;
    pUser->lang = this->lang;
    pUser->reconnect = this->reconnect;

    if (pUser)
    {
        Fir::logger->debug("[客户端登录_3]:登录游戏成功(创建GateUser,%lu,%u,%s)",charid,acctype,account.c_str());
        initTime = GatewayTimeTick::currentTime;
        if (pUser->charid == 0)
        {
            Fir::logger->debug("[客户端登录_3]:登录游戏成功(创建GateUser且等待创建角色,%lu,%u,%s)",charid,acctype,account.c_str());
            char systemName[MAX_NAMESIZE] = {0};
            /*
               DWORD tryint = 10;
               zMemDB*  redishandle = zMemDBPool::getMe().getMemDBHandle();
               if(redishandle == NULL)
               return ;
               do{
               tryint--;
               snprintf(systemName,sizeof(systemName),"%s%lu","system",utils::unique_id_t::generate(HelloKittyMsgData::GUIDType_SYSTEMNAME,GatewayService::getMe().getServerID()));
               if(QWORD(redishandle->getInt("rolebaseinfo",systemName,"nickname")) > 0)
               {
               Fir::logger->error("name repeated :%s",systemName);
               continue;
               }
               if(!redishandle->getLock("resetname",QWORD(0),systemName,10))
               {
               Fir::logger->error("name repeated :%s lock fail",systemName);
               continue;
               }
               break;

               }while(tryint != 0 );
               if(tryint == 0)
               {
               Fir::logger->error("try 10 times fail name repeated");

               pUser->nameRepeat();
               return ;
               }
               */
            pUser->createState();
            //没有角色，系统先帮他创建一个吧
            HelloKittyMsgData::ReqCreateRole createMessage;
            createMessage.set_name(systemName);
            createMessage.set_sex(HelloKittyMsgData::Male);
            createMessage.set_heroid(1);
            Fir::logger->debug("[客户端登录_3]:登录游戏成功(系统创建角色,%lu,%u,%s)",charid,acctype,account.c_str());
            this->gate_user_cmd_dispatcher.dispatch(pUser,&createMessage);
            GateUserManager::getMe().addUserAccount(pUser);
        }
        else
        {
            if(!isForbid(pUser->charid))
            {
                return;
            }
            if(pUser->beginSelect())
            {
                GateUserManager::getMe().addUserAccount(pUser);
                if (!GateUserManager::getMe().addUserCharid(pUser))
                {
                    Fir::logger->debug("[客户端登录_3]:登录游戏失败(添加到GateUserManager,%lu,%u,%s)",charid,acctype,account.c_str());
                    return ;
                }
                //通知登陆服务器，网关连接数
                GatewayService::getMe().notifyLoginServer();
            }
            else 
            {
                Fir::logger->debug("[客户端登录_3]:登录游戏失败(beginSelect失败,%lu,%u,%s)",charid,acctype,account.c_str());
                SAFE_DELETE(pUser);
                m_pUser = NULL;

            }
        }
    }
}

/**
 * \brief 从容器中删除
 *
 */
void GatewayTask::removeFromContainer()
{
    //通知登陆服务器，网关连接数
    GatewayService::getMe().notifyLoginServer();
}

/**
 * \brief 添加一个连接线程
 *
 *
 * \return 成功返回ture,否则返回false
 */
bool GatewayTask::uniqueAdd()
{
    return GatewayTaskManager::getMe().uniqueAdd(this);
}

/**
 * \brief 删除一个连接线程
 *
 *
 * \return 成功返回ture,否则返回false
 */
bool GatewayTask::uniqueRemove()
{
    return GatewayTaskManager::getMe().uniqueRemove(this);
}


/**
 * \brief 将消息转发到场景
 *
 *
 * \param ptNullCMD: 需要转发的指令
 * \param nCmdLen: 指令长度
 * \return 转发是否成功
 */
bool GatewayTask::forwardScene(const google::protobuf::Message *message)
{
    if(m_pUser && m_pUser->scene)
    {
        BYTE buf[zSocket::MAX_DATASIZE] = {0};
        CMD::SCENE::t_Scene_ForwardScene *sendCmd=(CMD::SCENE::t_Scene_ForwardScene *)buf;
        constructInPlace(sendCmd);
        sendCmd->charid=m_pUser->charid;
        sendCmd->accid = m_pUser->accid;

        std::string ret;
        encodeMessage(message,ret);
        sendCmd->size = ret.size();
        bcopy(ret.c_str(),sendCmd->data,sendCmd->size);

        ret.clear();
        encodeMessage(sendCmd,sizeof(CMD::SCENE::t_Scene_ForwardScene)+sendCmd->size,ret);
        m_pUser->scene->sendCmd(ret.c_str(),ret.size());
        return true;
    }
    if(strcmp(message->GetTypeName().c_str(),"HelloKittyMsgData.ReqCompositeWork") == 0)
    {
        Fir::logger->debug("接收到消息 5555 HelloKittyMsgData.ReqCompositeWork");
    }
    return false;
}
/**
 * \brief 解析时间消息
 *
 *
 * \param ptNullCMD: 需要转发的指令
 * \param nCmdLen: 指令长度
 * \return 解析是否成功
 */
bool GatewayTask::msgParse_Time(const CMD::NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    using namespace CMD;

    switch(ptNullCmd->byParam)
    {
        case USERGAMETIME_TIMER_USERCMD_PARA:
            {
                if(!dwTimestampServer)
                {
                    dwTimestampServer=GatewayTimeTick::currentTime.msecs() - ptNullCmd->dwTimestamp;
                    if(!dwTimestampServer)
                    {
                        dwTimestampServer = 1; 
                    }
                }
                stUserGameTimeTimerUserCmd *rev=(stUserGameTimeTimerUserCmd *)ptNullCmd;
                if(qwGameTime && (SQWORD)(rev->qwGameTime - (qwGameTime + 
                                (GatewayTimeTick::currentTime.sec() - GameTimeSyn.sec()))) >= sampleInterval_error_sec)
                {   
                    if(this->m_pUser)
                    {
                        Fir::logger->trace("客户端游戏时间太快，使用了加速器，需要断开连接（accid = %u, %u, %lu）", this->m_pUser->accid, ptNullCmd->dwTimestamp, rev->qwGameTime - (qwGameTime + (GatewayTimeTick::currentTime.sec() - GameTimeSyn.sec())));
                    }
                    Fir::logger->debug("游戏时间:(accid=%u,%lu,%lu)", accid,rev->qwGameTime, qwGameTime + (GatewayTimeTick::currentTime.sec() - GameTimeSyn.sec()));
                    TerminateWait();
                }

                if (Fir::global["validtime"] == "false")
                    return true;

                haveCheckTime = true;
                return true;
            }
            break;
        case PING_TIMER_USERCMD_PARA:
            {       
                stPingTimeTimerUserCmd cmd;
                sendCmd(&cmd, sizeof(cmd));
                return true;
            }       
            break;  
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->byCmd, ptNullCmd->byParam, nCmdLen);
    return false;
}

/**
 * \brief 检测时间
 *
 * \param ct: 当前时间
 * \return 是否通过检测
 */
bool GatewayTask::checkTime(const zRTime &ct)
{
    return false;
#if 0
    Fir::logger->error("%s", __PRETTY_FUNCTION__);
    if (lastCheckTime(ct) && (initTime.elapse(ct)/1000L>60))
    {
        //			Fir::logger->trace("cur: %u, lastCheckTime: %u ", ct.msecs(), lastCheckTime.msecs());			
        //			if ((ct.msecs() - lastCheckTime.msecs())>120) return false;

        Fir::logger->error("%s 到时间了 可以校时", __PRETTY_FUNCTION__);

        if (haveCheckTime)
        {
            //timeout_count = 0;
            //校对时间
            if(_retset_gametime(ct))
            {
                CMD::stGameTimeTimerUserCmd cmd;
                if(qwGameTime)
                {
                    Fir::logger->error("%s 重新和客户端校时", __PRETTY_FUNCTION__);
                    cmd.qwGameTime = qwGameTime + (GatewayTimeTick::currentTime.sec() - GameTimeSyn.sec());
                    sendCmd(&cmd, sizeof(cmd));
                }
                //准备重新设置同步时间
                dwTimestampServer=0;
            }
            Fir::logger->error("%s 到时间了 跟客户端请求校时", __PRETTY_FUNCTION__);
            haveCheckTime = false; //wait to check time between client adn server.
            CMD::stRequestUserGameTimeTimerUserCmd cmd;
            sendCmd(&cmd, sizeof(cmd));
        }
        else
        {
            //	if (timeout_count>=3)
            //	{
            Fir::logger->trace("客户端指定时间内没有返回校对时间指令（accid = %u, snd_queue_size = %u）,注销", 
                    accid, mSocket.snd_queue_size());

            TerminateWait();
            //	}
            //	else
            //	{
            //		Fir::logger->trace("客户端指定时间内没有返回校对时间指令（accid = %u, snd_queue_size = %u）,次数:%u", accid, mSocket.snd_queue_size(), timeout_count);
            //		timeout_count++;
            //	}

            return true;
        }

        //			lastCheckTime = ct;
        //			lastCheckTime.addDelay(checkTimeInterval);
    }
    return false;
#endif
}

/**
 * \brief 对客户端发送过来的指令进行检测
 * 主要检测时间戳等，对加速器等进行防止
 * \param pCmd 待检测的指令
 * \param ct 当前时间
 * \return 检测是否成功
 */
bool GatewayTask::checkUserCmd(const CMD::NullCmd *pCmd, const zRTime &ct)
{
    if(dwTimestampServer)
    {
        //		if(abs((dwTimestampServer + pCmd->dwTimestamp) - ct.msecs()) > sampleInterval_error_msecs)
        if(int((dwTimestampServer + pCmd->dwTimestamp) - ct.msecs()) > (int)sampleInterval_error_msecs)
        {
            Fir::logger->trace("客户端指令时间太快，使用了加速器，需要断开连接（accid = %u, %llu, %llu）(%d, %d)", 
                    accid, (dwTimestampServer + pCmd->dwTimestamp) - ct.msecs(), initTime.elapse(ct), 
                    pCmd->byCmd, pCmd->byParam);

            v_lastSampleTime = pCmd->dwTimestamp + sampleInterval;
            //			dwTimestampServer = ct.msecs() - pCmd->dwTimestamp;

            return false;
        }
#ifdef _ZJW_DEBUG
        else
        {
            //this->debug("服务器相对时间:%d,客户端相对时间:%d",
            //		ct.msec() - _init_timeSyn.msec(), pCmd->dwTimestamp - _init_dwTimestamp);
        }
#endif

#ifdef _ALL_SUPER_GM
        Fir::logger->debug("accid=%u,客户端传过来的时间戮=%u,服务器时间戮=%u",accid, pCmd->dwTimestamp,v_lastSampleTime);
#endif

        if (pCmd->dwTimestamp >= v_lastSampleTime)
        {
            v_lastSampleTime = pCmd->dwTimestamp + sampleInterval;
            /*
               if(v_samplePackets > 30)
               {
               Fir::logger->debug("%d毫秒收到指令个数%d",sampleInterval , v_samplePackets);
               }
            // */
            v_samplePackets = 0;
            analysisCmd.clear();
#ifdef _ALL_SUPER_GM
            Fir::logger->debug("accid=%u,客户端传过来的时间戮=%u,服务器时间戮=%u,清空指令分析",accid, pCmd->dwTimestamp,v_lastSampleTime);
#endif
        }
        v_samplePackets++;
        analysisCmd[pCmd->_id]++;
        if (v_samplePackets > maxSamplePPS)
        {
            for (auto it = analysisCmd.begin(); it != analysisCmd.end(); ++it)
            {
                Fir::logger->debug("[客户端消息]id:%u,发送的次数:%u",it->first,it->second);
            }

            Fir::logger->trace("客户端指令发送过快，需要断开连接（accid = %u, %u/%u毫秒）(%u, %u)", 
                    accid, v_samplePackets, sampleInterval, pCmd->byCmd, pCmd->byParam);

            analysisCmd.clear();

#ifndef _ZJW_DEBUG			
            return false;
#endif			
        }
        /*
           int interval = pCmd->dwTimestamp - initTime.elapse(ct);
           if (interval > 1200)
           {
           Fir::logger->trace("客户端时间太快，可能使用了加速器，断开连接（accid = %u, %u, %lu）", accid, pCmd->dwTimestamp, initTime.elapse(ct));
           return false;
           }
        // */
    }
    return true;
}

bool GatewayTask::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    const google::protobuf::Message *message = extraceProtoMsg(data,nCmdLen);
    bool ret = false;
    if(!message) 
    {
        return ret;
    }

    //如果是消息转发，那么就先优先
    if(forward_cmd_dispatch(m_pUser,message))
    {
        SAFE_DELETE(message);
        return true;
    }

    if(!m_pUser)
    {
        ret = this->gate_task_cmd_dispatcher.dispatch(this,message);
    }
    else
    {
        ret = this->gate_user_cmd_dispatcher.dispatch(m_pUser,message);
    }

    SAFE_DELETE(message);
    return ret;
}

/**
 * \brief 解析消息
 *
 * \param ptNull: 需要转发的指令
 * \param nCmdLen: 指令长度
 * \return 解析是否成功
 */
bool GatewayTask::msgParseStruct(const CMD::t_NullCmd *ptNull, const DWORD nCmdLen)
{ 
    if(msgParseNullCmd(ptNull,nCmdLen))
    {
        return true;
    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNull->byCmd, ptNull->byParam, nCmdLen);
    return false;
}

/**
 * \brief 检查新创建的人物名字是否合法
 *
 *
 * \param newName 新名字
 * \return 是否合法
 */
bool GatewayTask::checkNewName(char *_newName)
{
    //DWORD i = 0;
    bool ret = true;

    unsigned char* newName = zMisc::stringConv((unsigned char*)_newName, "UTF-8", "GBK");
    if (!newName) return false;

    //if ((!wordFilter::getMe().doFilter((char*)newName, 16))
    if (strstr((char*)newName, "蒙面人")
            ||strstr((char*)newName, "GM")
            ||strstr((char*)newName, "GameMaster")
            ||strstr((char*)newName, ":"))
    {
        ret=false;
    }

    if (newName) SAFE_DELETE_VEC(newName);
    return ret;
}

void GatewayTask::login_return(const HelloKittyMsgData::LoginFailReason code)
{
    HelloKittyMsgData::AckLoginFailReturn message;
    message.set_failreason(code);

    std::string ret;
    if(encodeMessage(&message,ret))
    {
        this->sendCmd(ret.c_str(),ret.size());
    }
}

bool GatewayTask::sendCmd(const void *pstrCmd, const DWORD nCmdLen)
{
    return zTCPTask::sendCmd(pstrCmd,nCmdLen);
}

bool GatewayTask::checkLoginTime()
{
    if (!_login_gametime(GatewayTimeTick::currentTime))
    {
        return false;
    }
    return true;
}

bool GatewayTask::forward_cmd_dispatch(GateUser* u, const google::protobuf::Message *message)
{
    if(strcmp(message->GetTypeName().c_str(),"HelloKittyMsgData.ReqCompositeWork") == 0)
    {
        Fir::logger->debug("接收到消息 1111 HelloKittyMsgData.ReqCompositeWork");
    }
    auto iter = s_protoNameToServerTypeMap.find(message->GetDescriptor());
    if(iter == s_protoNameToServerTypeMap.end())
    {
        return false;
    }
    if(strcmp(message->GetTypeName().c_str(),"HelloKittyMsgData.ReqCompositeWork") == 0)
    {
        Fir::logger->debug("接收到消息 222 HelloKittyMsgData.ReqCompositeWork");
    }
    if(!u || !u->isPlayState())
    {
        return true;
    }

    if(strcmp(message->GetTypeName().c_str(),"HelloKittyMsgData.ReqCompositeWork") == 0)
    {
        Fir::logger->debug("接收到消息 333 HelloKittyMsgData.ReqCompositeWork");
    }
    ServerType serverType = iter->second;
    if(serverType == SCENESSERVER)
    {
        forwardScene(message);
    }
    if(strcmp(message->GetTypeName().c_str(),"HelloKittyMsgData.ReqCompositeWork") == 0)
    {
        Fir::logger->debug("接收到消息 444 HelloKittyMsgData.ReqCompositeWork");
    }
    if(!u || !u->isPlayState())
    {
        return false;
    }
    return true;
}

bool GatewayTask::isForbid(const QWORD charID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charID);
    if(!handle)
    {
        return false;
    }
    DWORD endTime = handle->getInt("forbid",charID,"forbidtime");
    DWORD now = GatewayTimeTick::currentTime.sec();
    if(now < endTime)
    {
        HelloKittyMsgData::AckForBid ackForBid;
        ackForBid.set_reason("被封号");

        std::string ret;
        encodeMessage(&ackForBid,ret);
        sendCmd(ret.c_str(),ret.size());
        return false;
    }
    return true;
}

void GatewayTask::initProtoDispatch()
{
    //注意，此处为注册某个proto消息流转到哪个类型的服务器

    //场景服务器

    //摊位和仓库
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqContribute::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAddItem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSallPutItem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqStoreItem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqUseItem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAdvertise::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqPayGrid::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSallSystem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSellPaper::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqPurchase::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpCell::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAdvertiseTime::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqPurchaseAdvertiseCD::descriptor()] = SCENESSERVER;
    //建筑
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBuildUpGrade::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqUpGrade::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBuildMovePlace::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBuildBuilding::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOneBuild::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAllBuild::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqPickUpBuid::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqPickOutBuid::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqWorker::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqResetWorker::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqClickRewardBuid::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqRecycleItem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBuildProduce::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqProduceCell::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqCompositeCell::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqProduceCellWork::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqProduceOp::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqCompositeOp::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpCard::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqUserCard::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBuildRoad::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqClearRoad::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAllConstructBuild::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqUnLockBuild::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqClickActiveBuild::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqParseBuildCD::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqCompositeWork::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSellWareHouseBuild::descriptor()] = SCENESSERVER;

    //乐园
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpenArea::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqKittyGarden::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqParseAreaGridCD::descriptor()] = SCENESSERVER;
    //任务
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAllTask::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSubmitTask::descriptor()] = SCENESSERVER;
    //图鉴
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAtlas::descriptor()] = SCENESSERVER;
    //gm指令
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqGM::descriptor()] = SCENESSERVER;
    //relation 关系 包括好友等
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAddFriend::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqKickFriend::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqRelationList::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqEnterGarden::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::opBuilding::descriptor()]  =  SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqclearSweetBox::descriptor()]  =  SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqGetOnePerson::descriptor()]  =  SCENESSERVER;
    //成就
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAllAchieve::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSubmitAchieve::descriptor()] = SCENESSERVER;
    //邮件
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqEmail::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSendEmail::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpEmail::descriptor()] = SCENESSERVER;
    //时装系统
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqDress::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpDress::descriptor()] = SCENESSERVER;
    //图纸系统
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqPaper::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpPaper::descriptor()] = SCENESSERVER;
    //占卜系统
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqDivineInfo::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqDivine::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqDivineVerify::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqDivineNotice::descriptor()] = SCENESSERVER;
    //嘉年华系统
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpenCarnical::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqClickCarnicalBox::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBuyCarnicalBox::descriptor()] = SCENESSERVER;
    //商店购买
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqPurchaseItem::descriptor()] = SCENESSERVER;
    //突发事件
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBurstEvent::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpBurstEvent::descriptor()] = SCENESSERVER;
    //家族
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqGetFamilyList::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAddFamily::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqCancelApply::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqCreateFamily::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAgreeJoin::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqselfFamilyInfo::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqQuitFamily::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqUpdateOtherFamilyJob::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqKickFamilyMember::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSetFamilyBaseInfo::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqFamilyRanking::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqFinishFamilyOrder::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqGetlastFamilyAward::descriptor()] = SCENESSERVER;

    //小游戏
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBeginStar::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqStarCommitStep::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBeginSlot::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBeginMacro::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBeginSuShi::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqCommitStep::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqStartGame::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqJoinStartGame::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqCancelStar::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSyncStar::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqReadyGame::descriptor()] = SCENESSERVER;
    //拍卖
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpCenter::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpRoom::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBid::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAutoBid::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBuyNormalGift::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqExchangeGiftNum::descriptor()] = SCENESSERVER;
    //订单
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOrderList::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqFinishOrder::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqFlushOrder::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqClearCD::descriptor()] = SCENESSERVER;

    //聊天
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqChat::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqLeaveMessage::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqServerNotice::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqDelMessage::descriptor()] = SCENESSERVER;

    //礼品
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpGift::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqUpdate::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAddress::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqChangeAddress::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqCashGift::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqCommitGift::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqPhyCondInfo::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSendFlower::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSendVirtualGift::descriptor()] = SCENESSERVER;
    //排行榜
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqRank::descriptor()] = SCENESSERVER;
    //新手
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSetRoleName::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::Reqsetguidefinish::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqsetTaskguidefinish::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSetHead::descriptor()] = SCENESSERVER;
    //活动
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqJoinActive::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAckActiveInfo::descriptor()] = SCENESSERVER;
    //扭蛋
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpToy::descriptor()] = SCENESSERVER;
    //空间
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqModifyPresent::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqModifyVoice::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqEnterRoom::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqNoCenter::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpLike::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqPersonInalInfo::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqRoomAndPersonalInfo::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqEditPersonInalInfo::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqNeonMark::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAddPicture::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqMovePicture::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSetPictureHead::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOutRoom::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqViewWechat::descriptor()] = SCENESSERVER;
    //火车订单
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqLoadCarriage::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAskHelpLoadCarriage::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAnswerHelpLoadCarriage::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqGetTrainAward::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpenNewTrain::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqClearTrainCD::descriptor()] = SCENESSERVER;
    //订货系统
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAddOrderSystem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqRunOrderSystem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqFinishOrderSystem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqClearOrderSystemCD::descriptor()] = SCENESSERVER;
    //签到
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqGetSignInData::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqSignIn::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqGetTotalAward::descriptor()] = SCENESSERVER;
    //充值，货币兑换
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqChangeMoney::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqChangeMoneyList::descriptor()] = SCENESSERVER;
    //黑市及其男仆
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqMarketAndServantInfo::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBuyMarketItem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqFlushMarket::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBuyServant::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqBuyServantItem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqGetServantAutoItem::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpenServantBox::descriptor()] = SCENESSERVER;
    //合建
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAllUnitBuildInfo::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqOpenColId::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqResetColId::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqUnitBuild::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAgreeUnitBuild::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqStopBuild::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqAddSpeedBuild::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqActiveBuild::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqCancelInvite::descriptor()] = SCENESSERVER;
    //玩家运营活动
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqgetActiveAward::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqgetPlayerActiveList::descriptor()] = SCENESSERVER;
    s_protoNameToServerTypeMap[HelloKittyMsgData::ReqRewardActiveCode::descriptor()] = SCENESSERVER;








}

