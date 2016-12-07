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
#include "GmToolClient.h"
#include "SuperServer.h"
#include "SuperCommand.h"
#include "ServerManager.h"
#include "GmToolClientManager.h"
#include "LoginUserCommand.h"
#include "extractProtoMsg.h"
#include "GmToolCommand.h"
#include "extractProtoMsg.h"

WORD GmToolClient::tempidAllocator = 0;

GmToolClient::GmToolClient(const std::string &ip,const unsigned short port) : zTCPClientTask(ip, port, true), tempid(++tempidAllocator)
{
}

GmToolClient::~GmToolClient()
{
}

int GmToolClient::checkRebound()
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
        
        using namespace CMD::GMTool;
        t_LoginGmTool_OK *ptCmd = (t_LoginGmTool_OK*)pstrCmd;
        if (GMTOOLCMD == ptCmd->cmd && PARA_LOGIN_OK == ptCmd->para)
        {
            Fir::logger->debug("登陆GmToolServer成功，收到区的编号：(%u,%u,%u,%s)",ptCmd->gameZone.id, ptCmd->gameZone.game, ptCmd->gameZone.zone, ptCmd->name);
            return 1;
        }
        Fir::logger->error("登陆GmToolServer失败");
        return -1;
	}
	else
		return retcode;
}

void GmToolClient::addToContainer()
{
	GmToolClientManager::getMe().add(this);
}

void GmToolClient::removeFromContainer()
{
	GmToolClientManager::getMe().remove(this);
}

bool GmToolClient::connect()
{
	if (!zTCPClientTask::connect())
    {
		return false;
    }
    using namespace CMD::GMTool;
    t_LoginGmTool tCmd;
    strncpy(tCmd.strIP, SuperService::getMe().getIP(), sizeof(tCmd.strIP)-1);
    tCmd.port = SuperService::getMe().getPort();
    
    std::string ret;
    if(encodeMessage(&tCmd,sizeof(tCmd),ret))
    {
        return sendCmd(ret.c_str(),ret.size());
    }
    return false;

}

bool GmToolClient::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    Fir::logger->error("GmToolClient::msgParseProto 没处理");
    return true;
}

bool GmToolClient::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }

    using namespace CMD::GMTool;
    switch(ptNullCmd->cmd)
    {
        case GMTOOLCMD:
            {
                return msgParseGmToolCmd((GmToolNullCmd*)ptNullCmd,nCmdLen);
            }
            break;
    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return true;
}

bool GmToolClient::msgParseGmToolCmd(const CMD::GMTool::GmToolNullCmd *gmToolNullCmd, const DWORD nCmdLen)
{
	using namespace CMD::GMTool;
    switch(gmToolNullCmd->para)
    {
        case PARA_Modify_Attr:
            {
                const t_GmToolModifyAttr *rev = (t_GmToolModifyAttr*)gmToolNullCmd;
                return modifyAttr(rev);
            }
            break;
        case PARA_Modify_Build:
            {
                const t_GmToolModifyBuild *rev = (t_GmToolModifyBuild*)gmToolNullCmd;
                return modifyBuild(rev);
            }
            break;
        case PARA_Forbid_Op:
            {
                const t_GmToolForbidOp *rev = (t_GmToolForbidOp*)gmToolNullCmd;
                return forbidAccount(rev);
            }
            break;
        case PARA_Email_Op:
            {
                const t_GmToolEmailOp *rev = (t_GmToolEmailOp*)gmToolNullCmd;
                return sendEmail(rev);
            }
            break;
        case PARA_Notice_Op:
            {
                std::string ret;
                const t_GmToolNoticeOp *ptCmd = (t_GmToolNoticeOp*)gmToolNullCmd;
                encodeMessage(ptCmd,sizeof(t_GmToolNoticeOp) + ptCmd->size * sizeof(GmToolNoticeData),ret);
                return ServerManager::getMe().broadcastByType(RECORDSERVER,ret.c_str(),ret.size());
            }
            break;
        case PARA_Cash_Delivery:
            {
                const t_GmToolCashDelivery *rev = (t_GmToolCashDelivery*)gmToolNullCmd;
                return modifyDelivery(rev);
            }
            break;
        case PARA_Gift_Store:
            {
                const t_GmToolGiftStore *rev = (t_GmToolGiftStore*)gmToolNullCmd;
                return modifyGiftStore(rev);
            }
            break;
        case PARA_Operator_Common:
            {
                const t_Operator_Common *rev = (t_Operator_Common *)gmToolNullCmd;
                return OperatorCommon(rev);
            }
            break;
        case PARA_Del_Picture:
            {
                const t_GmToolDelPicture *rev = (t_GmToolDelPicture*)gmToolNullCmd;
                return modifyPicture(rev);
            }
            break;
        case PARA_GLOBAL_EMAIL:
            {
                const t_GmToolGlobalEmail *rev = (t_GmToolGlobalEmail*)gmToolNullCmd;
                return sendGlobalEmail(rev);
            }
            break;
        case PARA_COMMON:
            {
                const t_GmToolCommon *rev = (t_GmToolCommon*)gmToolNullCmd;
                std::string ret;
                encodeMessage(rev,sizeof(CMD::GMTool::t_GmToolCommon) + rev->size,ret);
                return ServerManager::getMe().broadcastByType(RECORDSERVER,ret.c_str(),ret.size());
            }
            break;
        case PARA_Modify_Verify:
            {
                const t_GmToolModifyVerify *rev = (t_GmToolModifyVerify*)gmToolNullCmd;
                return modifyVerify(rev);
            }
            break;
    }
	Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, gmToolNullCmd->cmd, gmToolNullCmd->para, nCmdLen);
	return false;
}

