#include "TradeCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "TradeCommand.h"
#include "serialize.pb.h"
#include "extractProtoMsg.h"
#include "SceneToOtherManager.h"

bool TradeCmdHandle::reqModifyPresent(SceneUser* user, const HelloKittyMsgData::ReqModifyPresent *cmd) 
{
	if (!user || !cmd)
    {
        return false;
    }
    return user->modifyPresent(cmd->present());
}

bool TradeCmdHandle::reqModifyVoice(SceneUser* user, const HelloKittyMsgData::ReqModifyVoice *cmd) 
{
	if (!user || !cmd)
    {
        return false;
    }
    return user->modifyVoice(cmd);
}

bool TradeCmdHandle::reqEnterRoom(SceneUser* User,const HelloKittyMsgData::ReqEnterRoom *message)
{
    
    bool flag = false;
    do
    {
        if(message->charid() == 0)
        {
            break;
        }
        if(User->charid == message->charid())
        {
            User->ackRoom(message->charid());
            flag = true;
            break;
        }

        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(message->charid());
        if(!handle)
        {
            break;
        }
        DWORD SenceId = handle->getInt("playerscene",message->charid(),"sceneid");
        if(SenceId > 0)
        {
            SceneUser* user = SceneUserManager::getMe().getUserByID(message->charid());
            if(user)
            {
                user->ackRoom(User->charid);
            }
            CMD::SCENE::t_UserVistRoom sendCmd;
            sendCmd.charID = message->charid();
            sendCmd.vistor = User->charid;
            std::string ret;
            encodeMessage(&sendCmd,sizeof(sendCmd),ret);
            SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size());
            flag = true;
            break;
        }
        SceneUser* u  =  SceneUserManager::getMe().CreateTempUser(message->charid());
        flag = u ? u->ackRoom(User->charid) : false;
    }while(false);
    return flag;
}

bool TradeCmdHandle::reqNoCenter(SceneUser* user, const HelloKittyMsgData::ReqNoCenter *cmd) 
{
	if (!user || !cmd)
    {
        return false;
    }
    return user->ackNoCenter(cmd->searchtype());
}

bool TradeCmdHandle::reqOpLike(SceneUser* user, const HelloKittyMsgData::ReqOpLike *cmd) 
{
	if (!user || !cmd)
    {
        return false;
    }
    bool ret = false;
    do
    {
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->charid());
        if (!handle)
        {
            Fir::logger->error("不能获取内存连接句柄");
            break;
        }
        DWORD senceId = handle->getInt("playerscene",cmd->charid(),"sceneid");
        if(!senceId)
        {
            SceneUser* tempUser  =  SceneUserManager::getMe().CreateTempUser(cmd->charid());
            if(tempUser)
            {
                tempUser->opLike(user->charid,cmd->opadd());
                ret = true;
            }
            break;
        }
        SceneUser* tempUser = SceneUserManager::getMe().getUserByID(cmd->charid());
        if(tempUser)
        {
            tempUser->opLike(user->charid,cmd->opadd());
            ret = true;
        }
        else
        {
            CMD::SCENE::t_LikeOp likeOp;
            likeOp.charID = cmd->charid();
            likeOp.oper = user->charid;
            likeOp.addOp = cmd->opadd();
            ret = true;

            std::string msg;
            encodeMessage(&likeOp,sizeof(likeOp),msg);
            SceneClientToOtherManager::getMe().SendMsgToOtherScene(senceId,msg.c_str(),msg.size());
        }
    }while(false);

    HelloKittyMsgData::AckOpLike ackMsg;
    ackMsg.set_charid(cmd->charid());
    ackMsg.set_opadd(cmd->opadd());
    ackMsg.set_ret(ret);

    std::string msg;
    encodeMessage(&ackMsg,msg);
    user->sendCmdToMe(msg.c_str(),msg.size());
    return ret;
}

bool TradeCmdHandle::reqPersonInalInfo(SceneUser* user, const HelloKittyMsgData::ReqPersonInalInfo *cmd) 
{
	if (!user || !cmd)
    {
        return false;
    }
    bool ret = false;
    HelloKittyMsgData::ErrorCodeType code = HelloKittyMsgData::Error_Common_Occupy;
    do
    {
        //请求自己的
        if(cmd->charid() == user->charid)
        {
            return user->ackPersonalInfo(cmd->charid());
        }
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->charid());
        if (!handle)
        {
            Fir::logger->error("不能获取内存连接句柄");
            break;
        }
        QWORD charID = handle->getInt("rolebaseinfo",cmd->charid(),"charid");
        if(charID != cmd->charid())
        {
            code = HelloKittyMsgData::Check_Not_Exit;
            break;
        }
        DWORD senceId = handle->getInt("playerscene",cmd->charid(),"sceneid");
        if(!senceId)
        {
            SceneUser* tempUser =  SceneUserManager::getMe().CreateTempUser(cmd->charid());
            if(tempUser)
            {
                ret = tempUser->ackPersonalInfo(cmd->charid());
            }
            break;
        }
        SceneUser* tempUser = SceneUserManager::getMe().getUserByID(cmd->charid());
        if(tempUser)
        {
            ret = tempUser->ackPersonalInfo(user->charid);
        }
        else
        {
            CMD::SCENE::t_SeeOtherPerson seeOtherPerson;
            seeOtherPerson.charID = cmd->charid();
            seeOtherPerson.oper = user->charid;
            ret = true;

            std::string msg;
            encodeMessage(&seeOtherPerson,sizeof(seeOtherPerson),msg);
            SceneClientToOtherManager::getMe().SendMsgToOtherScene(senceId,msg.c_str(),msg.size());
        }
    }while(false);
    
    if(code != HelloKittyMsgData::Error_Common_Occupy)
    {
        user->opErrorReturn(code);
    }
    return ret;
}

