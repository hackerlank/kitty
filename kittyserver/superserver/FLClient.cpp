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
#include "FLClient.h"
#include "SuperServer.h"
#include "FLCommand.h"
#include "SuperCommand.h"
#include "ServerManager.h"
#include "FLClientManager.h"
#include "LoginUserCommand.h"
#include "extractProtoMsg.h"
#include "FLCommand.h"
#include "extractProtoMsg.h"

/**
 * \brief 临时编号分配器
 *
 */
WORD FLClient::tempidAllocator = 0;

/**
 * \brief 构造函数
 * \param ip 服务器地址
 * \param port 服务器端口
 */
FLClient::FLClient(
		const std::string &ip, 
		const unsigned short port) : zTCPClientTask(ip, port, true), tempid(++tempidAllocator), netType(NetType_near)
{
}

/**
 * \brief 析构函数
 *
 */
FLClient::~FLClient()
{
}

int FLClient::checkRebound()
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
        
        using namespace CMD::FL;
        t_LoginFL_OK *ptCmd = (t_LoginFL_OK *)pstrCmd;
        if (FLCMD == ptCmd->cmd && PARA_LOGIN_OK == ptCmd->para)
        {
            Fir::logger->debug("登陆FLServer成功，收到区的编号：%u(%u, %u), %s, %u",ptCmd->gameZone.id, ptCmd->gameZone.game, ptCmd->gameZone.zone, ptCmd->name,ptCmd->netType);
            netType = (ptCmd->netType == 0 ? NetType_near : NetType_far);
            SuperService::getMe().setZoneID(ptCmd->gameZone);
            SuperService::getMe().setZoneName(ptCmd->name);
            
             
            CMD::SUPER::t_ZoneID send;
            send.zone = SuperService::getMe().getZoneID();
            strncpy(send.name,ptCmd->name,MAX_NAMESIZE);
            std::string ret;
            if(encodeMessage(&send,sizeof(send),ret))
            {
                ServerManager::getMe().broadcastByType(SCENESSERVER, ret.c_str(),ret.size());
                ServerManager::getMe().broadcastByType(GATEWAYSERVER, ret.c_str(),ret.size());
                ServerManager::getMe().broadcastByType(RECORDSERVER, ret.c_str(),ret.size());
            }
            return 1;
        }
        Fir::logger->error("登陆FLServer失败");
        return -1;
	}
	else
		return retcode;
}

void FLClient::addToContainer()
{
	FLClientManager::getMe().add(this);
}

void FLClient::removeFromContainer()
{
	FLClientManager::getMe().remove(this);
}

bool FLClient::connect()
{
	if (!zTCPClientTask::connect())
		return false;
    
    using namespace CMD::FL;
    t_LoginFL tCmd;
    strncpy(tCmd.strIP, SuperService::getMe().getIP(), sizeof(tCmd.strIP)-1);
    tCmd.port = SuperService::getMe().getPort();
    
    std::string ret;
    if(encodeMessage(&tCmd,sizeof(tCmd),ret))
    {
        return sendCmd(ret.c_str(),ret.size());
    }
    return false;

}

bool FLClient::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    Fir::logger->error("FLClient::msgParseProto 没处理");
    return true;
}

bool FLClient::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }

    using namespace CMD::FL;
    switch(ptNullCmd->cmd)
    {
        case FLCMD:
            {
                return msgParseFlCmd((FLNullCmd*)ptNullCmd,nCmdLen);
            }
            break;
    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return true;
}

bool FLClient::msgParseFlCmd(const CMD::FL::FLNullCmd *flNullCmd, const DWORD nCmdLen)
{
	using namespace CMD::FL;

	switch(flNullCmd->para)
	{
        case PARA_FL_RQGYLIST:
            {
                t_RQGYList_FL tCmd;
                
                std::string ret;
                encodeMessage(&tCmd,sizeof(tCmd),ret);
                return ServerManager::getMe().broadcastByType(GATEWAYSERVER, ret.c_str(),ret.size()); 
            }
            break;
		case PARA_SESSION_NEWSESSION:
			{
				t_NewSession_Session *ptCmd = (t_NewSession_Session *)flNullCmd;
				zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(ptCmd->session.acctype);
				if (!handle) 
				{
                    Fir::logger->error("[客户端登录_2]:客户端请求网关失败(memdb连接失败,%u,%s)",ptCmd->session.acctype,ptCmd->session.account);
					return false;
				}
				int gate_state = handle->getInt("gatewaylogin", ptCmd->session.acctype, ptCmd->session.account, "state");
                if (gate_state==0)
				{
					//玩家不在线，更新会话信息
					if(!handle->setInt("gatewaylogin",ptCmd->session.acctype, ptCmd->session.account, "gate_id", ptCmd->session.wdGatewayID))
                    {
                        Fir::logger->error("[客户端登录_2]:客户端请求网关失败(redis设置网关失败,%u,%s,%u)",ptCmd->session.acctype,ptCmd->session.account,ptCmd->session.wdGatewayID);
                        return false;
                    }
                    std::string ret;
                    if(encodeMessage(flNullCmd,nCmdLen,ret))
                    {
                        this->sendCmd(ret.c_str(),ret.size());
                    }
                    DWORD nowGateID = handle->getInt("gatewaylogin",ptCmd->session.acctype,ptCmd->session.account,"gate_id");
                    Fir::logger->error("[客户端登录_2]:客户端请求网关成功(更新网关,%u,%s,%u)",ptCmd->session.acctype,ptCmd->session.account,nowGateID);
					return true;
				}
				else
				{
					DWORD old_gate_id = handle->getInt("gatewaylogin",ptCmd->session.acctype, ptCmd->session.account, "gate_id");
					handle->setInt("gatewaylogin",ptCmd->session.acctype, ptCmd->session.account, "gate_id", ptCmd->session.wdGatewayID);
					ptCmd->session.login_ret = 1; // 把同账号踢下线
                    
					// 把同账号踢下线
                    CMD::SUPER::t_Super2GateKickOutUser send;
                    bcopy(ptCmd->session.account,send.account,sizeof(send.account));
					send.acctype = ptCmd->session.acctype;
                    std::string ret;
                    encodeMessage(&send,sizeof(send),ret);
					ServerManager::getMe().broadcastByID(old_gate_id,ret.c_str(),ret.size());
                    
                    ret.clear();
                    encodeMessage(flNullCmd,nCmdLen,ret);
                    this->sendCmd(ret.c_str(),ret.size());
                    Fir::logger->error("[客户端登录_2]:客户端请求网关成功(剔除且更新网关,%u,%s)",ptCmd->session.acctype,ptCmd->session.account);
					return true;
				}
				return true;
			}
			break;
	}

	Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, flNullCmd->cmd, flNullCmd->para, nCmdLen);
	return false;
}

