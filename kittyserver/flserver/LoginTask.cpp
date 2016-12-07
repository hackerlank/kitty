/**
 * \file
 * \version  $Id: LoginTask.cpp 2935 2005-09-20 09:00:37Z whj $
 * \author  Songsiliang,
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 定义登陆连接任务
 *
 */

#include <iostream>
#include <vector>
#include <list>
#include <iterator>
#include <ext/hash_map>

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "zMisc.h"
#include "LoginTask.h"
#include "LoginManager.h"
#include "zType.h"
#include "FLCommand.h"
#include "zDBConnPool.h"
#include "FLServer.h"
#include "GYListManager.h"
#include "zMisc.h"
#include "FLCommand.h"
#include "ServerManager.h"
#include "zTime.h"
#include "LoginCommand.h"
#include "login.pb.h"
#include "LoginCmdDispatcher.h"
#include "extractProtoMsg.h"
#include "wordFilter.h"

LoginUserCmdDispatcher LoginTask::login_user_dispatcher("login_task_logincmd");
DWORD LoginTask::uniqueID = 0;
/**
 * \brief 构造函数
 * \param pool 所属的连接池
 * \param sock TCP/IP套接口
 */
LoginTask::LoginTask( zTCPTaskPool *pool, const int sock) : zTCPTask(pool, sock, NULL, true, false), lifeTime(), tempid(0)
{
    bzero(acc_name, sizeof(acc_name));
    //mSocket.setEncMethod(CEncrypt::ENCDEC_RC5);
    //unsigned char key[16] = { 0x2c,0xc5,0x29,0x25,0xd1,0x7c,0xa6,0xc6,0x33,0x5d,0xd2,0x2a,0x2f,0xf1,0x35,0xdf};
    //unsigned char key[16] = { 0x3f,0x79,0xd5,0xe2,0x4a,0x8c,0xb6,0xc1,0xaf,0x31,0x5e,0xc7,0xeb,0x9d,0x6e,0xcb};
    //mSocket.set_key_rc5((const unsigned char *)key, 16, 12);
    //DES_cblock des_key = {'a','a','a','a','a','a','a','a'};
    //mSocket.setEncMethod(CEncrypt::ENCDEC_DES);
    //mSocket.set_key_des(&des_key);
}

void LoginTask::setClientVersion(const float version)
{
    verify_client_version = version;
}


int LoginTask::verifyConn()
{
    int retcode = mSocket.recvToBuf_NoPoll();
    if (retcode > 0)
    {
        BYTE pstrCmd[zSocket::MAX_DATASIZE];
        int nCmdLen = mSocket.recvToCmd_NoPoll(pstrCmd, sizeof(pstrCmd));
        //这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
        if (nCmdLen <= 0)
        {
            return 0;
        }

        BYTE messageType = *(BYTE*)pstrCmd;
        nCmdLen -= sizeof(BYTE);
        if(messageType != PROTOBUF_TYPE or nCmdLen <= 0)
        {
            Fir::logger->error("[客户端登录_1]:客户端连接验证失败(消息非法)");
            return -1;
        }

        bool ret = msgParseProto(pstrCmd+sizeof(BYTE),nCmdLen);
        if(ret && verify_client_version)
        {
            Fir::logger->error("[客户端登录_1]:客户端连接验证成功");
            return 1;
        }
        Fir::logger->error("[客户端登录_1]:客户端连接验证失败(消息不对)");
        return -1;
    }
    else
        return retcode;
}

int LoginTask::recycleConn()
{
    mSocket.force_sync();
    return 1;
}

bool LoginTask::uniqueAdd()
{
    return LoginManager::getMe().add(this);
}

bool LoginTask::uniqueRemove()
{
    LoginManager::getMe().remove(this);
    return true;
}


bool LoginTask::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    google::protobuf::Message *message = extraceProtoMsg(data,nCmdLen);
    if(!message)
    {
        return false;
    }
    bool ret = this->login_user_dispatcher.dispatch(this,message) ? true : false;
    if(!ret)
    {
        Fir::logger->error("%s(%s, %u)", __PRETTY_FUNCTION__, message->GetTypeName().c_str(),nCmdLen);
    }
    SAFE_DELETE(message);
    return ret;
}