bool GmToolClient::modifyVerify(const CMD::GMTool::t_GmToolModifyVerify *rev)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(rev->charID);
    if(!handle)
    {
        return false;
    }
    QWORD charID = handle->getInt("rolebaseinfo",rev->charID,"charid");
    if(charID != rev->charID)
    {
        return false;
    }
    std::string ret;
    encodeMessage(rev,sizeof(CMD::GMTool::t_GmToolModifyVerify) + rev->size * sizeof(CMD::GMTool::Key32Val32Pair),ret);
    DWORD senceId = handle->getInt("playerscene",rev->charID,"sceneid");
    ServerTask *task = senceId ? ServerManager::getMe().getServer(senceId) : ServerManager::getMe().randServerByType(SCENESSERVER);
    if(task)
    {
        task->sendCmd(ret.c_str(),ret.size());
    }
    return true;
}
 
bool GmToolClient::modifyAttr(const CMD::GMTool::t_GmToolModifyAttr *gmToolModityAttr)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(gmToolModityAttr->charID);
    if(!handle)
    {
        return false;
    }
    QWORD charID = handle->getInt("rolebaseinfo",gmToolModityAttr->charID,"charid");
    if(charID != gmToolModityAttr->charID)
    {
        return false;
    }
    std::string ret;
    encodeMessage(gmToolModityAttr,sizeof(CMD::GMTool::t_GmToolModifyAttr) + gmToolModityAttr->size * sizeof(CMD::GMTool::ModifyAttr),ret);
    DWORD senceId = handle->getInt("playerscene",gmToolModityAttr->charID,"sceneid");
    ServerTask *task = senceId ? ServerManager::getMe().getServer(senceId) : ServerManager::getMe().randServerByType(SCENESSERVER);
    if(task)
    {
        task->sendCmd(ret.c_str(),ret.size());
    }
    return true;
}
    
bool GmToolClient::modifyBuild(const CMD::GMTool::t_GmToolModifyBuild *gmToolModityBuild)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(gmToolModityBuild->charID);
    if(!handle)
    {
        return false;
    }
    QWORD charID = handle->getInt("rolebaseinfo",gmToolModityBuild->charID,"charid");
    if(charID != gmToolModityBuild->charID)
    {
        return false;
    }
    std::string ret;
    encodeMessage(gmToolModityBuild,sizeof(CMD::GMTool::t_GmToolModifyBuild) + gmToolModityBuild->size * sizeof(CMD::GMTool::ModifyAttr),ret);
    DWORD senceId = handle->getInt("playerscene",gmToolModityBuild->charID,"sceneid");
    ServerTask *task = senceId ? ServerManager::getMe().getServer(senceId) : ServerManager::getMe().randServerByType(SCENESSERVER);
    if(task)
    {
        task->sendCmd(ret.c_str(),ret.size());
    }
    return true;
}

bool GmToolClient::forbidAccount(const CMD::GMTool::t_GmToolForbidOp *forbidCmd)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(forbidCmd->charID);
    if(!handle)
    {
        return false;
    }
    QWORD charID = handle->getInt("rolebaseinfo",forbidCmd->charID,"charid");
    if(charID != forbidCmd->charID)
    {
        return false;
    }
    std::string ret;
    encodeMessage(forbidCmd,sizeof(CMD::GMTool::t_GmToolForbidOp),ret);
    DWORD senceId = handle->getInt("playerscene",forbidCmd->charID,"sceneid");
    ServerTask *task = senceId ? ServerManager::getMe().getServer(senceId) : ServerManager::getMe().randServerByType(SCENESSERVER);
    if(task)
    {
        task->sendCmd(ret.c_str(),ret.size());
    }
    return true;
}
 
