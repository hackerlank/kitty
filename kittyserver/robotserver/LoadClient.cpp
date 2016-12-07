/**
 * \file
 * \version  $Id: LoadClient.cpp 65 2013-04-23 09:34:49Z  $
 * \author  ,@163.com
 * \date 2004年11月05日 13时58分55秒 CST
 * \brief 定义场景服务器连接客户端
 *
 */

#include <unistd.h>
#include <iostream>

#include "LoadClient.h"
#include "Fir.h"
#include <time.h>
#include "LoadClientManager.h"
#include "extractProtoMsg.h"

LoginUserCmdDispatcher LoadClient::login_user_dispatcher("login_task_logincmd"); 

/**
 * \brief 创建到场景服务器的连接
 *
 * \return 连接是否成功
 */

LoadClient::LoadClient(const std::string& strIP,SDWORD wPort,DWORD account,BYTE lang):zTCPClientTask(strIP,wPort),m_account(account),m_Loadstate(Loadfl),m_lang(lang)
{
    bisspecail = false;
    m_plattype  = 0;
}
LoadClient::LoadClient(const std::string& strIP, SDWORD wPort,std::string accont,BYTE plattype,BYTE lang,const std::string& pwd):zTCPClientTask(strIP,wPort),m_account(0),m_Loadstate(Loadfl),m_specailAcc(accont),m_plattype(plattype),m_pwd(pwd),m_lang(lang)
{
    bisspecail = true;
}
int LoadClient::checkRebound()
{
    return 1;
}
void LoadClient::addToContainer()
{
    switch(m_Loadstate)
    {
        case Loadfl:
            {
                if(!bisspecail)
                    LoadClientManager::getMe().addLoad(this);
                else
                    LoadClientManager::getMe().Set_Loadspecail(this);
            }
            break;
        case flgetGateWay:
        case InGame:
            {
                if(!bisspecail)
                    LoadClientManager::getMe().addGame(this);
                else
                    LoadClientManager::getMe().set_Gamespecail(this);
            }
            break;
        default:
            Fir::logger->error("err state %u ",m_Loadstate);
    }
}

void LoadClient::removeFromContainer()
{
    if(m_Loadstate < flgetGateWay)
    {
        if(!bisspecail)
            LoadClientManager::getMe().removeLoad(this); 
        else
            LoadClientManager::getMe().Set_Loadspecail(NULL);
    }
    else
    {
        if(!bisspecail)
            LoadClientManager::getMe().removeGame(this);
        else
            LoadClientManager::getMe().set_Gamespecail(NULL);
    }
}

void LoadClient::getstrAccountAndPlat(std::string &stracconut,DWORD &platID,std::string &strpwd)
{
    if(bisspecail)
    {
        stracconut = m_specailAcc;
        platID = m_plattype;
        strpwd = m_pwd;
    }
    else
    {
        char buf[100];
        sprintf(buf,"robot_%d",getAccount());
        stracconut =std::string(buf);
        platID = HelloKittyMsgData::debug;
        sprintf(buf,"pwd_%d",getAccount());
        strpwd = std::string(buf);

    }

}


bool LoadClient::connect()
{
    if (!zTCPClientTask::connect())
    {
        Fir::logger->error("链接登陆失败");
        return false;
    }
    switch(m_Loadstate)
    {
        case Loadfl:
            {
                HelloKittyMsgData::ReqVersion req;
                req.set_clientversion(1.0);
                std::string ret; 
                if(encodeMessage(&req,ret))
                    sendCmd(ret.c_str(), ret.size()); 
            }
            break;
        case flgetGateWay:
            {

                HelloKittyMsgData::ReqLoginGateway loginreq; 
                std::string straccount;
                DWORD type;
                std::string strpwd;
                getstrAccountAndPlat(straccount,type,strpwd);
                loginreq.set_account(straccount);
                loginreq.set_platid(HelloKittyMsgData::debug);
                loginreq.set_lang((HelloKittyMsgData::ELanguage)m_lang);
                //loginreq.set_lang(static_cast<HelloKittyMsgData::ELanguage>(m_lang));
                std::string ret; 
                if(encodeMessage(&loginreq,ret)) 
                    sendCmd(ret.c_str(), ret.size());  


            }
            break;
        default:
            Fir::logger->error("err state %u ",m_Loadstate);
    }


    return true;


}

bool LoadClient::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    google::protobuf::Message *message = extraceProtoMsg(data,nCmdLen);
    if(!message)
    {
        return false;
    }
    bool ret = this->login_user_dispatcher.dispatch(this,message) ? true : false;
    if(!ret)
    {
        //Fir::logger->error("%s(%s, %u)", __PRETTY_FUNCTION__, message->GetTypeName().c_str(),nCmdLen);
    }
    SAFE_DELETE(message);
    return ret;


}



bool LoadClient::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    Fir::logger->error("%s(%u, %u, %u), %s", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen, ip.c_str());
    return false;

}



void  LoadClient::visit(QWORD othercharid)
{
    HelloKittyMsgData::ReqEnterGarden req;
    req.set_charid(othercharid);
    std::string ret; 
    if(encodeMessage(&req,ret))
        sendCmd(ret.c_str(), ret.size()); 

}

void LoadClient::opbuild(QWORD buildid,DWORD isicon)
{
    HelloKittyMsgData::opBuilding ack;
    HelloKittyMsgData::Builditype &rBuild = *(ack.mutable_build());
    rBuild.set_buildid(buildid);
    rBuild.set_isicon(isicon > 0 ? 1 : 0);
    std::string ret;
    if(encodeMessage(&ack,ret))
        sendCmd(ret.c_str(), ret.size()); 

}