bool LoginTask::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(CMD::CMD_NULL == ptNullCmd->cmd && CMD::PARA_NULL == ptNullCmd->para)
    {
        std::string ret;
        if(encodeMessage(ptNullCmd,nCmdLen,ret) && sendCmd(ret.c_str(),ret.size()))
        {
            return true;
        }
        return false;
    }

    Fir::logger->error("%s(%u, %u, %u), %s", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen, getIP());
    return false;
}

/**
 * \brief 得到客户端的IP，补足16位，型如:192.168.005.001.
 * \param clientIP 输出IP
 */
void LoginTask::getClientIP(char *clientIP)
{
    char tmpIP[MAX_IP_LENGTH+1];
    bzero(tmpIP, MAX_IP_LENGTH+1);
    strncpy(tmpIP, getIP(), MAX_IP_LENGTH);
    std::vector<std::string> ip_para;
    Fir::stringtok(ip_para, tmpIP, ".", 3);
    std::ostringstream os_ip;
    for(int i =0; i < 4; i++)
    {
        int zeroLen = 3-ip_para[i].size();
        for(int j = 0; j < zeroLen; j++)
        {
            os_ip << "0";
        }
        os_ip << ip_para[i] <<".";
    }
    strncpy(clientIP, os_ip.str().c_str(), MAX_IP_LENGTH);
    return;
}
#if 0
bool LoginTask::verifyToken(const login::IphoneUserRequestLoginCmd *message)
{
    if (!message) return false;

    using namespace CMD;
    //生成区唯一编号
    GameZone_t gameZone;
    gameZone.game = message->game();
    gameZone.zone = message->zone();
    Fir::logger->debug("请求登陆游戏区:userType=%u,account=%s,zoneid=%u,uid=%lu,token=%s", (DWORD)message->usertype(), message->account().c_str(), gameZone.zone, message->uid(), message->token().c_str());

    if (message->usertype() == 0)
    {
        LoginReturn(LOGIN_RETURN_TOKEN_ERROR);
        return false;
    }
    else
    {
        Fir::logger->info("请求令牌验证:gameZone.id:%u,account=%s,ptUserType=%u,token=%s",gameZone.id,message->account().c_str(),message->usertype(),message->token().c_str());
    }
    return true;
}
#endif
void LoginTask::LoginReturnMsg(const std::string& err_msg, const HelloKittyMsgData::LoginFailReason reason,const bool tm)
{   
    Fir::logger->debug("LoginFailReason=%s",err_msg.c_str());

    HelloKittyMsgData::AckLoginFailReturn failReturn;
    failReturn.set_failreason(reason);

    std::string ret;
    if(encodeMessage(&failReturn,ret))
    {
        sendCmd(ret.c_str(),ret.size());
    }
    else
    {
        Fir::logger->error("%s(%s, %u)", __PRETTY_FUNCTION__, failReturn.GetTypeName().c_str(),failReturn.ByteSize());
    }
    //由于登陆错误，需要断开连接
    //whj 可能导致coredown,屏蔽测试
    if (tm)  Terminate();
} 

