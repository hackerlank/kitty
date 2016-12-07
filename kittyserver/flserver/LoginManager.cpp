/**
 * \file
 * \version  $Id: LoginManager.cpp 2699 2005-08-30 13:53:37Z yhc $
 * \author  Songsiliang,
 * \date 2004年12月17日 13时17分28秒 CST
 * \brief 登陆连接管理容器
 *
 * 
 */

#include "zMisc.h"
#include "LoginTask.h"
#include "LoginManager.h"
#include "FLCommand.h"
#include "GYListManager.h"
#include "ServerManager.h"
#include "SlaveCommand.h"
#include "login.pb.h"
#include "extractProtoMsg.h"
#include "FLServer.h"
#include "json/json.h"

DWORD LoginManager::maxGatewayUser=MAX_GATEWAYUSER;

/**
 * \brief 向容器中添加一个连接
 *
 * \param task 一个连接任务
 * \return 添加是否成功
 */
bool LoginManager::add(LoginTask *task)
{
    if (task)
    {
        zRWLock_scope_wrlock scope_wrlock(rwlock);
        task->genTempID();
        LoginTaskHashmap_const_iterator it = loginTaskSet.find(task->getTempID());
        DWORD taskID = task->getTempID();
        Fir::logger->debug("向容器中添加一个连接,ID:%d", taskID);
        if (it != loginTaskSet.end())
        {
            Fir::logger->error("向容器中添加一个连接error");
            return false;
        }
        std::pair<LoginTaskHashmap_iterator, bool> p = loginTaskSet.insert(LoginTaskHashmap_pair(task->getTempID(), task));
        return p.second;
    }
    else
    {
        Fir::logger->error("连接任务error");
        return false;
    }
}

/**
 * \brief 从一个容器中移除一个连接
 *
 * \param task 一个连接任务
 */
void LoginManager::remove(LoginTask *task)
{
    if (task)
    {
        zRWLock_scope_wrlock scope_wrlock(rwlock);
        loginTaskSet.erase(task->getTempID());
    }
}

/**
 * \brief 广播指令到指定的登陆连接
 *
 * \param loginTempID 登陆连接的唯一编号
 * \param pstrCmd 待转发的指令
 * \param nCmdLen 待转发的指令长度
 * \return 转发是否成功
 */
bool LoginManager::broadcast(const DWORD loginTempID, const void *pstrCmd, int nCmdLen)
{
    zRWLock_scope_rdlock scope_rdlock(rwlock);
    LoginTaskHashmap_iterator it = loginTaskSet.find(loginTempID);
    if (it != loginTaskSet.end())
        return it->second->sendCmd(pstrCmd, nCmdLen);
    else
    {
        Fir::logger->error("广播指令到指定的登陆连接error");
        return false;
    }
}

bool LoginManager::broadcastNewSession(const DWORD loginTempID, const t_NewLoginSession &session)
{
    bool retFlg = false;
    zRWLock_scope_rdlock scope_rdlock(rwlock);
    LoginTaskHashmap_iterator it = loginTaskSet.find(loginTempID);
    if (it != loginTaskSet.end())
    {
        HelloKittyMsgData::AckLoginSuccessReturn loginSuccess;
        LoginTask *task = it->second;

        if (!task->checkACCNAME(session.account))
        {
            Fir::logger->error("[客户端登录_2]:客户端请求网关失败(账号重复,%u,%s)",session.acctype,session.account);
            task->LoginReturnMsg("登录超时",HelloKittyMsgData::TimeOut);
            return false;
        }

        loginSuccess.set_logintempid(session.loginTempID);
        loginSuccess.set_gatewayip(session.pstrIP);
        loginSuccess.set_gatewayport(session.wdPort);

        //密钥隐藏在一段数据中
        for (int i=0; i<64; i++)
        {
            loginSuccess.add_keyarr(zMisc::randBetween(0,255));
        }

        do
        {
            loginSuccess.set_keyarr(58,zMisc::randBetween(0,248));
        }
        while((loginSuccess.keyarr(58)>49)&&((loginSuccess.keyarr(58)<59)));

        for(size_t index = 0;index < sizeof(DES_cblock)/sizeof(DWORD);++index)
        {
            loginSuccess.set_keyarr(58+index,(DWORD)session.des_key[index*sizeof(DWORD)]);
        }
        loginSuccess.set_loginret(session.login_ret ? HelloKittyMsgData::Kick : HelloKittyMsgData::Normal);

        std::string ret;
        encodeMessage(&loginSuccess,ret);
        Fir::logger->debug("[客户端登录_2]:客户端请求网关成功(向客户端发送网关ip和端口,%s,%u,%s,%u,%u,%u,%u,%s,%u", (NULL==task)?"0.0.0.0":task->getIP(),session.loginTempID, session.pstrIP, session.wdPort, session.gameZone.id, session.gameZone.game,session.gameZone.zone,session.account, session.acctype);
        retFlg = task->sendCmd(ret.c_str(),ret.size());
        //此时需要断开连接
        task->Terminate();
    }
    else
    {
        Fir::logger->error("[客户端登录_2]:客户端请求网关失败(找不到loginTempID,%u,%u,%s)",loginTempID,session.acctype,session.account);

    }
    return retFlg;
}

