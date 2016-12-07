/**
 * \file
 * \version  $Id: FLClient.cpp 252 2013-05-30 04:02:01Z  $
 * \author  ,@163.com
 * \date 2005年03月12日 16时16分40秒 CST
 * \brief 定义登陆服务器客户端
 *
 * 
 */

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "ResourceClient.h"
#include "SuperServer.h"
#include "SuperCommand.h"
#include "ServerManager.h"
#include "ResourceClientManager.h"
#include "LoginUserCommand.h"
#include "extractProtoMsg.h"
#include "ResourceCommand.h"
#include "extractProtoMsg.h"

WORD ResourceClient::tempidAllocator = 0;

ResourceClient::ResourceClient(const std::string &ip,const unsigned short port) : zTCPClientTask(ip, port, true), tempid(++tempidAllocator)
{
}

ResourceClient::~ResourceClient()
{
}

int ResourceClient::checkRebound()
{
	int retcode = pSocket->recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE acceptCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = pSocket->recvToCmd_NoPoll(acceptCmd, sizeof(acceptCmd));
		//这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
        if (nCmdLen <= 0)
        {
			return 0;
        }
        
        BYTE messageType = *(BYTE*)acceptCmd;
        nCmdLen -= sizeof(BYTE);
        BYTE *pstrCmd = acceptCmd + sizeof(BYTE);
        if(messageType != STRUCT_TYPE ||  nCmdLen <= 0)
        {
            Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__, messageType,nCmdLen-1);
            return -1;
        }
        
        using namespace CMD::RES;
        t_LoginRes_OK *ptCmd = (t_LoginRes_OK*)pstrCmd;
        if (RESCMD == ptCmd->cmd && PARA_LOGIN_OK == ptCmd->para)
        {
            Fir::logger->debug("登陆ResourceServer成功，收到区的编号：(%u,%u,%u,%s)",ptCmd->gameZone.id, ptCmd->gameZone.game, ptCmd->gameZone.zone, ptCmd->name);
            return 1;
        }
        Fir::logger->error("登陆ResourceServer失败");
        return -1;
	}
	else
		return retcode;
}

void ResourceClient::addToContainer()
{
	ResourceClientManager::getMe().add(this);
}

void ResourceClient::removeFromContainer()
{
	ResourceClientManager::getMe().remove(this);
}

bool ResourceClient::connect()
{
	if (!zTCPClientTask::connect())
    {
		return false;
    }
    using namespace CMD::RES;
    t_LoginRes tCmd;
    strncpy(tCmd.strIP, SuperService::getMe().getIP(), sizeof(tCmd.strIP)-1);
    tCmd.port = SuperService::getMe().getPort();
    
    std::string ret;
    if(encodeMessage(&tCmd,sizeof(tCmd),ret))
    {
        return sendCmd(ret.c_str(),ret.size());
    }
    return false;

}

bool ResourceClient::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    Fir::logger->error("ResourceClient::msgParseProto 没处理");
    return true;
}

bool ResourceClient::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }

    using namespace CMD::RES;
    switch(ptNullCmd->cmd)
    {
        case RESCMD:
            {
                return msgParseResourceCmd((ResNullCmd*)ptNullCmd,nCmdLen);
            }
            break;
    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return true;
}

bool ResourceClient::msgParseResourceCmd(const CMD::RES::ResNullCmd *resNullCmd, const DWORD nCmdLen)
{
	using namespace CMD::RES;
    switch(resNullCmd->para)
    {
        case PARA_RSP_ADD_RES:
            const t_RspAddRes *rspAddRes = (const t_RspAddRes*)resNullCmd;
            zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(rspAddRes->charID);
            if (!handle)
            {
                Fir::logger->error("不能获取内存连接句柄");
                break;
            }
            std::string msg;
            encodeMessage(rspAddRes,sizeof(CMD::RES::t_RspAddRes),msg);
            DWORD senceId = handle->getInt("playerscene",rspAddRes->charID,"sceneid");
            if(senceId)
            {
                ServerManager::getMe().broadcastByID(senceId,msg.c_str(),msg.size());
            }
            else
            {
                ServerManager::getMe().broadcastByOne(SCENESSERVER,msg.c_str(),msg.size());
            }
            return true;
    }
	Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, resNullCmd->cmd, resNullCmd->para, nCmdLen);
	return false;
}