bool LoginTask::ReqRegister(const HelloKittyMsgData::ReqRegister *message)
{
    HelloKittyMsgData::AckRegister ack;
    do
    {
        if(!message->account().size() || wordFilter::getMe().hasForbitWord(message->account().c_str()))
        {
            ack.set_result(HelloKittyMsgData::RegisterResult_AccountErr);
            break;
        }

        //查看长度
        if(message->account().empty())
        {
            ack.set_result(HelloKittyMsgData::RegisterResult_Account_Short);
            break;
        }
        if(message->account().size() > 21)
        {
            ack.set_result(HelloKittyMsgData::RegisterResult_Account_Long);
            break;
        }
        //逻辑符号检测
        if(message->account().find("~") != std::string::npos || message->account().find("`") != std::string::npos || message->account().find("!") != std::string::npos || message->account().find("@") != std::string::npos || message->account().find("#") != std::string::npos || message->account().find("$") != std::string::npos || message->account().find("%") != std::string::npos || message->account().find("^") != std::string::npos || message->account().find("&") != std::string::npos || message->account().find("*") != std::string::npos || message->account().find("(") != std::string::npos || message->account().find(")") != std::string::npos || message->account().find("-") != std::string::npos || message->account().find("+") != std::string::npos || message->account().find("   ") != std::string::npos || message->account().find("=") != std::string::npos || message->account().find("{") != std::string::npos || message->account().find("}") != std::string::npos || message->account().find("[") != std::string::npos || message->account().find("]") != std::string::npos || message->account().find(";") != std::string::npos || message->account().find(":") != std::string::npos || message->account().find("'") != std::string::npos || message->account().find("\\") != std::string::npos || message->account().find("|") != std::string::npos || message->account().find(",") != std::string::npos || message->account().find("<") != std::string::npos || message->account().find(".") != std::string::npos || message->account().find(">") != std::string::npos || message->account().find("/") != std::string::npos || message->account().find("?") != std::string::npos || message->account().find(".") != std::string::npos)
        {
            ack.set_result(HelloKittyMsgData::RegisterResult_AccountErr);
            break;
        }

        AccountInfo accountInfo;
        strncpy(accountInfo.account,message->account().c_str(),sizeof(accountInfo.account));
        strncpy(accountInfo.passwd,message->pwd().c_str(),sizeof(accountInfo.passwd));
        accountInfo.accType = HelloKittyMsgData::debug;
        if(AccountMgr::getMe().findAccount(accountInfo))
        {
            ack.set_result(HelloKittyMsgData::RegisterResult_HasRegiter);
            break;
        }
        if(!AccountMgr::getMe().dbInsert(accountInfo))
        {
            ack.set_result(HelloKittyMsgData::RegisterResult_HasRegiter);
            break;
        }
        ack.set_result(HelloKittyMsgData::RegisterResult_Suc);
    }while(false);

    std::string ret;
    encodeMessage(&ack,ret);
    sendCmd(ret.c_str(),ret.size());
    bool flg = ack.result() != HelloKittyMsgData::RegisterResult_Suc ? false : true;
    Fir::logger->error("[客户端注册_1]:客户端注册%s(%s,%u,%u)",flg ? "成功" : "失败",message->account().c_str(),HelloKittyMsgData::debug,ack.result());
    if(!flg)
    {
        //Terminate();
    }
    else
    {
        HelloKittyMsgData::ReqLogin reqLogin;
        reqLogin.set_account(message->account());
        reqLogin.set_platid(HelloKittyMsgData::debug);
        reqLogin.set_tocken(message->pwd());
        login_user_dispatcher.dispatch(this,&reqLogin);
    }
    return true;
}