bool TradeCmdHandle::reqRoomAndPersonalInfo(SceneUser* user, const HelloKittyMsgData::ReqRoomAndPersonalInfo *cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    bool ret = false;
    do
    {
        HelloKittyMsgData::AckRoomAndPersonalInfo ackMsg;
        if(ISSTATICNPC(cmd->charid()))
        {
            SceneUser::ackNpcRoomAndPerson(cmd->charid(),ackMsg);
            std::string ret;
            encodeMessage(&ackMsg,ret);
            user->sendCmdToMe(ret.c_str(),ret.size());
            return true;
        }
        //请求自己的
        if(cmd->charid() == user->charid)
        {
            ret = user->ackRoomAndPerson(cmd->charid());
            break;
        }
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->charid());
        if (!handle)
        {
            Fir::logger->error("不能获取内存连接句柄");
            break;
        }
        CharBase charbase;
        if(handle->getBin("charbase",cmd->charid(),"charbase",(char*)&(charbase)) == 0)
        {
            ackMsg.set_code(HelloKittyMsgData::Role_Not_EXIST);
            std::string msg;
            encodeMessage(&ackMsg,msg);
            user->sendCmdToMe(msg.c_str(),msg.size());
            user->opErrorReturn(HelloKittyMsgData::Role_Not_EXIST);
            break;
        }
        QWORD charID = handle->getInt("rolebaseinfo",cmd->charid(),"charid");
        if(charID != cmd->charid())
        {
            break;
        }
        DWORD senceId = handle->getInt("playerscene",cmd->charid(),"sceneid");
        if(!senceId)
        {
            SceneUser* tempUser =  SceneUserManager::getMe().CreateTempUser(cmd->charid());
            if(tempUser)
            {
                ret = tempUser->ackRoomAndPerson(user->charid);
            }
            break;
        }
        SceneUser* tempUser = SceneUserManager::getMe().getUserByID(cmd->charid());
        if(tempUser)
        {
            ret = tempUser->ackRoomAndPerson(user->charid);
        }
        else
        {
            CMD::SCENE::t_SeeOtherPerson seeOtherPerson;
            seeOtherPerson.charID = cmd->charid();
            seeOtherPerson.oper = user->charid;
            seeOtherPerson.onlyPerson = false;
            ret = true;

            std::string msg;
            encodeMessage(&seeOtherPerson,sizeof(seeOtherPerson),msg);
            SceneClientToOtherManager::getMe().SendMsgToOtherScene(senceId,msg.c_str(),msg.size());
        }
    }while(false);
    return ret;
}

bool TradeCmdHandle::reqEditPersonInalInfo(SceneUser* user, const HelloKittyMsgData::ReqEditPersonInalInfo *cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->modifyPersonalInfo(cmd);
}
 
bool TradeCmdHandle::reqNeonMark(SceneUser* user, const HelloKittyMsgData::ReqNeonMark *cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->reqNeonMark(cmd);
}
 
bool TradeCmdHandle::reqAddPicture(SceneUser* user, const HelloKittyMsgData::ReqAddPicture *cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->addPicture(cmd);
}

bool TradeCmdHandle::reqMovePicture(SceneUser* user, const HelloKittyMsgData::ReqMovePicture *cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->movePicture(cmd);
}

bool TradeCmdHandle::reqSetPictureHead(SceneUser* user, const HelloKittyMsgData::ReqSetPictureHead *cmd) 
{
    if (!user || !cmd)
    {
        return false;
    }
    return user->setPictureHead(cmd);
}
 
bool TradeCmdHandle::reqOutRoom(SceneUser* User,const HelloKittyMsgData::ReqOutRoom *message)
{
    if(message->charid() == 0)
    {
        return true;
    }
    if(User->charid == message->charid())
    {
        return User->outRoom(message->charid());
    }

    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(message->charid());
    if(!handle)
    {
        return false;
    }
    DWORD SenceId = handle->getInt("playerscene",message->charid(),"sceneid");
    if(SenceId > 0)
    {
        SceneUser* user = SceneUserManager::getMe().getUserByID(message->charid());
        if(user)
        {
            return user->outRoom(User->charid);
        }
        CMD::SCENE::t_UserVistRoom sendCmd;
        sendCmd.charID = message->charid();
        sendCmd.vistor = User->charid;
        sendCmd.enter = false;
        std::string ret;
        encodeMessage(&sendCmd,sizeof(sendCmd),ret);
        return SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size());
    }
    SceneUser* u  =  SceneUserManager::getMe().CreateTempUser(message->charid());
    return u ? u->outRoom(User->charid) : false;
}

bool TradeCmdHandle::reqBuyloginLast(SceneUser* user,const HelloKittyMsgData::ReqBuyloginLast *message)
{
    if(user && message)
    {
        user->buyLoginLast();
    }
    return true;
}

bool TradeCmdHandle::reqViewWechat(SceneUser* user,const HelloKittyMsgData::ReqViewWechat *message)
{
    if(user && message)
    {
        user->viewWechat(message->charid());
    }
    return true;
}

