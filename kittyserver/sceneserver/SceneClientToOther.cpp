/**
 * \file
 * \version  $Id: SceneClientToOther.cpp 65 2013-04-23 09:34:49Z  $
 * \author  ,@163.com
 * \date 2004年11月05日 13时58分55秒 CST
 * \brief 定义场景服务器连接客户端
 *
 */

#include <unistd.h>
#include <iostream>

#include "zTCPClient.h"
#include "SceneCommand.h"
#include "SceneClientToOther.h"
#include "Fir.h"
#include "Command.h"
#include <time.h>
#include "extractProtoMsg.h"
#include "SceneToOtherManager.h"
#include "SceneServer.h"



/**
 * \brief 创建到场景服务器的连接
 *
 * \return 连接是否成功
 */
bool SceneClientToOther::connectToSceneServer()
{
    if (!zTCPClientTask::connect())
    {
        Fir::logger->error("连接场景服务器失败");
        return false;
    }
    using namespace CMD::SCENE;
    t_LoginScene tCmd;
    tCmd.wdServerID = SceneService::getMe().getServerID();
    tCmd.wdServerType = SceneService::getMe().getServerType();

    std::string ret;
    return encodeMessage(&tCmd,sizeof(tCmd),ret) && sendCmd(ret.c_str(),ret.size());


}

int SceneClientToOther::checkRebound()
{
    return 1;
}
void SceneClientToOther::addToContainer()
{
    SceneClientToOtherManager::getMe().add(this);
}

void SceneClientToOther::removeFromContainer()
{
    SceneClientToOtherManager::getMe().remove(this);
}
bool SceneClientToOther::connect()
{
    return connectToSceneServer();
}
bool SceneClientToOther::msgParse(const BYTE *message, const DWORD nCmdLen)
{
    return msgPush(message,nCmdLen);
}
bool SceneClientToOther::cmdMsgParse(const BYTE *message, const DWORD cmdLen)
{
    return zProcessor::msgParse(message,cmdLen);
}


bool SceneClientToOther::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    Fir::logger->error("SceneClientToOther::msgParseProto 消息没处理");
    return true;
}


bool SceneClientToOther::msgParseSceneCmd(const CMD::SCENE::SceneNull *sceneCmd,const DWORD nCmdLen)
{
    using namespace CMD::SCENE;

    switch(sceneCmd->para)
    {
        /*
           case PARA_SCENE_USER: 
           {
           t_User_FromScene* rev = (t_User_FromScene*)sceneCmd;
           GateUser* u = GateUserManager::getMe().getUserCharid(rev->charid);
           if (u && !u->isTerminateWait() && !u->isTerminate())
           {
           u->sendCmd(rev->data, rev->size);
           }
           return true;
           }
           break;
           case PARA_REFRESH_LOGIN_SCENE:
           {
           return msgParseSceneFresh((t_Refresh_LoginScene*)sceneCmd,nCmdLen);
           }
           */
        case PARA_START_OK_SCENE_GATE:
            {
                this->isStartOK = true;
                Fir::logger->debug("场景网关连接启动完成:%u",this->getServerID());
                return true;
            }
            break;
    }
    return SceneClientToOtherManager::getMe().msgParseOtherSceneCmd(wdServerID,sceneCmd,nCmdLen);
}


/**
 * \brief 解析来自场景服务器的指令
 *
 * \param ptNullCmd 待处理的指令
 * \param nCmdLen 待处理的指令长度
 * \return 解析是否成功
 */
bool SceneClientToOther::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }

    using namespace CMD::SCENE;
    bool ret = false;    
    switch(ptNullCmd->cmd)
    {
        case SCENECMD:
            {
                ret =  msgParseSceneCmd((SceneNull*)ptNullCmd,nCmdLen);
            }
            break;
    }
    if(ret == false)
        Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return true;
}

