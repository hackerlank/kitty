/**
 * \file
 * \version  $Id: LoginCmdDispatcher.cpp 64 2013-04-23 02:05:08Z  $
 * \author   ,
 * \date 2013年03月27日 12时14分48秒 CST
 * \brief 定义用户登录相关命令处理文件，注册给dispatcher
 *
 */

#include "LoginCmdDispatcher.h"

bool LoginCmdHandle::setVersion(LoginTask* task, const HelloKittyMsgData::ReqVersion *message)
{
    if(task && message)
    {
        task->setClientVersion(message->clientversion());
        HelloKittyMsgData::AckVersion ackVersion;
        ackVersion.set_version(message->clientversion());

        std::string ret;
        if(encodeMessage(&ackVersion,ret))
        {
            task->sendCmd(ret.c_str(), ret.size());
        }   
		return true;
    }
    return false;

}

bool LoginCmdHandle::requireLogin(LoginTask* task,const HelloKittyMsgData::ReqLogin *message)
{
    if(!task || !message)
    {
        return false;
    }
    
    task->requireLogin(message);
    return true;
}

bool LoginCmdHandle::ReqRegister(LoginTask* task,const HelloKittyMsgData::ReqRegister *message)
{
    if(!task || !message)
    {
        return false;
    }
    task->ReqRegister(message);
    return true;


}