bool LoginTask::requireLogin(const HelloKittyMsgData::ReqLogin * message)
{
    strncpy(m_accountinfo.account,message->account().c_str(),sizeof(m_accountinfo.account) -1);
    strncpy(m_accountinfo.passwd,message->tocken().c_str(),sizeof(m_accountinfo.passwd));
    m_accountinfo.accType = message->platid();
    HelloKittyMsgData::LoginFailReason retcode;
    if(HelloKittyMsgData::debug == m_accountinfo.accType)
    {

        if(!AccountMgr::getMe().findAccount(m_accountinfo))
        {
            retcode = HelloKittyMsgData::NoAccount; 
            LoginReturnMsg("账号错误",retcode,false);
            return false;
        }
        if(!AccountMgr::getMe().verifyPasswd(m_accountinfo))
        {
            retcode = HelloKittyMsgData::PasswdError;
            LoginReturnMsg("账号登录错误",retcode,false);
            return false;
        }
    }
    else//需要验证
    {
        //里面有检查
        if(!AccountMgr::getMe().findAccount(m_accountinfo))
        {
            AccountMgr::getMe().dbInsert(m_accountinfo);
        }
#if 0
        if(!LoginManager::getMe().SendVertifyToPlat(this))
        {
            retcode = HelloKittyMsgData::TockenError;
            Fir::logger->error("[客户端登录_3]:客户端Tocken失败(%f,%s)",verify_client_version,m_accountinfo.account);
            LoginReturnMsg("账号登录错误",retcode);
            return false;

        }
        else
        {
            return true;
        }
#endif

    }
    return DoReqLogin(true);
}
bool LoginTask::DoReqLogin(bool bSuc)
{
    HelloKittyMsgData::LoginFailReason retcode;
    if(bSuc)
    {
        GameZone_t gameZone;
        gameZone.id = AccountMgr::getMe().getZoneidByAccount();
        if (GYListManager::getMe().verifyVer(gameZone.zone, verify_client_version, retcode))
        {
            Fir::logger->debug("[客户端登录_2]:客户端请求网关成功(验证版本,%u,%f,%s)",gameZone.zone,verify_client_version,m_accountinfo.account);
        }
        else
        {
            Fir::logger->error("[客户端登录_2]:客户端请求网关失败(验证版本,%u,%f,%s)",gameZone.zone,verify_client_version,m_accountinfo.account);
            LoginReturnMsg("账号登录错误",retcode);
            return false;
        }

        t_NewLoginSession session;
        bzero(&session, sizeof(session));
        session.gameZone = gameZone;
        session.loginTempID = tempid;
        strncpy(session.client_ip, getIP(), MAX_IP_LENGTH);
        session.acctype = m_accountinfo.accType;	
        strncpy(session.account, m_accountinfo.account, sizeof(session.account));
        strncpy(acc_name, m_accountinfo.account, sizeof(acc_name));
        this->verify_login(tempid, session);
    }
    else
    {
        retcode = HelloKittyMsgData::TockenError;
        Fir::logger->error("[客户端登录_3]:客户端Tocken失败(%f,%s)",verify_client_version,m_accountinfo.account);
        LoginReturnMsg("账号登录错误",retcode);
        return false;

    }
    return true;


}

void LoginTask::verify_login(const DWORD loginTempID,const t_NewLoginSession& session)
{	
    //登陆成功直接分配网关
    GYList *gy = GYListManager::getMe().getAvl(session.gameZone.zone,session.wdNetType);
    if (NULL == gy)
    {
        Fir::logger->error("[客户端登录_2]:客户端请求网关失败(网关没开,%u,%u,%s)",session.gameZone.zone,session.loginTempID,session.account);
        this->LoginReturnMsg("网关没有开",HelloKittyMsgData::GatewayNotOpen);
    }
    else if (gy->wdNumOnline >= (LoginManager::maxGatewayUser - 10))
    {
        Fir::logger->error("[客户端登录_2]:客户端请求网关失败(用户数已满,%u,%u,%s)",session.gameZone.zone,gy->wdNumOnline,session.account);
        this->LoginReturnMsg("户数满",HelloKittyMsgData::UserFull);
    }
    else
    {
        t_NewLoginSession send_session = session;
        send_session.wdGatewayID = gy->wdServerID;
        bcopy(gy->pstrIP, send_session.pstrIP, sizeof(send_session.pstrIP));
        send_session.wdPort = gy->wdPort;
        Fir::logger->debug("[客户端登录_2]:客户端请求网关成功(分配网关,%u,%s,%u,%s)",(DWORD)gy->wdServerID,gy->pstrIP,(DWORD)gy->wdPort,session.account);	
        //生成des加密密钥
        CEncrypt e;
        e.random_key_des(&send_session.des_key);

        CMD::FL::t_NewSession_Session fl_tCmd; 
        fl_tCmd.session = send_session;
        std::string ret;
        bool flg = encodeMessage(&fl_tCmd,sizeof(fl_tCmd),ret) && ServerManager::getMe().sendCmdToZone(session.gameZone.zone,ret.c_str(),ret.size());
        if(!flg)
        {
            Fir::logger->debug("[客户端登录_2]:客户端请求网关失败(向super注册,%u,%s,%u,%s)",(DWORD)gy->wdServerID,gy->pstrIP,(DWORD)gy->wdPort,session.account);
            this->LoginReturnMsg("向区发送指令失败",HelloKittyMsgData::SendZoneError);
        }
        else
        {
            Fir::logger->debug("[客户端登录_2]:客户端请求网关成功(向super注册,%u,%s,%u,%s)",(DWORD)gy->wdServerID,gy->pstrIP,(DWORD)gy->wdPort,session.account);

            //预分配
            gy->wdNumOnline++;
        }
    }
}