/**
 * \brief 验证错误时返回新的坐标到指定的客户端
 * \param loginTempID 指定的客户端连接临时编号
 * \param retcode 待返回的代码
 * \param tm 返回信息以后是否断开连接，缺省是断开连接
 */
void LoginManager::loginReturnMtcard(const DWORD loginTempID,const char *name,CMD::stServerReturnLoginFailedCmd *tCmd, const bool tm)
{
    zRWLock_scope_rdlock scope_rdlock(rwlock);
    LoginTaskHashmap_iterator it = loginTaskSet.find(loginTempID);
    if (it != loginTaskSet.end())
    {
        LoginTask *task = it->second;
        if (!task->checkACCNAME(name))
        {
            Fir::logger->error("%s 串号，服务器繁忙，断开连接 %s, %s", __PRETTY_FUNCTION__, name, task->getACCNAME());
            tCmd->byReturnCode = CMD::LOGIN_RETURN_TIMEOUT;
            task->LoginReturnMtcard(tCmd);
        }
        else
            task->LoginReturnMtcard(tCmd, tm);
    }
    else
        Fir::logger->debug("找不到该连接！！！");
}

/**
 * \brief 返回错误代码到指定的客户端
 * \param loginTempID 指定的客户端连接临时编号
 * \param retcode 待返回的代码
 * \param tm 返回信息以后是否断开连接，缺省是断开连接
 */
void LoginManager::loginReturn(const DWORD loginTempID, const BYTE retcode, const bool tm)
{
    zRWLock_scope_rdlock scope_rdlock(rwlock);
    LoginTaskHashmap_iterator it = loginTaskSet.find(loginTempID);
    if (it != loginTaskSet.end())
    {
        LoginTask *task = it->second;
        if(NULL != task)
        {
            task->LoginReturn(retcode, tm);
        }
    }
    else
        Fir::logger->debug("找不到该连接！！！");
}

/**
 * \brief 对容器中的所有元素调用回调函数
 * \param cb 回调函数实例
 */
void LoginManager::execAll(LoginTaskCallback &cb)
{
    zRWLock_scope_rdlock scope_rdlock(rwlock);
    for(LoginTaskHashmap_iterator it = loginTaskSet.begin(); it != loginTaskSet.end(); ++it)
    {
        cb.exec(it->second);
    }
}

bool LoginManager::CallBackLogin(const DWORD loginTempID,bool bSuc)
{
    zRWLock_scope_rdlock scope_rdlock(rwlock);
    LoginTaskHashmap_iterator it = loginTaskSet.find(loginTempID);
    if (it != loginTaskSet.end())
    {
        LoginTask *task = it->second;
        if (task)
        {
            //如果帐号不存在，那么创建帐号
            if(!AccountMgr::getMe().findAccount(task->account()))
            {
                AccountMgr::getMe().dbInsert(task->account());
            }
            task->DoReqLogin(bSuc);
            return true;
        }
    }
    return false;
}

bool LoginManager::SendVertifyToPlat(LoginTask *pTask)
{
    const char *url = NULL;
    char postdata[1024];
    switch(pTask->account().accType)
    {
        case HelloKittyMsgData::tongyao:
            {
                url = (Fir::global["tongyaourl"]).c_str();
                snprintf(postdata,sizeof(postdata),"token=%s&plateId=%u",pTask->account().passwd,pTask->account().accType);

            }
            break;
        default:
            break;
    }
    if(url ==NULL)
        return false;
    zHttpReq *pReq = new zHttpReq(url,postdata,&LoginManager::httpcallback,0,pTask->getTempID());
    pReq->setType(HttpReqType_Post);
    FLService::getMe().gethttppool()->addHttp(pReq);
    return true;
}


void LoginManager::httpcallback(const unsigned int & serverid, const unsigned int &loginTempID, const std::string &ret_data)
{
#if 0
    bool bSuc =false;
    Json::Reader reader;
    Json::Value root;
    Fir::logger->debug("plat return %s",ret_data.c_str());
    try{
        if(reader.parse(ret_data,root))
        {
            std::string code = root["code"].asString();
            std::string msg = root["res"]["msg"].asString();
            Fir::logger->debug("plat return code  %s ,msg %s",code.c_str(),msg.c_str());
            if(code == string("0"))
            {
                bSuc = true;
            }

        }
    }
    catch(...)
    {
        Fir::logger->error("plat return %s,err",ret_data.c_str());
        bSuc = false;
    }

    //TODO:
#endif
    bool bSuc =true;
    LoginManager::getMe().CallBackLogin(loginTempID,bSuc);

}
#if 0
void LoginManager::callBackLoginFail(const DWORD loginTempID, const HelloKittyMsgData::LoginFailReason error)
{
    zRWLock_scope_rdlock scope_rdlock(rwlock);
    LoginTaskHashmap_iterator it = loginTaskSet.find(loginTempID);
    if (it != loginTaskSet.end())
    {
        LoginTask *task = it->second;
        if (task)
        {
            task->LoginReturnMsg("登陆错误返回",error);
        }
    }
}
#endif