bool GmToolClient::sendEmail(const CMD::GMTool::t_GmToolEmailOp *gmToolEmailOp)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(gmToolEmailOp->charID);
    if(!handle)
    {
        return false;
    }
    QWORD charID = handle->getInt("rolebaseinfo",gmToolEmailOp->charID,"charid");
    if(charID != gmToolEmailOp->charID)
    {
        return false;
    }
    std::string ret;
    encodeMessage(gmToolEmailOp,sizeof(CMD::GMTool::t_GmToolEmailOp) + gmToolEmailOp->size,ret);
    DWORD senceId = handle->getInt("playerscene",gmToolEmailOp->charID,"sceneid");
    ServerTask *task = senceId ? ServerManager::getMe().getServer(senceId) : ServerManager::getMe().randServerByType(SCENESSERVER);
    if(task)
    {
        task->sendCmd(ret.c_str(),ret.size());
    }
    return true;
}
 
bool GmToolClient::modifyDelivery(const CMD::GMTool::t_GmToolCashDelivery *gmDelivery)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(gmDelivery->charID);
    if(!handle)
    {
        return false;
    }
    QWORD charID = handle->getInt("rolebaseinfo",gmDelivery->charID,"charid");
    if(charID != gmDelivery->charID)
    {
        return false;
    }
    std::string ret;
    encodeMessage(gmDelivery,sizeof(CMD::GMTool::t_GmToolCashDelivery) + gmDelivery->size * sizeof(CMD::GMTool::t_GmToolCashDelivery),ret);
    DWORD senceId = handle->getInt("playerscene",gmDelivery->charID,"sceneid");
    ServerTask *task = senceId ? ServerManager::getMe().getServer(senceId) : ServerManager::getMe().randServerByType(SCENESSERVER);
    if(task)
    {
        task->sendCmd(ret.c_str(),ret.size());
    }
    return true;
}

bool GmToolClient::modifyGiftStore(const CMD::GMTool::t_GmToolGiftStore *gmGiftStore)
{
    std::string ret;
    encodeMessage(gmGiftStore,sizeof(CMD::GMTool::t_GmToolGiftStore) + gmGiftStore->size * sizeof(CMD::GMTool::GiftStoreInfo),ret);
    ServerManager::getMe().broadcastByType(RECORDSERVER,ret.c_str(),ret.size());
    return true;
}

bool GmToolClient::OperatorCommon(const CMD::GMTool::t_Operator_Common *ptCmd)
{
    std::string ret;
    encodeMessage(ptCmd,sizeof(CMD::GMTool::t_Operator_Common) + ptCmd->size,ret);
    ServerManager::getMe().broadcastByType(SCENESSERVER,ret.c_str(),ret.size());
    return true;

}

bool GmToolClient::modifyPicture(const CMD::GMTool::t_GmToolDelPicture *gmToolDelPicture)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(gmToolDelPicture->charID);
    if(!handle)
    {
        return false;
    }
    QWORD charID = handle->getInt("rolebaseinfo",gmToolDelPicture->charID,"charid");
    if(charID != gmToolDelPicture->charID)
    {
        return false;
    }
    std::string ret;
    encodeMessage(gmToolDelPicture,sizeof(CMD::GMTool::t_GmToolDelPicture) + gmToolDelPicture->size * sizeof(CMD::GMTool::DelPicture),ret);
    DWORD senceId = handle->getInt("playerscene",gmToolDelPicture->charID,"sceneid");
    ServerTask *task = senceId ? ServerManager::getMe().getServer(senceId) : ServerManager::getMe().randServerByType(SCENESSERVER);
    if(task)
    {
        task->sendCmd(ret.c_str(),ret.size());
    }
    return true;
}

bool GmToolClient::sendGlobalEmail(const CMD::GMTool::t_GmToolGlobalEmail *gmToolEmailOp)
{
    std::string ret;
    encodeMessage(gmToolEmailOp,sizeof(CMD::GMTool::t_GmToolGlobalEmail) + sizeof(CMD::GMTool::Key32Val32Pair) * gmToolEmailOp->size,ret);
    ServerTask *task = ServerManager::getMe().randServerByType(SCENESSERVER);
    if(task)
    {
        task->sendCmd(ret.c_str(),ret.size());
    }
    return true;
}
 
